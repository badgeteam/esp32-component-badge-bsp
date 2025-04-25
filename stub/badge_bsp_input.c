// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include "bsp/input.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

esp_err_t __attribute__((weak)) bsp_input_initialize(void) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_input_get_queue(QueueHandle_t* out_queue) {
    return ESP_ERR_NOT_SUPPORTED;
}

bool __attribute__((weak)) bsp_input_needs_on_screen_keyboard(void) {
    return false;
}

esp_err_t __attribute__((weak)) bsp_input_get_backlight_brightness(uint8_t* out_percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_input_set_backlight_brightness(uint8_t percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_input_read_navigation_key(bsp_input_navigation_key_t key, bool* out_state) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_input_read_action(bsp_input_action_type_t action, bool* out_state) {
    return ESP_ERR_NOT_SUPPORTED;
}
