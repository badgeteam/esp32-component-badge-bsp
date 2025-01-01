// Board support package API: MCH2022 implementation
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
#include "bsp/mch2022.h"
#include "driver/gpio.h"
#include "driver/i2c_types.h"
#include "esp_check.h"
#include "esp_err.h"
#include "freertos/semphr.h"
#include "mch2022_hardware.h"
#include "rp2040.h"

static char const* TAG = "BSP device";

static i2c_master_bus_handle_t i2c_handle = NULL;
static SemaphoreHandle_t i2c_semaphore = NULL;

static RP2040 rp2040;
static bool rp2040_initialised = false;

static char const device_name[] = "MCH2022 badge";
static char const device_manufacturer[] = "Badge.Team";

esp_err_t bsp_mch2022_coprocessor_get_handle(RP2040* handle) {
    if (!rp2040_initialised) {
        return ESP_FAIL;
    }
    *handle = rp2040;
    return ESP_OK;
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

esp_err_t bsp_device_initialize(void) {
    gpio_install_isr_service(0);

    ESP_RETURN_ON_ERROR(bsp_display_initialize(), TAG, "Display failed to initialize");
    ESP_RETURN_ON_ERROR(bsp_i2c_primary_bus_initialize(), TAG, "Primary I2C bus failed to initialize");

    ESP_RETURN_ON_ERROR(bsp_i2c_primary_bus_get_handle(&i2c_handle), TAG, "Could not get the I2C handle");
    ESP_RETURN_ON_ERROR(bsp_i2c_primary_bus_get_semaphore(&i2c_semaphore), TAG, "Could not get the I2C semaphore");

    rp2040.i2c_bus_handle = i2c_handle;
    rp2040.i2c_address = BSP_RP2040_I2C_ADDR;
    rp2040.pin_interrupt = BSP_RP2040_INTR_PIN;
    rp2040.callback = bsp_mch2022_coprocessor_input_callback;
    rp2040.i2c_semaphore = i2c_semaphore;
    ESP_RETURN_ON_ERROR(rp2040_init(&rp2040), TAG, "Failed to initialise the coprocessor");
    rp2040_initialised = true;

    ESP_RETURN_ON_ERROR(bsp_input_initialize(), TAG, "Failed to initialize BSP input framework");

    return ESP_OK;
}
