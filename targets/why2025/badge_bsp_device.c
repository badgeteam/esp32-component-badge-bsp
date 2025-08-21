// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "bsp/device.h"
#include "bsp/i2c.h"
#include "bsp/macro.h"
#include "esp_err.h"

static i2c_master_bus_handle_t i2c_bus_handle_internal   = NULL;
static SemaphoreHandle_t       i2c_concurrency_semaphore = NULL;

static char const device_name[]         = "WHY2025";
static char const device_manufacturer[] = "WHY2025 team:badge";

esp_err_t bsp_device_initialize_custom(void) {
    BSP_RETURN_ON_FAILURE(bsp_i2c_primary_bus_get_handle(&i2c_bus_handle_internal));
    BSP_RETURN_ON_FAILURE(bsp_i2c_primary_bus_get_semaphore(&i2c_concurrency_semaphore));
    return ESP_OK;
}

bool bsp_device_get_initialized_without_coprocessor(void) {
    return false;
}

esp_err_t bsp_device_get_name(char* output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_name, buffer_length);
    return ESP_OK;
}

esp_err_t bsp_device_get_manufacturer(char* output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_manufacturer, buffer_length);
    return ESP_OK;
}
