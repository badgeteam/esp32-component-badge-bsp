// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "badge_bsp_input_hooks.h"
#include "bsp/i2c.h"
#include "bsp/input.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "hal/gpio_types.h"
#include "kami_hardware.h"
#include "mpr121.h"

static char const* TAG = "BSP: INPUT";

static QueueHandle_t   event_queue = NULL;
static mpr121_handle_t mpr121      = NULL;

static void mpr121_touch_callback(mpr121_handle_t handle, uint32_t previous_touch_state, uint32_t touch_state) {
    bsp_input_event_t event = {0};

    const bsp_input_navigation_key_t keys[] = {
        BSP_INPUT_NAVIGATION_KEY_GAMEPAD_A, BSP_INPUT_NAVIGATION_KEY_GAMEPAD_B, BSP_INPUT_NAVIGATION_KEY_START,
        BSP_INPUT_NAVIGATION_KEY_SELECT,    BSP_INPUT_NAVIGATION_KEY_DOWN,      BSP_INPUT_NAVIGATION_KEY_RIGHT,
        BSP_INPUT_NAVIGATION_KEY_UP,        BSP_INPUT_NAVIGATION_KEY_LEFT,      BSP_INPUT_NAVIGATION_KEY_MENU,
        BSP_INPUT_NAVIGATION_KEY_HOME,
    };

    for (uint8_t i = 0; i < 10; i++) {
        uint32_t mask = 1 << i;
        if ((previous_touch_state & mask) != (touch_state & mask)) {
            event.type                  = INPUT_EVENT_TYPE_NAVIGATION;
            event.args_navigation.key   = keys[i];
            event.args_navigation.state = (touch_state & mask) ? 1 : 0;

            // Process through hooks first; if consumed, don't queue
            if (!bsp_input_hooks_process(&event)) {
                xQueueSend(event_queue, &event, portMAX_DELAY);
            }
        }
    }
}

static void mpr121_input_callback(mpr121_handle_t handle, uint32_t previous_input_state, uint32_t input_state) {
    printf("MPR121 input state changed: %04" PRIx32 "\r\n", input_state);
}

esp_err_t bsp_input_initialize(void) {
    if (event_queue == NULL) {
        event_queue = xQueueCreate(32, sizeof(bsp_input_event_t));
        ESP_RETURN_ON_FALSE(event_queue, ESP_ERR_NO_MEM, TAG, "Failed to create input event queue");
    }

    static i2c_master_bus_handle_t i2c_bus_handle_internal   = NULL;
    static SemaphoreHandle_t       i2c_concurrency_semaphore = NULL;
    ESP_RETURN_ON_ERROR(bsp_i2c_primary_bus_get_handle(&i2c_bus_handle_internal), TAG, "Failed to get I2C bus handle");
    ESP_RETURN_ON_ERROR(bsp_i2c_primary_bus_get_semaphore(&i2c_concurrency_semaphore), TAG,
                        "Failed to get I2C bus semaphore");

    mpr121_config_t mpr121_config = {
        .int_io_num            = BSP_MPR121_INT_PIN,
        .i2c_bus               = i2c_bus_handle_internal,
        .i2c_address           = BSP_MPR121_I2C_ADDRESS,
        .concurrency_semaphore = i2c_concurrency_semaphore,
        .touch_callback        = mpr121_touch_callback,
        .input_callback        = mpr121_input_callback,
        .i2c_timeout           = 1000,
    };

    ESP_RETURN_ON_ERROR(mpr121_initialize(&mpr121_config, &mpr121), TAG, "Failed to initialize MPR121");

    /*mpr121_touch_set_baseline(mpr121_handle_t handle, uint8_t pin, uint8_t baseline);
    mpr121_touch_set_touch_threshold(mpr121_handle_t handle, uint8_t pin, uint8_t touch_threshold);
    mpr121_touch_set_release_threshold(mpr121_handle_t handle, uint8_t pin, uint8_t release_threshold);*/

    mpr121_gpio_set_mode(mpr121, BSP_MPR121_PIN_INPUT_CHRG, MPR121_INPUT_PULL_UP);
    mpr121_gpio_set_mode(mpr121, BSP_MPR121_PIN_INPUT_SD_DET, MPR121_INPUT_PULL_UP);

    mpr121_touch_configure(mpr121, 10, 0, true);  // Use first 10 electrodes for touch

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

esp_err_t bsp_input_read_navigation_key(bsp_input_navigation_key_t key, bool* out_state) {
    *out_state = false;
    return ESP_OK;
}

esp_err_t bsp_input_read_action(bsp_input_action_type_t action, bool* out_state) {
    *out_state = false;
    return ESP_OK;
}
