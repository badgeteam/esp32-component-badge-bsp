// Board support package API: WHY2025 implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/device.h"
#include "bsp/display.h"
#include "bsp/i2c.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "tanmatsu_coprocessor.h"
#include "why2025_hardware.h"

#include <stdbool.h>
#include <stdint.h>

#include <string.h>

static char const *TAG = "BSP device";

static i2c_master_bus_handle_t       i2c_bus_handle_internal   = NULL;
static SemaphoreHandle_t             i2c_concurrency_semaphore = NULL;
static tanmatsu_coprocessor_handle_t coprocessor_handle        = NULL;

static char const device_name[]         = "WHY2025 badge";
static char const device_manufacturer[] = "Badge.Team";

static void coprocessor_keyboard_callback(tanmatsu_coprocessor_handle_t handle, tanmatsu_coprocessor_keys_t *prev_keys, tanmatsu_coprocessor_keys_t *keys) {
    ESP_LOGI(TAG, "Coprocessor keyboard callback stub called\r\n");
}

static void coprocessor_input_callback(tanmatsu_coprocessor_handle_t handle, tanmatsu_coprocessor_inputs_t *prev_inputs, tanmatsu_coprocessor_inputs_t *inputs) {
    ESP_LOGI(TAG, "Coprocessor input callback stub called\r\n");
}

static void coprocessor_faults_callback(tanmatsu_coprocessor_handle_t handle, tanmatsu_coprocessor_pmic_faults_t *prev_faults, tanmatsu_coprocessor_pmic_faults_t *faults) {
    ESP_LOGI(TAG, "Coprocessor faults callback stub called\r\n");
}

esp_err_t bsp_device_initialize(void) {
    tanmatsu_coprocessor_config_t coprocessor_config = {
        .int_io_num            = BSP_COPROCESSOR_INTERRUPT_PIN,
        .i2c_bus               = i2c_bus_handle_internal,
        .i2c_address           = BSP_COPROCESSOR_I2C_ADDRESS,
        .concurrency_semaphore = i2c_concurrency_semaphore,
        .on_keyboard_change    = coprocessor_keyboard_callback,
        .on_input_change       = coprocessor_input_callback,
        .on_faults_change      = coprocessor_faults_callback,
    };

    ESP_RETURN_ON_ERROR(bsp_display_initialize(), TAG, "Display failed to initialize");
    ESP_RETURN_ON_ERROR(bsp_i2c_primary_bus_initialize(), TAG, "Primary I2C bus failed to initialize");
    ESP_RETURN_ON_ERROR(bsp_i2c_primary_bus_get_handle(&i2c_bus_handle_internal), TAG, "Failed to get I2C bus handle");
    ESP_RETURN_ON_ERROR(bsp_i2c_primary_bus_get_semaphore(&i2c_concurrency_semaphore), TAG, "Failed to get I2C bus semaphore");
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_initialize(&coprocessor_config, &coprocessor_handle), TAG, "Failed to initialize coprocessor driver");
    return ESP_OK;
}

esp_err_t bsp_device_get_name(char *output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_name, buffer_length);
    return ESP_OK;
}

esp_err_t bsp_device_get_manufacturer(char *output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_manufacturer, buffer_length);
    return ESP_OK;
}
