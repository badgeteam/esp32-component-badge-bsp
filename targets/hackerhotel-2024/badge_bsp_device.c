// Board support package API: Hackerhotel 2024 implementation
// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "bsp/device.h"
#include "bsp/display.h"
#include "bsp/i2c.h"
#include "bsp/input.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

static char const* TAG = "BSP device";

static char const device_name[]         = "Hackerhotel 2024 badge";
static char const device_manufacturer[] = "Badge.Team";

esp_err_t bsp_device_initialize(void) {
    gpio_install_isr_service(0);

    ESP_RETURN_ON_ERROR(bsp_display_initialize(), TAG, "Display failed to initialize");
    ESP_RETURN_ON_ERROR(bsp_input_initialize(), TAG, "Input failed to initialize");
    return ESP_OK;
}

bool bsp_device_get_initialized_without_coprocessor(void) {
    return false;
}

esp_err_t bsp_device_get_name(char* output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_name, buffer_length);
    return ESP_OK;
}

esp_err_t bsp_device_get_manufacturer(char* output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_manufacturer, buffer_length);
    return ESP_OK;
}
