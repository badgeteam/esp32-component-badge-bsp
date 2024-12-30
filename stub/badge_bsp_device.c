// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/device.h"
#include "esp_err.h"

#include <stdbool.h>
#include <stdint.h>

#include <string.h>

static char const device_name[]         = "Generic board";
static char const device_manufacturer[] = "Unknown";

esp_err_t __attribute__((weak)) bsp_device_initialize(void) {
    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_device_get_name(char *output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_name, buffer_length);
    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_device_get_manufacturer(char *output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_manufacturer, buffer_length);
    return ESP_OK;
}

bool __attribute__((weak)) bsp_device_get_initialized_without_coprocessor(void) {
    return false;
}
