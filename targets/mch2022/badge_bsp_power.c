// Board support package API: MCH2022 implementation
// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "bsp/mch2022.h"
#include "bsp/power.h"
#include "esp_check.h"
#include "esp_err.h"
#include "rp2040.h"

static char const* TAG = "BSP: power";

esp_err_t bsp_power_initialize(void) {
    return ESP_OK;
}

esp_err_t bsp_power_get_button_state(bool* pressed) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_power_get_battery_information(bsp_power_battery_information_t* out_information) {
    if (out_information == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    RP2040 handle;
    ESP_RETURN_ON_ERROR(bsp_mch2022_coprocessor_get_handle(&handle), TAG, "Failed to get the coprocessor handle");
    uint8_t charging_state;
    ESP_RETURN_ON_ERROR(rp2040_get_charging(&handle, &charging_state), TAG, "Failed to read charging state");

    uint16_t battery_voltage;
    ESP_RETURN_ON_ERROR(bsp_power_get_battery_voltage(&battery_voltage), TAG, "Failed to get the battery voltage");
    uint16_t usb_voltage;
    ESP_RETURN_ON_ERROR(bsp_power_get_input_voltage(&usb_voltage), TAG, "Failed to read input voltage");

    double battery_voltage_double = (double)battery_voltage / 1000.0;  // [v]
    double battery_percentage     = 123.0 - 123.0 / pow(1.0 + pow(battery_voltage_double / 3.7, 80.0), 0.165);

    out_information->type                     = "LiPo";
    out_information->power_supply_available   = (usb_voltage > 4400);
    out_information->battery_available        = true;
    out_information->charging_disabled        = false;
    out_information->battery_charging         = charging_state ? true : false;
    out_information->maximum_charging_current = 500;
    out_information->current_charging_current = charging_state ? 500 : 0;
    out_information->voltage                  = battery_voltage;
    out_information->charging_target_voltage  = 4200;
    out_information->remaining_percentage     = battery_percentage;
    return ESP_OK;
}

esp_err_t bsp_power_get_battery_voltage(uint16_t* out_millivolt) {
    if (out_millivolt == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    RP2040 handle;
    ESP_RETURN_ON_ERROR(bsp_mch2022_coprocessor_get_handle(&handle), TAG, "Failed to get the coprocessor handle");

    uint16_t vbat_raw;
    ESP_RETURN_ON_ERROR(rp2040_read_vbat_raw(&handle, &vbat_raw), TAG, "Failed to get the battery voltage");

    // 12-bit ADC with 3.3v vref
    // Connected through 100k/100k divider
    uint32_t vbat_mv = (((uint32_t)vbat_raw) * 2 * 3300) / (1 << 12);

    *out_millivolt = (uint16_t)vbat_mv;
    return ESP_OK;
}

esp_err_t bsp_power_get_system_voltage(uint16_t* out_millivolt) {
    if (out_millivolt == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *out_millivolt = 3300;
    return ESP_OK;
}

esp_err_t bsp_power_get_input_voltage(uint16_t* out_millivolt) {
    if (out_millivolt == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    RP2040 handle;
    ESP_RETURN_ON_ERROR(bsp_mch2022_coprocessor_get_handle(&handle), TAG, "Failed to get the coprocessor handle");

    uint16_t vbat_raw;
    ESP_RETURN_ON_ERROR(rp2040_read_vusb_raw(&handle, &vbat_raw), TAG, "Failed to get the battery voltage");

    // 12-bit ADC with 3.3v vref
    // Connected through 100k/100k divider
    uint32_t vbat_mv = (((uint32_t)vbat_raw) * 2 * 3300) / (1 << 12);

    *out_millivolt = (uint16_t)vbat_mv;
    return ESP_OK;
}

esp_err_t bsp_power_get_charging_configuration(bool* out_disabled, uint16_t* out_current) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_power_configure_charging(bool disable, uint16_t current) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_power_get_usb_host_boost_enabled(bool* out_enabled) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_power_set_usb_host_boost_enabled(bool enable) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_power_get_radio_state(bsp_radio_state_t* out_state) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_power_set_radio_state(bsp_radio_state_t state) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_power_off(bool enable_alarm_wakeup) {
    return ESP_ERR_NOT_SUPPORTED;
}
