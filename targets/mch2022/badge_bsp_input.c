// Board support package API: MCH2022 implementation
// SPDX-FileCopyrightText: 2024 Orange-Murker
// SPDX-License-Identifier: MIT

#include "bsp/input.h"
#include "bsp/mch2022.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "portmacro.h"
#include "rp2040.h"

#include <inttypes.h>
#include <stdint.h>

static char const *TAG = "BSP INPUT";

static RP2040 rp2040;

static QueueHandle_t event_queue = NULL;

void bsp_mch2022_coprocessor_input_callback(rp2040_input_t input, bool state) {
    bsp_input_event_t event = {0};
    switch (input) {
        case RP2040_INPUT_BUTTON_HOME:
            event.type                  = INPUT_EVENT_TYPE_NAVIGATION;
            event.args_navigation.key   = BSP_INPUT_NAVIGATION_KEY_F1;
            event.args_navigation.state = state;
            break;
        case RP2040_INPUT_BUTTON_MENU:
            event.type                  = INPUT_EVENT_TYPE_NAVIGATION;
            event.args_navigation.key   = BSP_INPUT_NAVIGATION_KEY_F2;
            event.args_navigation.state = state;
            break;
        case RP2040_INPUT_BUTTON_SELECT:
            event.type                  = INPUT_EVENT_TYPE_NAVIGATION;
            event.args_navigation.key   = BSP_INPUT_NAVIGATION_KEY_F3;
            event.args_navigation.state = state;
            break;
        case RP2040_INPUT_BUTTON_START:
            event.type                  = INPUT_EVENT_TYPE_NAVIGATION;
            event.args_navigation.key   = BSP_INPUT_NAVIGATION_KEY_F4;
            event.args_navigation.state = state;
            break;
        case RP2040_INPUT_BUTTON_ACCEPT:
            event.type                  = INPUT_EVENT_TYPE_NAVIGATION;
            event.args_navigation.key   = BSP_INPUT_NAVIGATION_KEY_GAMEPAD_A;
            event.args_navigation.state = state;
            break;
        case RP2040_INPUT_BUTTON_BACK:
            event.type                  = INPUT_EVENT_TYPE_NAVIGATION;
            event.args_navigation.key   = BSP_INPUT_NAVIGATION_KEY_GAMEPAD_B;
            event.args_navigation.state = state;
            break;

        case RP2040_INPUT_JOYSTICK_LEFT:
            event.type                  = INPUT_EVENT_TYPE_NAVIGATION;
            event.args_navigation.key   = BSP_INPUT_NAVIGATION_KEY_LEFT;
            event.args_navigation.state = state;
            break;
        case RP2040_INPUT_JOYSTICK_DOWN:
            event.type                  = INPUT_EVENT_TYPE_NAVIGATION;
            event.args_navigation.key   = BSP_INPUT_NAVIGATION_KEY_DOWN;
            event.args_navigation.state = state;
            break;
        case RP2040_INPUT_JOYSTICK_UP:
            event.type                  = INPUT_EVENT_TYPE_NAVIGATION;
            event.args_navigation.key   = BSP_INPUT_NAVIGATION_KEY_UP;
            event.args_navigation.state = state;
            break;
        case RP2040_INPUT_JOYSTICK_RIGHT:
            event.type                  = INPUT_EVENT_TYPE_NAVIGATION;
            event.args_navigation.key   = BSP_INPUT_NAVIGATION_KEY_RIGHT;
            event.args_navigation.state = state;
            break;
        case RP2040_INPUT_JOYSTICK_PRESS:
            event.type                  = INPUT_EVENT_TYPE_NAVIGATION;
            event.args_navigation.key   = BSP_INPUT_NAVIGATION_KEY_RETURN;
            event.args_navigation.state = state;
            break;
        default: ESP_LOGW(TAG, "RP2040 event not handled. Event: %d State: %d", input, state); return;
    }

    xQueueSend(event_queue, &event, portMAX_DELAY);
}

esp_err_t bsp_input_initialize(void) {
    bsp_mch2022_coprocessor_get_handle(&rp2040);

    if (event_queue == NULL) {
        event_queue = xQueueCreate(32, sizeof(bsp_input_event_t));
        ESP_RETURN_ON_FALSE(event_queue, ESP_ERR_NO_MEM, TAG, "Failed to create input event queue");
    }

    return ESP_OK;
}

esp_err_t bsp_input_get_queue(QueueHandle_t *out_queue) {
    if (out_queue == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (event_queue == NULL) {
        return ESP_FAIL;
    }

    *out_queue = event_queue;

    return ESP_OK;
}

bool needs_on_screen_keyboard() {
	return true;
}
