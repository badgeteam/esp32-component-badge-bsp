// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "bsp/i2c.h"
#include "bsp/input.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/queue.h"

static char const* TAG = "BSP: INPUT";

static QueueHandle_t event_queue = NULL;

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
