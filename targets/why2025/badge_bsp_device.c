// Board support package API: WHY2025 implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/device.h"
#include "bsp/display.h"
#include "bsp/i2c.h"
#include "esp_check.h"
#include "esp_err.h"

#include <stdbool.h>
#include <stdint.h>

#include <string.h>

static char const *TAG = "BSP device";

static char const device_name[]         = "WHY2025 badge";
static char const device_manufacturer[] = "Badge.Team";

esp_err_t bsp_device_initialize(void) {
    ESP_RETURN_ON_ERROR(bsp_display_initialize(), TAG, "Display failed to initialize");
    ESP_RETURN_ON_ERROR(bsp_i2c_primary_bus_initialize(), TAG, "Primary I2C bus failed to initialize");
    return ESP_OK;
}

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
