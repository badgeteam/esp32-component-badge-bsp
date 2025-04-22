// Board support package API: Tanmatsu implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-FileCopyrightText: 2024 Orange-Murker
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "bsp/device.h"
#include "bsp/display.h"
#include "bsp/i2c.h"
#include "bsp/input.h"
#include "bsp/tanmatsu.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "tanmatsu_coprocessor.h"
#include "tanmatsu_hardware.h"

static char const* TAG = "BSP device";

static i2c_master_bus_handle_t       i2c_bus_handle_internal         = NULL;
static SemaphoreHandle_t             i2c_concurrency_semaphore       = NULL;
static tanmatsu_coprocessor_handle_t coprocessor_handle              = NULL;
static bool                          initialized_without_coprocessor = false;

#if defined(CONFIG_BSP_TARGET_TANMATSU)
static char const device_name[]         = "Tanmatsu";
static char const device_manufacturer[] = "Nicolai Electronics";
#elif defined(CONFIG_BSP_TARGET_HACKERHOTEL_2026)
static char const device_name[]         = "Hackerhotel 2026 badge";
static char const device_manufacturer[] = "Badge.Team";
#else
static char const device_name[]         = "Konsool";
static char const device_manufacturer[] = "Badge.Team";
#endif

esp_err_t bsp_tanmatsu_coprocessor_get_handle(tanmatsu_coprocessor_handle_t* handle) {
    if (coprocessor_handle == NULL) {
        return ESP_FAIL;
    }
    *handle = coprocessor_handle;
    return ESP_OK;
}

esp_err_t bsp_device_initialize(void) {
    gpio_install_isr_service(0);

    ESP_RETURN_ON_ERROR(bsp_display_initialize(), TAG, "Display failed to initialize");
    ESP_RETURN_ON_ERROR(bsp_i2c_primary_bus_initialize(), TAG, "Primary I2C bus failed to initialize");
    ESP_RETURN_ON_ERROR(bsp_i2c_primary_bus_get_handle(&i2c_bus_handle_internal), TAG, "Failed to get I2C bus handle");
    ESP_RETURN_ON_ERROR(bsp_i2c_primary_bus_get_semaphore(&i2c_concurrency_semaphore), TAG,
                        "Failed to get I2C bus semaphore");
    ESP_RETURN_ON_ERROR(bsp_input_initialize(), TAG, "Failed to initialize BSP input framework");

    initialized_without_coprocessor = true;

    tanmatsu_coprocessor_config_t coprocessor_config = {
        .int_io_num            = BSP_COPROCESSOR_INTERRUPT_PIN,
        .i2c_bus               = i2c_bus_handle_internal,
        .i2c_address           = BSP_COPROCESSOR_I2C_ADDRESS,
        .concurrency_semaphore = i2c_concurrency_semaphore,
        .on_keyboard_change    = bsp_internal_coprocessor_keyboard_callback,
        .on_input_change       = bsp_internal_coprocessor_input_callback,
        .on_faults_change      = bsp_internal_coprocessor_faults_callback,
    };

    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_initialize(&coprocessor_config, &coprocessor_handle), TAG,
                        "Failed to initialize coprocessor driver");

    initialized_without_coprocessor = false;
    return ESP_OK;
}

bool bsp_device_get_initialized_without_coprocessor(void) {
    return initialized_without_coprocessor;
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
