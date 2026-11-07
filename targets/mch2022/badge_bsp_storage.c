// Board support package API: MCH2022 implementation
// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <string.h>
#include "bsp/storage.h"
#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "mch2022_hardware.h"
#include "sdmmc_cmd.h"

#define MOUNT_POINT_STRING_LENGTH 32

static char const TAG[] = "BSP: storage";

static sdmmc_card_t* sd_card                                   = NULL;
static char          sd_mount_point[MOUNT_POINT_STRING_LENGTH] = "";
static bool          sd_failed                                 = false;

static wl_handle_t int_wl_handle                              = WL_INVALID_HANDLE;
static char        int_mount_point[MOUNT_POINT_STRING_LENGTH] = "";

esp_err_t bsp_storage_initialize(void) {
    gpio_config_t power_pin_conf = {
        .pin_bit_mask = BIT64(BSP_SDCARD_LED_POWER_PIN),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = 0,
        .pull_down_en = 0,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&power_pin_conf);
    gpio_set_level(BSP_SDCARD_LED_POWER_PIN, true);
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
            return (int_wl_handle != WL_INVALID_HANDLE) ? BSP_STORAGE_STATUS_MOUNTED : BSP_STORAGE_STATUS_NOT_MOUNTED;
        case BSP_STORAGE_TYPE_SDCARD:
            if (sd_card != NULL) {
                return BSP_STORAGE_STATUS_MOUNTED;
            }
            return sd_failed ? BSP_STORAGE_STATUS_ERROR : BSP_STORAGE_STATUS_NOT_MOUNTED;
        default:
            return BSP_STORAGE_STATUS_NOT_SUPPORTED;
    }
}

esp_err_t bsp_storage_mount(bsp_storage_type_t type, const char* mountpoint) {
    return bsp_storage_mount_advanced(type, mountpoint, 5, false);
}

esp_err_t bsp_storage_mount_advanced(bsp_storage_type_t type, const char* mountpoint, int max_files,
                                     bool format_if_mount_failed) {
    if (mountpoint == NULL || strlen(mountpoint) >= MOUNT_POINT_STRING_LENGTH) {
        ESP_LOGE(TAG, "Invalid mount point");
        return ESP_ERR_INVALID_ARG;
    }

    if (type == BSP_STORAGE_TYPE_INTERNAL) {
        if (int_wl_handle != WL_INVALID_HANDLE) {
            bsp_storage_unmount(BSP_STORAGE_TYPE_INTERNAL);
        }

        esp_vfs_fat_mount_config_t mount_config = {
            .format_if_mount_failed   = format_if_mount_failed,
            .max_files                = max_files,
            .allocation_unit_size     = CONFIG_WL_SECTOR_SIZE,
            .disk_status_check_enable = false,
            .use_one_fat              = false,
        };

        snprintf(int_mount_point, sizeof(int_mount_point), "%s", mountpoint);

        esp_err_t res = esp_vfs_fat_spiflash_mount_rw_wl(int_mount_point, "locfd", &mount_config, &int_wl_handle);
        if (res != ESP_OK) {
            ESP_LOGE(TAG, "Failed to mount internal FAT filesystem: %s", esp_err_to_name(res));
            int_wl_handle = WL_INVALID_HANDLE;
            return res;
        }

        ESP_LOGI(TAG, "Mounted internal FAT filesystem at '%s'", int_mount_point);

        return ESP_OK;
    }

    if (type == BSP_STORAGE_TYPE_SDCARD) {
        sd_failed = false;
        if (sd_card) {
            bsp_storage_unmount(BSP_STORAGE_TYPE_SDCARD);
        }

        gpio_set_level(BSP_SDCARD_LED_POWER_PIN, true);

        // The SD bus only has a single data line connected (D1/D2/D3 are
        // repurposed as I2S pins), so pull-ups on CMD/CLK/D0 must be
        // enabled in software: the board has no pull-up resistors for them.
        gpio_config_t bus_pin_conf = {
            .pin_bit_mask = BIT64(BSP_SDCARD_CLK_PIN) | BIT64(BSP_SDCARD_CMD_PIN) | BIT64(BSP_SDCARD_D0_PIN),
            .mode         = GPIO_MODE_INPUT_OUTPUT_OD,
            .pull_up_en   = 1,
            .pull_down_en = 0,
            .intr_type    = GPIO_INTR_DISABLE,
        };
        gpio_config(&bus_pin_conf);
        gpio_set_level(BSP_SDCARD_CLK_PIN, 1);
        gpio_set_level(BSP_SDCARD_CMD_PIN, 1);
        gpio_set_level(BSP_SDCARD_D0_PIN, 1);

        sdmmc_host_t host = SDMMC_HOST_DEFAULT();
        host.flags        = SDMMC_HOST_FLAG_1BIT;

        sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
        slot_config.clk                 = BSP_SDCARD_CLK_PIN;
        slot_config.cmd                 = BSP_SDCARD_CMD_PIN;
        slot_config.d0                  = BSP_SDCARD_D0_PIN;
        slot_config.width               = 1;

        esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = format_if_mount_failed,
            .max_files              = max_files,
            .allocation_unit_size   = 16 * 1024,
        };

        snprintf(sd_mount_point, sizeof(sd_mount_point), "%s", mountpoint);

        esp_err_t res = esp_vfs_fat_sdmmc_mount(sd_mount_point, &host, &slot_config, &mount_config, &sd_card);
        if (res != ESP_OK) {
            if (res == ESP_FAIL) {
                ESP_LOGE(TAG, "Failed to mount the SD card filesystem");
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
    if (type == BSP_STORAGE_TYPE_INTERNAL) {
        if (int_wl_handle == WL_INVALID_HANDLE) {
            ESP_LOGI(TAG, "Already unmounted");
            return ESP_OK;
        }
        esp_err_t res = esp_vfs_fat_spiflash_unmount_rw_wl(int_mount_point, int_wl_handle);
        if (res != ESP_OK) {
            ESP_LOGE(TAG, "Internal FAT unmount returned error: %s", esp_err_to_name(res));
            return res;
        }
        ESP_LOGI(TAG, "Internal FAT filesystem unmounted");
        int_wl_handle = WL_INVALID_HANDLE;
        return ESP_OK;
    }
    if (type == BSP_STORAGE_TYPE_SDCARD) {
        if (sd_card == NULL) {
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
    if (type == BSP_STORAGE_TYPE_INTERNAL) {
        *out_mounted = (int_wl_handle != WL_INVALID_HANDLE);
        return ESP_OK;
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
    if (type == BSP_STORAGE_TYPE_INTERNAL) {
        snprintf(out_mountpoint, max_length, "%s", int_mount_point);
        return ESP_OK;
    }
    if (type == BSP_STORAGE_TYPE_SDCARD) {
        snprintf(out_mountpoint, max_length, "%s", sd_mount_point);
        return ESP_OK;
    }
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_storage_format(bsp_storage_type_t type) {
    if (type == BSP_STORAGE_TYPE_INTERNAL) {
        if (int_wl_handle == WL_INVALID_HANDLE) {
            return ESP_ERR_INVALID_STATE;
        }
        return esp_vfs_fat_spiflash_format_rw_wl(int_mount_point, "locfd");
    }
    if (type == BSP_STORAGE_TYPE_SDCARD) {
        if (sd_card == NULL) {
            return ESP_ERR_INVALID_STATE;
        }
        return esp_vfs_fat_sdcard_format(sd_mount_point, sd_card);
    }
    return ESP_ERR_NOT_SUPPORTED;
}
