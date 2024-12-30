// Board support package API: WHY2025 implementation
// SPDX-FileCopyrightText: 2024 Orange-Murker
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/power.h"
#include "bsp/why2025.h"
#include "esp_check.h"
#include "esp_err.h"
#include "tanmatsu_coprocessor.h"

#include <stdbool.h>
#include <stdint.h>

#include <math.h>

static char const *TAG = "BSP: power";

esp_err_t bsp_power_initialize(void) {
    return ESP_OK;
}

esp_err_t bsp_power_get_battery_information(bsp_power_battery_information_t *out_information) {
    ESP_RETURN_ON_FALSE(out_information, ESP_ERR_INVALID_ARG, TAG, "Information output argument is NULL");

    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_why2025_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");

    bool    battery_attached;
    bool    charging_disabled;
    bool    usb_attached;
    uint8_t charging_status;
    ESP_RETURN_ON_ERROR(
        tanmatsu_coprocessor_get_pmic_charging_status(handle, &battery_attached, &usb_attached, &charging_disabled, &charging_status),
        TAG,
        "Failed to get the battery charging status"
    );

    bool    chrg_disabled;
    uint8_t chrg_speed;
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_pmic_charging_control(handle, &chrg_disabled, &chrg_speed), TAG, "Failed to get charging control status");

    uint32_t imax = 0;
    switch (chrg_speed) {
        case 0: imax = 500; break;
        case 1: imax = 1000; break;
        case 2: imax = 1500; break;
        case 3: imax = 2000; break;
        default: imax = 0; break;
    }

    uint16_t vbat;
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_pmic_vbat(handle, &vbat), TAG, "Failed to read battery voltage");

    double battery_voltage    = (double)vbat / 1000.0;
    double battery_percentage = 123.0 - 123.0 / pow(1.0 + pow(battery_voltage / 3.7, 80.0), 0.165);

    uint16_t ichgr;
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_pmic_ichgr(handle, &ichgr), TAG, "Failed to read charging current");

    out_information->type                     = "LiPo";
    out_information->power_supply_available   = usb_attached;
    out_information->battery_available        = battery_attached;
    out_information->charging_disabled        = chrg_disabled;
    out_information->battery_charging         = (charging_status == TANMATSU_CHARGE_STATUS_PRE_CHARGING || charging_status == TANMATSU_CHARGE_STATUS_FAST_CHARGING);
    out_information->maximum_charging_current = imax;
    out_information->current_charging_current = ichgr;
    out_information->voltage                  = vbat;
    out_information->charging_target_voltage  = 4200;
    out_information->remaining_percentage     = battery_percentage;
    return ESP_OK;
}

esp_err_t bsp_power_get_battery_voltage(uint16_t *out_millivolt) {
    ESP_RETURN_ON_FALSE(out_millivolt, ESP_ERR_INVALID_ARG, TAG, "Millivolt output argument is NULL");
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_why2025_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_pmic_vbat(handle, out_millivolt), TAG, "Failed to read battery voltage");
    return ESP_OK;
}

esp_err_t bsp_power_get_system_voltage(uint16_t *out_millivolt) {
    ESP_RETURN_ON_FALSE(out_millivolt, ESP_ERR_INVALID_ARG, TAG, "Millivolt output argument is NULL");
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_why2025_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_pmic_vsys(handle, out_millivolt), TAG, "Failed to read system voltage");
    return ESP_OK;
}

esp_err_t bsp_power_get_input_voltage(uint16_t *out_millivolt) {
    ESP_RETURN_ON_FALSE(out_millivolt, ESP_ERR_INVALID_ARG, TAG, "Millivolt output argument is NULL");
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_why2025_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_pmic_vbus(handle, out_millivolt), TAG, "Failed to read input voltage");
    return ESP_OK;
}

esp_err_t bsp_power_get_charging_configuration(bool *out_disabled, uint16_t *out_current) {
    bool                          disabled;
    uint8_t                       chrg_speed;
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_why2025_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_pmic_charging_control(handle, &disabled, &chrg_speed), TAG, "Failed to get charging configuration");
    if (out_disabled) {
        *out_disabled = disabled;
    }
    if (out_current) {
        switch (chrg_speed) {
            case 0: *out_current = 500; break;
            case 1: *out_current = 1000; break;
            case 2: *out_current = 1500; break;
            case 3: *out_current = 2000; break;
            default: *out_current = 0; break;
        }
    }
    return ESP_OK;
}

esp_err_t bsp_power_configure_charging(bool disable, uint16_t current) {
    uint8_t chrg_speed = 0;
    if (current >= 1000) {
        chrg_speed = 1;
    }
    if (current >= 1500) {
        chrg_speed = 2;
    }
    if (current >= 2000) {
        chrg_speed = 3;
    }
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_why2025_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_set_pmic_charging_control(handle, disable, chrg_speed), TAG, "Failed to configure charging");
    return ESP_OK;
}

/*esp_err_t bsp_power_get_usb_host_boost_enabled(bool *out_enabled) {
    ESP_RETURN_ON_FALSE(out_enabled, ESP_ERR_INVALID_ARG, TAG, "Enabled output argument is NULL");
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_why2025_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_pmic_otg_control(handle, out_enabled), TAG, "Failed to get USB host boot status");
    return ESP_OK;
}*/

esp_err_t bsp_power_set_usb_host_boost_enabled(bool enable) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_why2025_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_set_pmic_otg_control(handle, enable), TAG, "Failed to set USB host boot configuration");
    return ESP_OK;
}

esp_err_t bsp_power_get_radio_enabled(bool *out_enabled) {
    ESP_RETURN_ON_FALSE(out_enabled, ESP_ERR_INVALID_ARG, TAG, "Enabled output argument is NULL");
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_why2025_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    tanmatsu_coprocessor_radio_state_t status;
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_radio_state(handle, &status), TAG, "Failed to get radio status");
    *out_enabled = (status == tanmatsu_coprocessor_radio_state_enabled_application);
    return ESP_OK;
}

esp_err_t bsp_power_set_radio_enabled(bool enable) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_why2025_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    ESP_RETURN_ON_ERROR(
        tanmatsu_coprocessor_set_radio_state(handle, enable ? tanmatsu_coprocessor_radio_state_enabled_application : tanmatsu_coprocessor_radio_state_disabled),
        TAG,
        "Failed to set radio status"
    );
    return ESP_OK;
}
