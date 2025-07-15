// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include "bsp/input.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "hal/gpio_types.h"
#include "targets/bornhack-2024-pov/bh24_hardware.h"

static char const TAG[] = "BSP: INPUT";

static QueueHandle_t event_queue = NULL;

esp_err_t __attribute__((weak)) bsp_input_initialize(void) {
    // Create input queue.
    if (!event_queue) {
        event_queue = xQueueCreate(32, sizeof(bsp_input_event_t));
        ESP_RETURN_ON_FALSE(event_queue, ESP_ERR_NO_MEM, TAG, "Failed to create input event queue");
    }

    gpio_set_direction(BSP_GPIO_BTN_UP, GPIO_MODE_INPUT);
    gpio_set_direction(BSP_GPIO_BTN_DOWN, GPIO_MODE_INPUT);
    gpio_set_direction(BSP_GPIO_BTN_SELECT, GPIO_MODE_INPUT);

    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_input_get_queue(QueueHandle_t* out_queue) {
    if (out_queue == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (event_queue == NULL) {
        return ESP_FAIL;
    }
    *out_queue = event_queue;
    return ESP_OK;
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
    switch (key) {
        default:
            return ESP_ERR_NOT_SUPPORTED;
        case BSP_INPUT_NAVIGATION_KEY_UP:
            *out_state = gpio_get_level(BSP_GPIO_BTN_UP);
            return ESP_OK;
        case BSP_INPUT_NAVIGATION_KEY_DOWN:
            *out_state = gpio_get_level(BSP_GPIO_BTN_DOWN);
            return ESP_OK;
        case BSP_INPUT_NAVIGATION_KEY_SELECT:
            *out_state = gpio_get_level(BSP_GPIO_BTN_SELECT);
            return ESP_OK;
    }
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_input_read_action(bsp_input_action_type_t action, bool* out_state) {
    return ESP_ERR_NOT_SUPPORTED;
}
