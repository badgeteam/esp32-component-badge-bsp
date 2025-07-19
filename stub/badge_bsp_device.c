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
    rtc_retain_mem_t* mem = bootloader_common_get_rtc_retain_mem();

    // Remove the magic value set by the launcher to invalidated appfs bootloader struct
    memset(mem->custom, 0, sizeof(uint64_t));

    // Restart the device
    esp_restart();
}
