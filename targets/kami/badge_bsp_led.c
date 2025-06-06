// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include "bsp/led.h"
#include "bsp/tanmatsu.h"
#include "esp_check.h"
#include "esp_err.h"

static char const* TAG = "BSP: LEDs";

esp_err_t bsp_led_initialize(void) {
    return ESP_OK;
}

esp_err_t bsp_led_write(uint8_t* data, uint32_t length) {
    return ESP_OK;
}
