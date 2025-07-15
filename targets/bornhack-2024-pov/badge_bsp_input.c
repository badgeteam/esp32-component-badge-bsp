// SPDX-FileCopyrightText: 2025 Julian Schefferss
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include "bsp/input.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "hal/gpio_types.h"
#include "portmacro.h"
#include "targets/bornhack-2024-pov/bh24_hardware.h"

static char const TAG[] = "BSP: INPUT";

static QueueHandle_t event_queue = NULL;

static int const input_pins[] = {
    BSP_GPIO_BTN_UP,
    BSP_GPIO_BTN_DOWN,
    BSP_GPIO_BTN_SELECT,
};
static bsp_input_navigation_key_t const input_nav_keys[] = {
    BSP_INPUT_NAVIGATION_KEY_UP,
    BSP_INPUT_NAVIGATION_KEY_DOWN,
    BSP_INPUT_NAVIGATION_KEY_SELECT,
};

static IRAM_ATTR void gpio_isr(void* arg) {
    size_t            index = (size_t)arg;
    bsp_input_event_t event = {
        .type = INPUT_EVENT_TYPE_NAVIGATION,
        .args_navigation =
            {
                .key       = input_nav_keys[index],
                .modifiers = 0,
                .state     = !gpio_get_level(input_pins[index]),
            },
    };
    BaseType_t shouldYield = pdFALSE;
    xQueueSendFromISR(event_queue, &event, &shouldYield);
    if (shouldYield == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

esp_err_t bsp_input_initialize(void) {
    // Create input queue.
    if (event_queue) {
        return ESP_OK;
    }

    event_queue = xQueueCreate(32, sizeof(bsp_input_event_t));
    ESP_RETURN_ON_FALSE(event_queue, ESP_ERR_NO_MEM, TAG, "Failed to create input event queue");

    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(gpio_set_direction(input_pins[i], GPIO_MODE_INPUT));
        ESP_ERROR_CHECK(gpio_set_intr_type(input_pins[i], GPIO_INTR_ANYEDGE));
        ESP_ERROR_CHECK(gpio_isr_handler_add(input_pins[i], gpio_isr, (void*)i));
        ESP_ERROR_CHECK(gpio_intr_enable(input_pins[i]));
    }

    return ESP_OK;
}

esp_err_t bsp_input_get_queue(QueueHandle_t* out_queue) {
    if (out_queue == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (event_queue == NULL) {
        return ESP_FAIL;
    }
    *out_queue = event_queue;
    return ESP_OK;
}

bool bsp_input_needs_on_screen_keyboard(void) {
    return false;
}

esp_err_t bsp_input_get_backlight_brightness(uint8_t* out_percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_input_set_backlight_brightness(uint8_t percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_input_read_navigation_key(bsp_input_navigation_key_t key, bool* out_state) {
    switch (key) {
        default:
            return ESP_ERR_NOT_SUPPORTED;
        case BSP_INPUT_NAVIGATION_KEY_UP:
            *out_state = !gpio_get_level(BSP_GPIO_BTN_UP);
            return ESP_OK;
        case BSP_INPUT_NAVIGATION_KEY_DOWN:
            *out_state = !gpio_get_level(BSP_GPIO_BTN_DOWN);
            return ESP_OK;
        case BSP_INPUT_NAVIGATION_KEY_SELECT:
            *out_state = !gpio_get_level(BSP_GPIO_BTN_SELECT);
            return ESP_OK;
    }
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_input_read_action(bsp_input_action_type_t action, bool* out_state) {
    return ESP_ERR_NOT_SUPPORTED;
}
