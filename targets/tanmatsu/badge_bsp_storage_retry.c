#include "badge_bsp_storage_retry.h"
#include "bsp/storage.h"

#include "diskio_impl.h"
#include "diskio_sdmmc.h"
#include "esp_log.h"
#include "esp_memory_utils.h"
#include "esp_heap_caps.h"
#include "soc/soc_caps.h"
#include "sdkconfig.h"
#include <string.h>
#include "ff.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdmmc_cmd.h"

static char const* TAG = "sdretry";

// ESCALATING, because the cause is not pinned down and the two
// candidates want opposite things.
//
// ESP_ERR_TIMEOUT arrives here from three different hardware
// interrupts, and the driver does not say which at default log level
// (the line that would, "error 0x%x (status=%08x)" in
// process_cmd_response_status, is ESP_LOGD):
//
//   RTO  the card never answered the command
//   DTO  the data phase ran past the controller's timeout counter
//   EBE  the card's CRC-status token came back malformed on a write
//
// DTO is a card that is genuinely busy -- finishing a program or erase
// cycle, which on a cheap card is 100-250 ms and on a bad one more. A
// retry 5 ms later would land in the middle of the same cycle and fail
// again, and four of those would burn the whole allowance in 20 ms and
// give up while the card was still working. RTO and EBE are the
// opposite: signalling, where the fast retry is the one that works and
// waiting achieves nothing.
//
// So: fast first, then long enough to outlast a program cycle. The
// first retry is 2 ms and costs nothing; the last leaves a third of a
// second of quiet, which is past the point where a healthy card is
// still thinking.
static uint16_t const SD_BACKOFF_MS[] = {0, 2, 20, 100, 250};
#define SD_TRIES ((int)(sizeof SD_BACKOFF_MS / sizeof SD_BACKOFF_MS[0]))

// Both cache levels on the P4 have a 64 B line, and the driver demands the
// buffer aligned to the line of the level that covers it. Fallbacks for
// targets where these config values do not exist.
#ifndef CONFIG_CACHE_L1_CACHE_LINE_SIZE
#define CONFIG_CACHE_L1_CACHE_LINE_SIZE 32
#endif
#ifndef CONFIG_CACHE_L2_CACHE_LINE_SIZE
#define CONFIG_CACHE_L2_CACHE_LINE_SIZE 32
#endif

// Whether esp_cache_msync() would take this address. Written out as the
// ranges it accepts, because the check itself is private to IDF
// (esp_mm/esp_cache_msync.c:113-117) and logs at VERBOSE: L1 covers
// internal RAM, L2 covers flash and PSRAM, and nothing else.
//
// Two heap ranges fall outside both. SPM at 0x30100000 is in the heap
// unconditionally (heap/port/esp32p4/memory_layout.c:115), and LPRAM at
// 0x50108000 is in the heap whenever CONFIG_ESP_SYSTEM_ALLOW_RTC_FAST_MEM_AS_HEAP
// is on -- which is what this run's log was missing, and what
// sdkconfigs/tanmatsu now turns off. esp_ptr_internal() counts both as
// internal, so it cannot be used on its own.
static bool cacheable(void const* p) {
    if (esp_ptr_external_ram(p)) return true;
#if SOC_MEM_SPM_SUPPORTED
    if (esp_ptr_in_spm(p)) return false;
#endif
    if (esp_ptr_in_rtc_dram_fast(p)) return false;
    return esp_ptr_internal(p);
}

static size_t cache_line_of(void const* p) {
    return esp_ptr_external_ram(p) ? CONFIG_CACHE_L2_CACHE_LINE_SIZE : CONFIG_CACHE_L1_CACHE_LINE_SIZE;
}

// What sd_host_check_buffer_alignment() plus esp_cache_msync() demand: the
// buffer in a cacheable range, aligned to that cache line, with a length
// that is a multiple of it. Both are properties of the address, so neither
// changes between retries -- which is why a refused buffer is not worth
// backing off for.
static bool usable_for_dma(void const* buff, size_t bytes) {
    if (!cacheable(buff)) return false;
    size_t const line = cache_line_of(buff);
    return ((uintptr_t)buff % line == 0) && (bytes % line == 0);
}

// The buffer the transaction runs on when the caller's cannot be used.
// Allocated on first need, in PSRAM with internal RAM as the fallback:
// PSRAM is L2 cacheable, so it is a valid SDMMC DMA target, and 8 KiB does
// not come out of the internal pool the SDMMC DMA buffer, ESP-Hosted and the
// task stacks are competing for. Sector multiples, so the length is always a
// multiple of the line. Larger requests are bounced in pieces. Released by
// bsp_storage_release_retrying_diskio() on unmount, so it is only held while
// a card is mounted.
#define BOUNCE_SECTORS 16
#define BOUNCE_BYTES   (BOUNCE_SECTORS * 512)

static uint8_t* s_bounce;

static uint8_t* bounce_buffer(void) {
    if (s_bounce != NULL) return s_bounce;
    // Whichever region ends up holding it has to be aligned to that region's
    // cache line, so align for the wider of the two.
    size_t const line = (CONFIG_CACHE_L2_CACHE_LINE_SIZE > CONFIG_CACHE_L1_CACHE_LINE_SIZE)
                            ? CONFIG_CACHE_L2_CACHE_LINE_SIZE
                            : CONFIG_CACHE_L1_CACHE_LINE_SIZE;
    s_bounce = heap_caps_aligned_alloc(line, BOUNCE_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (s_bounce == NULL) {
        s_bounce = heap_caps_aligned_alloc(line, BOUNCE_BYTES, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
    return s_bounce;
}

// Whether a failure is worth another go. RTO, DTO and EBE say the
// transaction was disturbed and the card has just said it is ready, so
// the same bytes sent again usually work. ESP_ERR_NOT_SUPPORTED says the
// buffer was refused before the card was asked, and the same buffer is
// refused every time -- so the backoff only spends time on it.
static bool worth_retrying(esp_err_t err) { return err != ESP_ERR_NOT_SUPPORTED; }

// Where a DMA buffer sits, in the terms the SDMMC path uses. The
// cacheability check accepts internal RAM (0x4ff00000-0x4ffc0000, L1)
// and flash plus PSRAM (0x40000000-0x4C000000, L2), and nothing else.
// Two heap ranges fall outside both on the P4: LPRAM at 0x50108000, in
// the heap whenever CONFIG_ESP_SYSTEM_ALLOW_RTC_FAST_MEM_AS_HEAP is on
// (it was, until this file's run), and SPM at 0x30100000. A buffer there
// still passes the driver's alignment
// check, which is why a run can show transactions failing with no line
// saying why -- every other failure in that path logs its own ESP_LOGE,
// and this one logs at VERBOSE.
static char const* buffer_region(void const* p) {
    if (esp_ptr_external_ram(p)) return "PSRAM";
#if SOC_MEM_SPM_SUPPORTED
    if (esp_ptr_in_spm(p)) return "SPM";
#endif
    if (esp_ptr_in_rtc_dram_fast(p)) return "LPRAM";
    if (esp_ptr_internal(p)) return "internal RAM";
    return "outside every cacheable range";
}

static sdmmc_card_t* s_card;
static uint32_t s_bounced;   // transactions that had to run on the bounce buffer

// How long the backoff had spent waiting by attempt `try`. Printed with
// a success, because "worked on attempt 4, after 122 ms" says the card
// needed the time, and "attempt 2, after 2 ms" says it did not -- which
// is the difference between DTO and RTO/EBE without needing debug
// logging turned on.
static unsigned waited_ms(int try) {
    unsigned ms = 0;
    for (int i = 0; i <= try && i < SD_TRIES; i++) ms += SD_BACKOFF_MS[i];
    return ms;
}

// What it cost, so a card or a controller that is genuinely unwell
// cannot hide behind the retries.
static uint32_t s_retried;   // operations that needed more than one go
static uint32_t s_failed;    // ... and the ones that ran out of goes

static void report_failure(bool write, DWORD sector, UINT count, int attempts, esp_err_t err, void const* caller) {
    s_failed++;
    if (write) {
        ESP_LOGE(TAG, "write of %u sector(s) at %" PRIu32 " failed %d time(s) (0x%x) -- FatFs will leak the cluster",
                 (unsigned)count, (uint32_t)sector, attempts, err);
    } else {
        ESP_LOGE(TAG, "read of %u sector(s) at %" PRIu32 " failed %d time(s) (0x%x)", (unsigned)count,
                 (uint32_t)sector, attempts, err);
    }
    if (err == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGE(TAG, "buffer %p is %s -- esp_cache_msync() refused it, so no retry can succeed", caller,
                 buffer_region(caller));
    }
}

// One run of the backoff over a buffer that is known to be usable.
static esp_err_t sd_run(void const* buf, DWORD sector, UINT count, bool write, int* attempts) {
    esp_err_t err = ESP_OK;
    for (int try = 0; try < SD_TRIES; try++) {
        if (SD_BACKOFF_MS[try] > 0) vTaskDelay(pdMS_TO_TICKS(SD_BACKOFF_MS[try]));
        // Idempotent: the same bytes to the same sectors. A partially
        // written multi-sector run is simply written again from the
        // start.
        err = write ? sdmmc_write_sectors(s_card, buf, sector, count)
                    : sdmmc_read_sectors(s_card, (void*)buf, sector, count);
        (*attempts)++;
        if (err == ESP_OK) {
            if (try > 0) {
                s_retried++;
                // Which attempt worked is the diagnosis: attempt 2 is a
                // glitch, attempt 4 or 5 is a card that needed the time.
                ESP_LOGW(TAG, "%s of sector %" PRIu32 " succeeded on attempt %d (after %u ms of waiting)",
                         write ? "write" : "read", sector, try + 1, (unsigned)waited_ms(try));
            }
            return ESP_OK;
        }
        if (!worth_retrying(err)) break;
    }
    return err;
}

static DRESULT sd_transfer(BYTE pdrv, void* buff, DWORD sector, UINT count, bool write) {
    (void)pdrv;
    if (s_card == NULL) return RES_ERROR;
    size_t const sector_size = s_card->csd.sector_size;

    for (DWORD done = 0; done < count;) {
        UINT n = count - done;
        char* at = (char*)buff + (size_t)done * sector_size;
        int attempts = 0;

        if (usable_for_dma(at, (size_t)n * sector_size)) {
            esp_err_t err = sd_run(at, sector + done, n, write, &attempts);
            if (err != ESP_OK) {
                report_failure(write, sector, count, attempts, err, at);
                return RES_ERROR;
            }
            done += n;
            continue;
        }

        // The caller's buffer cannot carry a transaction. Copy through one
        // that can, in pieces if the request is bigger than the bounce.
        uint8_t* b = bounce_buffer();
        if (b == NULL) {
            s_failed++;
            ESP_LOGE(TAG, "buffer %p is %s and no bounce buffer could be allocated", at, buffer_region(at));
            return RES_ERROR;
        }
        if (n > BOUNCE_SECTORS) n = BOUNCE_SECTORS;
        s_bounced++;
        if (write) memcpy(b, at, (size_t)n * sector_size);
        esp_err_t err = sd_run(b, sector + done, n, write, &attempts);
        if (err != ESP_OK) {
            report_failure(write, sector, count, attempts, err, at);
            return RES_ERROR;
        }
        if (!write) memcpy(at, b, (size_t)n * sector_size);
        done += n;
    }
    return RES_OK;
}

static DRESULT sd_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count) {
    return sd_transfer(pdrv, buff, sector, count, false);
}

static DRESULT sd_write(BYTE pdrv, BYTE const* buff, DWORD sector, UINT count) {
    return sd_transfer(pdrv, (void*)buff, sector, count, true);
}

static DSTATUS sd_status(BYTE pdrv) {
    (void)pdrv;
    return s_card != NULL ? 0 : STA_NOINIT;
}

static DSTATUS sd_initialize(BYTE pdrv) {
    return sd_status(pdrv);
}

// The same answers ESP-IDF's ff_sdmmc_ioctl gives. TRIM is left out on
// purpose: FF_USE_TRIM is off in this build, and a discard that fails
// half way is not something to retry blindly.
static DRESULT sd_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    (void)pdrv;
    if (s_card == NULL) return RES_ERROR;
    switch (cmd) {
        case CTRL_SYNC: return RES_OK;
        case GET_SECTOR_COUNT: *((DWORD*)buff) = (DWORD)s_card->csd.capacity; return RES_OK;
        case GET_SECTOR_SIZE: *((WORD*)buff) = (WORD)s_card->csd.sector_size; return RES_OK;
        case GET_BLOCK_SIZE: return RES_ERROR;  // as IDF's does
        default: break;
    }
    return RES_ERROR;
}

void bsp_storage_install_retrying_diskio(sdmmc_card_t* card) {
    if (card == NULL) return;
    s_card = card;

    BYTE const pdrv = ff_diskio_get_pdrv_card(card);
    if (pdrv == 0xFF) {
        ESP_LOGE(TAG, "the card has no drive number; leaving ESP-IDF's disk layer in place");
        return;
    }

    static ff_diskio_impl_t const impl = {
        .init   = &sd_initialize,
        .status = &sd_status,
        .read   = &sd_read,
        .write  = &sd_write,
        .ioctl  = &sd_ioctl,
    };
    ff_diskio_register(pdrv, &impl);
    ESP_LOGI(TAG, "retrying disk layer installed on pdrv %u (%d tries, up to %u ms of backoff, %u sector(s) of bounce)",
             (unsigned)pdrv, SD_TRIES, waited_ms(SD_TRIES - 1), (unsigned)BOUNCE_SECTORS);
}

void bsp_storage_get_retry_stats(uint32_t* retried, uint32_t* failed, uint32_t* bounced) {
    if (retried != NULL) *retried = s_retried;
    if (failed != NULL) *failed = s_failed;
    if (bounced != NULL) *bounced = s_bounced;
}

void bsp_storage_release_retrying_diskio(void) {
    // The card pointer goes first: every disk callback checks it, so nothing
    // can reach the buffer while it is being freed or use the card after
    // esp_vfs_fat_sdcard_unmount() has freed it.
    s_card = NULL;
    if (s_bounce != NULL) {
        heap_caps_free(s_bounce);
        s_bounce = NULL;
    }
}
