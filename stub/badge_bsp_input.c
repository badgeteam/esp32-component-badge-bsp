// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/input.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include <stdint.h>

static char const *TAG = "BSP INPUT";

esp_err_t __attribute__((weak)) bsp_input_initialize(void) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_input_get_queue(QueueHandle_t *out_queue) {
    return ESP_ERR_NOT_SUPPORTED;
}

bool __attribute__((weak)) needs_on_screen_keyboard() {
	return false;
}
