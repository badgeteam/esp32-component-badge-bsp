// Board support package API: Tanmatsu implementation
// SPDX-FileCopyrightText: 2024 Orange-Murker
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stdint.h>
#include "bsp/battery.h"
#include "bsp/tanmatsu.h"
#include "esp_check.h"
#include "tanmatsu_coprocessor.h"

static char const* TAG = "BSP Battery";

esp_err_t bsp_battery_is_charging(bool* charging) {
    if (charging == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get the coprocessor handle");

    bool    battery_attached;
    bool    charging_disabled;
    bool    usb_attached;
    uint8_t charging_status;
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_pmic_charging_status(handle, &battery_attached, &usb_attached,
                                                                      &charging_disabled, &charging_status),
                        TAG, "Failed to get the battery charging status");

    *charging = charging_status == TANMATSU_CHARGE_STATUS_PRE_CHARGING ||
                charging_status == TANMATSU_CHARGE_STATUS_FAST_CHARGING;

    return ESP_OK;
}

esp_err_t bsp_battery_get_voltage(uint16_t* bat_mv) {
    if (bat_mv == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get the coprocessor handle");

    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_pmic_vbat(handle, bat_mv), TAG, "Failed to get the battery voltage");

    return ESP_OK;
}
