// Board support package API: MCH2022 implementation
// SPDX-FileCopyrightText: 2024 Orange-Murker
// SPDX-License-Identifier: MIT

#include "bsp/input.h"
#include "esp_check.h"
#include "esp_err.h"
#include "freertos/queue.h"

#include <inttypes.h>
#include <stdint.h>

static char const *TAG = "BSP INPUT";

static QueueHandle_t event_queue = NULL;

esp_err_t bsp_input_initialize(void) {
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
