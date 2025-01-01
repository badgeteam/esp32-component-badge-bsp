// Board support package API: Tanmatsu implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/led.h"
#include "bsp/tanmatsu.h"
#include "esp_check.h"
#include "esp_err.h"
#include "tanmatsu_coprocessor.h"

#include <stdint.h>

static char const *TAG = "BSP: LEDs";

esp_err_t bsp_led_initialize(void) {
    return ESP_OK;
}

esp_err_t bsp_led_write(uint8_t *data, uint32_t length) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    return tanmatsu_coprocessor_set_led_data(handle, data, length);
}
