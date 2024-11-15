// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/device.h"
#include "esp_err.h"
#include "esp_log.h"

#include <stdbool.h>
#include <stdint.h>

#include <string.h>

static char const device_name[]         = "Generic board";
static char const device_manufacturer[] = "Unknown";

static char const TAG[] = "generic-bsp-impl";

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

esp_err_t __attribute__((weak)) bsp_init() {
    ESP_LOGE(TAG, "init() not defined in the BSP");
    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_set_display_backlight(uint8_t level) {
    ESP_LOGE(TAG, "set_display_backlight() not defined in the BSP");
    return ESP_FAIL;
}

void __attribute__((weak)) bsp_disp_update(void const *framebuffer) {
    ESP_LOGE(TAG, "bsp_disp_update() not defined in the BSP");
}

void __attribute__((weak))
bsp_disp_update_part(void const *framebuffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    ESP_LOGE(TAG, "bsp_disp_update() not defined in the BSP");
}
