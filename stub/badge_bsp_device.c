// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "bsp/device.h"
#include "esp_err.h"
#include "esp_log.h"

static const char device_name[] = "Generic board";
static const char device_manufacturer[] = "Unknown";

static const char TAG[] = "generic-bsp-impl";

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

esp_err_t __attribute__((weak)) bsp_init() {
	ESP_LOGE(TAG, "init() not defined in the BSP");
	return ESP_OK;
}

esp_err_t bsp_set_display_backlight(uint8_t level) {
	ESP_LOGE(TAG, "set_display_backlight() not defined in the BSP");
	return ESP_FAIL;
}
