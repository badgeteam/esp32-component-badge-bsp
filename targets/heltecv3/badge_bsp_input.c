// SPDX-FileCopyrightText: 2026 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include "bsp/input.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "heltecv3_hardware.h"

static char const* TAG = "BSP INPUT";

static QueueHandle_t event_queue       = NULL;
static bool          prev_button_state = false;

IRAM_ATTR static void button_interrupt_handler(void* pvParameters) {
    bool state = !gpio_get_level(BSP_GPIO_BTN);  // GPIO is active low
    if (state != prev_button_state) {
        prev_button_state                = state;
        bsp_input_event_t scancode_event = {
            .type                   = INPUT_EVENT_TYPE_SCANCODE,
            .args_scancode.scancode = BSP_INPUT_SCANCODE_ENTER | (state ? 0 : BSP_INPUT_SCANCODE_RELEASE_MODIFIER),
        };
        xQueueSendFromISR(event_queue, &scancode_event, false);
        bsp_input_event_t navigation_event = {
            .type                      = INPUT_EVENT_TYPE_NAVIGATION,
            .args_navigation.key       = BSP_INPUT_NAVIGATION_KEY_RETURN,
            .args_navigation.modifiers = 0,
            .args_navigation.state     = state,
        };
        xQueueSendFromISR(event_queue, &navigation_event, false);
    }
}

esp_err_t bsp_input_initialize(void) {
    if (event_queue == NULL) {
        event_queue = xQueueCreate(32, sizeof(bsp_input_event_t));
        ESP_RETURN_ON_FALSE(event_queue, ESP_ERR_NO_MEM, TAG, "Failed to create input event queue");
    }

    gpio_config_t int_pin_cfg = {
        .pin_bit_mask = BIT64(BSP_GPIO_BTN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = false,
        .pull_down_en = false,
        .intr_type    = GPIO_INTR_ANYEDGE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&int_pin_cfg), TAG, "Failed to configure button GPIO");
    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(BSP_GPIO_BTN, button_interrupt_handler, NULL), TAG,
                        "Failed to add interrupt handler for button GPIO");

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
    return true;
}

esp_err_t bsp_input_get_backlight_brightness(uint8_t* out_percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_input_set_backlight_brightness(uint8_t percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_input_read_navigation_key(bsp_input_navigation_key_t key, bool* out_state) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_input_read_scancode(bsp_input_scancode_t key, bool* out_state) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_input_read_action(bsp_input_action_type_t action, bool* out_state) {
    return ESP_ERR_NOT_SUPPORTED;
}
