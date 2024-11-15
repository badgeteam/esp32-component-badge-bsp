// Board support package API: WHY2025 implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/device.h"
#include "coprocessor.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_vfs_fat.h"
#include "freertos/idf_additions.h"
#include "hal/gpio_types.h"
#include "driver/sdmmc_host.h"
#include "hardware.h"
#include "tanmatsu_coprocessor.h"
#include "ch32v203prog.h"

#include <stdbool.h>
#include <stdint.h>

#include <string.h>

static char const device_name[]         = "WHY2025 badge";
static char const device_manufacturer[] = "Badge.Team";

static const char TAG[] = "why2025-bsp";

tanmatsu_coprocessor_handle_t coprocessor_handle        = NULL;
extern uint8_t const ch32_firmware_start[] asm("_binary_ch32_firmware_bin_start");
extern uint8_t const ch32_firmware_end[] asm("_binary_ch32_firmware_bin_end");

sdmmc_slot_config_t const why2025_sdio_config = {
    .clk   = BSP_SDIO_CLK,
    .cmd   = BSP_SDIO_CMD,
    .d0    = BSP_SDIO_D0,
    .d1    = BSP_SDIO_D1,
    .d2    = BSP_SDIO_D2,
    .d3    = BSP_SDIO_D3,
    .d4    = BSP_SDIO_D4,
    .d5    = BSP_SDIO_D5,
    .d6    = BSP_SDIO_D6,
    .d7    = BSP_SDIO_D7,
    .cd    = BSP_SDIO_CD,
    .wp    = BSP_SDIO_WP,
    .width = BSP_SDIO_WIDTH,
    .flags = BSP_SDIO_FLAGS,
};

// SD-card SDIO bus configuration.
sdmmc_slot_config_t const why2025_sdcard_config = {
    .clk   = 0,
    .cmd   = 0,
    .d0    = 0,
    .d1    = 0,
    .d2    = 0,
    .d3    = 0,
    .d4    = 0,
    .d5    = 0,
    .d6    = 0,
    .d7    = 0,
    .cd    = 0,
    .wp    = 0,
    .width = BSP_SDCARD_WIDTH,
    .flags = BSP_SDCARD_FLAGS,
};


esp_err_t bsp_device_get_name(char *output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_name, buffer_length);
    return ESP_OK;
}

esp_err_t bsp_device_get_manufacturer(char *output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_manufacturer, buffer_length);
    return ESP_OK;
}


void update_coprocessor() {
    uint16_t version = 0xffff;
    esp_err_t res              = tanmatsu_coprocessor_get_firmware_version(coprocessor_handle, &version);
    if (res) {
        ESP_LOGW(TAG, "Unable to read CH32 version");
    } else if (version != BSP_CH32_VERSION) {
        ESP_LOGI(
            TAG,
            "CH32 version 0x%04" PRIx16 " too %s (expected %04" PRIx16 ")",
            version,
            version < BSP_CH32_VERSION ? "old" : "new",
            BSP_CH32_VERSION
        );
    } else {
        ESP_LOGI(TAG, "CH32 version 0x%04" PRIx16, version);
    }

    // Program the CH32 if there is a mismatch.
    if (res || version != BSP_CH32_VERSION) {
        ESP_LOGI(TAG, "Programming CH32");
        rvswd_handle_t handle = {
            .swdio = 22,
            .swclk = 23,
        };
        ch32_program(&handle, ch32_firmware_start, ch32_firmware_end - ch32_firmware_start);
        esp_restart();
    }
}

// Try to mount internal FAT filesystem.
static void bsp_mount_fatfs() {
    esp_vfs_fat_mount_config_t mount_cfg = VFS_FAT_MOUNT_DEFAULT_CONFIG();
    mount_cfg.format_if_mount_failed     = true;
    wl_handle_t wl_handle                = WL_INVALID_HANDLE;
    esp_err_t   res                      = esp_vfs_fat_spiflash_mount_rw_wl("/int", NULL, &mount_cfg, &wl_handle);
    if (res) {
        ESP_LOGE(TAG, "FAT mount error %s (%d)", esp_err_to_name(res), res);
    } else {
        ESP_LOGI(TAG, "FAT filesystem mounted");
    }
}

esp_err_t bsp_init() {
    // Check I²C pin levels.
    gpio_set_direction(BSP_I2CINT_SCL_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(BSP_I2CINT_SDA_PIN, GPIO_MODE_INPUT);
    esp_rom_delay_us(100);
    if (!gpio_get_level(BSP_I2CINT_SCL_PIN)) {
        ESP_LOGW(TAG, "SCL pin is being held LOW");
        // why2025_enable_i2cint = false;
        return ESP_FAIL;
    }
    if (!gpio_get_level(BSP_I2CINT_SDA_PIN)) {
        ESP_LOGW(TAG, "SDA pin is being held LOW");
        // why2025_enable_i2cint = false;
        return ESP_FAIL;
    }

    // Enable GPIO interrupts.
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_install_isr_service(0));

    ESP_ERROR_CHECK_WITHOUT_ABORT(bsp_why2025_coproc_init(&coprocessor_handle));

		update_coprocessor();

    ESP_ERROR_CHECK_WITHOUT_ABORT(
        tanmatsu_coprocessor_set_radio_state(coprocessor_handle, tanmatsu_coprocessor_radio_state_enabled_application)
    );

    ESP_ERROR_CHECK_WITHOUT_ABORT(sdmmc_host_init());
    ESP_ERROR_CHECK_WITHOUT_ABORT(sdmmc_host_init_slot(SDMMC_HOST_SLOT_1, &why2025_sdio_config));
    ESP_ERROR_CHECK_WITHOUT_ABORT(bsp_c6_init());

    // Try to mount internal FAT filesystem.
    bsp_mount_fatfs();

    return ESP_OK;
}

esp_err_t bsp_set_display_brightness(uint8_t brightness) {
	return tanmatsu_coprocessor_set_display_backlight(coprocessor_handle, brightness);
}
