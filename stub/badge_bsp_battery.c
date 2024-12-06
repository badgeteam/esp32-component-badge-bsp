// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2024 Orange-Murker
// SPDX-License-Identifier: MIT

#include "bsp/battery.h"
#include "esp_err.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

esp_err_t __attribute__((weak)) bsp_battery_is_charging(bool *charging) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_battery_get_voltage(uint16_t *bat_mv) {
    return ESP_ERR_NOT_SUPPORTED;
}
