#include "bsp/input.h"
#include "bsp/storage.h"
#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_idf_version.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "hal/gpio_types.h"
#include "sd_protocol_defs.h"
#include "sd_pwr_ctrl.h"
#include "sd_pwr_ctrl_by_on_chip_ldo.h"
#include "sdmmc_cmd.h"

#define MOUNT_POINT_STRING_LENGTH 32

#if defined(CONFIG_ESP_HOSTED_SDIO_HOST_INTERFACE) && (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0))
#define SDCARD_WORKAROUND_HOSTED_DOES_SDMMC_HOST_INIT 1
#else
#define SDCARD_WORKAROUND_HOSTED_DOES_SDMMC_HOST_INIT 0
#endif

static char const TAG[] = "sdcard";

// SD card
static sdmmc_card_t*        sd_card       = NULL;
static sd_pwr_ctrl_handle_t sd_pwr_handle = NULL;
static uint8_t*             sd_dma_buf    = NULL;  // 2KB aligned buffer, allocated once and reused
static char                 sd_mount_point[MOUNT_POINT_STRING_LENGTH] = "";
static bool                 sd_failed                                 = false;

// Internal FAT filesystem
static wl_handle_t int_wl_handle                              = WL_INVALID_HANDLE;
static char        int_mount_point[MOUNT_POINT_STRING_LENGTH] = "";

#if SDCARD_WORKAROUND_HOSTED_DOES_SDMMC_HOST_INIT
static esp_err_t sdmmc_host_init_noop(void) {
    return ESP_OK;
}

// The SDMMC ISR services the slot of the last transaction (cur_slot_id). If slot 0 is removed while it
// is still that slot, the next SDIO interrupt from ESP-Hosted dereferences the removed slot and panics.
// Do a harmless CMD52 read (CCCR address 0) on the ESP-Hosted slot first so the ISR points at a live slot.
static esp_err_t sdmmc_host_deinit_slot_hosted(int slot) {
    if (slot != CONFIG_ESP_HOSTED_SDIO_SLOT) {
        sdmmc_command_t cmd = {
            .opcode = SD_IO_RW_DIRECT,
            .arg    = 0,
            .flags  = SCF_CMD_AC | SCF_RSP_R5,
            // The raw host call doesn't apply a default timeout; 0 would give up before the
            // response arrives and leave it pending for ESP-Hosted's next transaction.
            .timeout_ms = 1000,
        };
        esp_err_t res = sdmmc_host_do_transaction(CONFIG_ESP_HOSTED_SDIO_SLOT, &cmd);
        if (res != ESP_OK) {
            ESP_LOGW(TAG, "Dummy transaction on ESP-Hosted slot failed (%s)", esp_err_to_name(res));
        }
    }
    return sdmmc_host_deinit_slot(slot);
}
#endif

static sd_pwr_ctrl_handle_t initialize_sd_ldo(void) {
    sd_pwr_ctrl_ldo_config_t ldo_config = {
        .ldo_chan_id = 4,
    };
    sd_pwr_ctrl_handle_t pwr_ctrl_handle = NULL;
    esp_err_t            res             = sd_pwr_ctrl_new_on_chip_ldo(&ldo_config, &pwr_ctrl_handle);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create a new on-chip LDO power control driver");
        return NULL;
    }
    // Don't set voltage here - let SDMMC driver set it via host.io_voltage (3.3V default)
    return pwr_ctrl_handle;
}

static esp_err_t reset_sd_card(void) {
    if (sd_pwr_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    ESP_LOGI(TAG, "Power cycling SD card...");

    // Pull all SDIO bus lines low
    gpio_config_t gpio_cfg = {
        .pin_bit_mask = BIT64(GPIO_NUM_39) | BIT64(GPIO_NUM_40) | BIT64(GPIO_NUM_41) | BIT64(GPIO_NUM_42) |
                        BIT64(GPIO_NUM_43) | BIT64(GPIO_NUM_44),
        .mode         = GPIO_MODE_OUTPUT_OD,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&gpio_cfg);
    gpio_set_level(GPIO_NUM_39, 0);
    gpio_set_level(GPIO_NUM_40, 0);
    gpio_set_level(GPIO_NUM_41, 0);
    gpio_set_level(GPIO_NUM_42, 0);
    gpio_set_level(GPIO_NUM_43, 0);
    gpio_set_level(GPIO_NUM_44, 0);

    // Decrease LDO output voltage to minimum
    sd_pwr_ctrl_set_io_voltage(sd_pwr_handle, 0);
    vTaskDelay(pdMS_TO_TICKS(150));  // Wait 150ms for card to power down

    // Power on the SD card at 3.3V & release GPIOs
    gpio_cfg.mode = GPIO_MODE_INPUT;
    gpio_config(&gpio_cfg);
    sd_pwr_ctrl_set_io_voltage(sd_pwr_handle, 3300);
    vTaskDelay(pdMS_TO_TICKS(150));  // Wait 150ms for card to stabilize
    return ESP_OK;
}

static bool get_sd_inserted(void) {
    bool      sdcard_inserted = false;
    esp_err_t res             = bsp_input_read_action(BSP_INPUT_ACTION_TYPE_SD_CARD, &sdcard_inserted);
    return (res == ESP_OK) && sdcard_inserted;
}

esp_err_t bsp_storage_initialize(void) {
    sd_pwr_handle = initialize_sd_ldo();
    if (sd_pwr_handle == NULL) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

bool bsp_storage_get_supported(bsp_storage_type_t type) {
    switch (type) {
        case BSP_STORAGE_TYPE_INTERNAL:
        case BSP_STORAGE_TYPE_SDCARD:
            return true;
        default:
            return false;
    }
}

bsp_storage_status_t bsp_storage_get_status(bsp_storage_type_t type) {
    switch (type) {
        case BSP_STORAGE_TYPE_INTERNAL:
            return BSP_STORAGE_STATUS_NOT_SUPPORTED;
            break;
        case BSP_STORAGE_TYPE_SDCARD:
            if (get_sd_inserted()) {
                return sd_failed ? BSP_STORAGE_STATUS_ERROR
                                 : (sd_card ? BSP_STORAGE_STATUS_MOUNTED : BSP_STORAGE_STATUS_NOT_MOUNTED);
            } else {
                return BSP_STORAGE_STATUS_NOT_PRESENT;
            }
            break;
        default:
            break;
    }
    return BSP_STORAGE_STATUS_NOT_SUPPORTED;
}

esp_err_t bsp_storage_mount(bsp_storage_type_t type, const char* mountpoint) {
    if (mountpoint == NULL || strlen(mountpoint) >= MOUNT_POINT_STRING_LENGTH) {
        ESP_LOGE(TAG, "Invalid mount point");
        return ESP_ERR_INVALID_ARG;
    }

    if (type == BSP_STORAGE_TYPE_SDCARD) {
        sd_failed = false;
        if (sd_card) {
            bsp_storage_unmount(BSP_STORAGE_TYPE_SDCARD);
        }

        esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = false, .max_files = 10, .allocation_unit_size = 16 * 1024};

        ESP_LOGI(TAG, "Initializing SD card");

        // Power cycle the SD card to ensure it's in a known state
        // This prevents issues when the card was left in SDMMC mode from a previous session
        esp_err_t res = reset_sd_card();
        if (res != ESP_OK) {
            ESP_LOGE(TAG, "Failed to reset SD card");
            sd_failed = true;
            return res;
        }

        sdmmc_host_t host    = SDMMC_HOST_DEFAULT();
        host.slot            = SDMMC_HOST_SLOT_0;     // Use SLOT0 for native IOMUX pins
        host.max_freq_khz    = SDMMC_FREQ_HIGHSPEED;  // 40MHz
        host.pwr_ctrl_handle = sd_pwr_handle;
#if SDCARD_WORKAROUND_HOSTED_DOES_SDMMC_HOST_INIT
        // ESP-Hosted already owns the shared SDMMC host controller, so skip init. Don't set
        // host.deinit: it shares a union with deinit_p, which must still release slot 0
        // on unmount (the controller is kept while ESP-Hosted uses it).
        host.init     = &sdmmc_host_init_noop;
        host.deinit_p = &sdmmc_host_deinit_slot_hosted;
#endif

        if (sd_dma_buf == NULL) {
            ESP_LOGI(TAG, "Allocating SD DMA buffer");
            sd_dma_buf = heap_caps_malloc(512 * 4, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
            if (sd_dma_buf == NULL) {
                ESP_LOGE(TAG, "Failed to allocate DMA buffer for SD card");
                sd_failed = true;
                return ESP_ERR_NO_MEM;
            }
        }
        host.dma_aligned_buffer = sd_dma_buf;

        sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
        slot_config.clk                 = GPIO_NUM_43;
        slot_config.cmd                 = GPIO_NUM_44;
        slot_config.d0                  = GPIO_NUM_39;
        slot_config.d1                  = GPIO_NUM_40;
        slot_config.d2                  = GPIO_NUM_41;
        slot_config.d3                  = GPIO_NUM_42;
        slot_config.width               = 4;  // 4-bit mode

        snprintf(sd_mount_point, sizeof(sd_mount_point), "%s", mountpoint);

        res = esp_vfs_fat_sdmmc_mount(sd_mount_point, &host, &slot_config, &mount_config, &sd_card);

        if (res != ESP_OK) {
            if (res == ESP_FAIL) {
                ESP_LOGE(TAG, "Failed to mount SD card filesystem");
            } else {
                ESP_LOGE(TAG, "Failed to initialize the SD card (%s)", esp_err_to_name(res));
            }
            sd_failed = true;
            return res;
        }

        ESP_LOGI(TAG, "Mounted SD at '%s'", sd_mount_point);

        return ESP_OK;
    }

    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_storage_unmount(bsp_storage_type_t type) {
    if (type == BSP_STORAGE_TYPE_SDCARD) {
        if (sd_card == NULL) {
            // Already unmounted
            ESP_LOGI(TAG, "Already unmounted");
            return ESP_OK;
        }
        esp_err_t res = esp_vfs_fat_sdcard_unmount(sd_mount_point, sd_card);
        if (res != ESP_OK) {
            ESP_LOGE(TAG, "SD card unmount returned error: %s", esp_err_to_name(res));
            return res;
        }
        ESP_LOGI(TAG, "SD card unmounted");
        sd_card = NULL;
        return ESP_OK;
    }
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_storage_get_mounted(bsp_storage_type_t type, bool* out_mounted) {
    if (out_mounted == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (type == BSP_STORAGE_TYPE_SDCARD) {
        *out_mounted = (sd_card != NULL);
        return ESP_OK;
    }
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_storage_get_mountpoint(bsp_storage_type_t type, char* out_mountpoint, size_t max_length) {
    if (out_mountpoint == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (type == BSP_STORAGE_TYPE_SDCARD) {
        snprintf(out_mountpoint, max_length, "%s", sd_mount_point);
        return ESP_OK;
    }
    return ESP_ERR_NOT_SUPPORTED;
}
