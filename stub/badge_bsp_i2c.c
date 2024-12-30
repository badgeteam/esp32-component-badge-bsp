// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/i2c.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

esp_err_t __attribute__((weak)) bsp_i2c_primary_bus_initialize(void) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_i2c_primary_bus_get_handle(i2c_master_bus_handle_t *handle) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_i2c_primary_bus_get_semaphore(SemaphoreHandle_t *semaphore) {
    return ESP_ERR_NOT_SUPPORTED;
}
