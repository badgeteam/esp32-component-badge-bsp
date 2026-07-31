// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "bootloader_common.h"
#include "bsp/device.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"

#if CONFIG_APPFS_USE_RTC_REG
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32C2
#include "soc/rtc_cntl_reg.h"
#define APPFS_RTC_REG RTC_CNTL_STORE0_REG
#elif CONFIG_IDF_TARGET_ESP32C61
#include "soc/lp_aon_reg.h"
#define APPFS_RTC_REG LP_AON_STORE0_REG
#elif CONFIG_IDF_TARGET_ESP32S31
#include "soc/lp_system_reg.h"
#define APPFS_RTC_REG LP_SYSTEM_REG_LP_STORE0_REG
#endif
#endif

static char const device_name[]         = "Generic board";
static char const device_manufacturer[] = "Unknown";

esp_err_t __attribute__((weak)) bsp_device_initialize_custom(void) {
    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_device_get_name(char* output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_name, buffer_length);
    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_device_get_manufacturer(char* output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_manufacturer, buffer_length);
    return ESP_OK;
}

bool __attribute__((weak)) bsp_device_get_initialized_without_coprocessor(void) {
    return false;
}

void __attribute__((weak)) bsp_device_restart_to_launcher(void) {
    // This function is common to all supported devices, but it can still be overridden if needed
#if CONFIG_APPFS_USE_RTC_REG
    // Clear the retained register so the bootloader does not try to boot into AppFS again.
    REG_WRITE(APPFS_RTC_REG, 0);
#else
    rtc_retain_mem_t* mem = bootloader_common_get_rtc_retain_mem();

    // Remove the magic value set by the launcher to invalidated appfs bootloader struct
    memset(mem->custom, 0, sizeof(uint64_t));
#endif

    // Restart the device
    esp_restart();
}
