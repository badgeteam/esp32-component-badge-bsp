// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/power.h"
#include "esp_err.h"

#include <stdbool.h>
#include <stdint.h>

esp_err_t __attribute__((weak)) bsp_power_initialize(void) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_power_get_button_state(bool *pressed) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_power_get_battery_information(bsp_power_battery_information_t *out_information) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_power_get_battery_voltage(uint16_t *out_millivolt) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_power_get_system_voltage(uint16_t *out_millivolt) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_power_get_input_voltage(uint16_t *out_millivolt) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_power_get_charging_configuration(bool *out_disabled, uint16_t *out_current) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_power_configure_charging(bool disable, uint16_t current) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_power_get_usb_host_boost_enabled(bool *out_enabled) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_power_set_usb_host_boost_enabled(bool enable) {
    return ESP_ERR_NOT_SUPPORTED;
}
