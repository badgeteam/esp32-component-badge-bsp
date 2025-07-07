// SPDX-FileCopyrightText: 2024-2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "bsp/power.h"
#include "esp_check.h"
#include "esp_err.h"

static char const* TAG = "BSP: power";

esp_err_t bsp_power_initialize(void) {
    return ESP_OK;
}

esp_err_t bsp_power_get_battery_information(bsp_power_battery_information_t* out_information) {
    ESP_RETURN_ON_FALSE(out_information, ESP_ERR_INVALID_ARG, TAG, "Information output argument is NULL");
    out_information->type                     = "LiPo";
    out_information->power_supply_available   = false;
    out_information->battery_available        = false;
    out_information->charging_disabled        = false;
    out_information->battery_charging         = false;
    out_information->maximum_charging_current = 0;
    out_information->current_charging_current = 0;
    out_information->voltage                  = 0;
    out_information->charging_target_voltage  = 4200;
    out_information->remaining_percentage     = 0;
    return ESP_OK;
}

esp_err_t bsp_power_get_battery_voltage(uint16_t* out_millivolt) {
    ESP_RETURN_ON_FALSE(out_millivolt, ESP_ERR_INVALID_ARG, TAG, "Millivolt output argument is NULL");
    *out_millivolt = 0;
    return ESP_OK;
}

esp_err_t bsp_power_get_system_voltage(uint16_t* out_millivolt) {
    ESP_RETURN_ON_FALSE(out_millivolt, ESP_ERR_INVALID_ARG, TAG, "Millivolt output argument is NULL");
    *out_millivolt = 0;
    return ESP_OK;
}

esp_err_t bsp_power_get_input_voltage(uint16_t* out_millivolt) {
    ESP_RETURN_ON_FALSE(out_millivolt, ESP_ERR_INVALID_ARG, TAG, "Millivolt output argument is NULL");
    *out_millivolt = 0;
    return ESP_OK;
}

esp_err_t bsp_power_get_charging_configuration(bool* out_disabled, uint16_t* out_current) {
    if (out_disabled) {
        *out_disabled = false;
    }
    if (out_current) {
        *out_current = 0;
    }
    return ESP_OK;
}

esp_err_t bsp_power_configure_charging(bool disable, uint16_t current) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_power_get_radio_state(bsp_radio_state_t* out_state) {
    ESP_RETURN_ON_FALSE(out_state, ESP_ERR_INVALID_ARG, TAG, "State output argument is NULL");
    *out_state = BSP_POWER_RADIO_STATE_APPLICATION;
    return ESP_OK;
}

esp_err_t bsp_power_set_radio_state(bsp_radio_state_t state) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_power_off(bool enable_alarm_wakeup) {
    return ESP_ERR_NOT_SUPPORTED;
}
