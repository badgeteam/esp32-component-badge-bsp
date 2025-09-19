// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include "bsp/led.h"
#include "esp_err.h"

esp_err_t __attribute__((weak)) bsp_led_initialize(void) {
    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_led_write(const uint8_t* data, uint32_t length) {
    return ESP_ERR_NOT_SUPPORTED;
}
