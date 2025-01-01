// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/leds.h"
#include "esp_err.h"

#include <stdint.h>

esp_err_t __attribute__((weak)) bsp_led_initialize(void) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_led_write(uint8_t *data, uint32_t length) {
    return ESP_ERR_NOT_SUPPORTED;
}
