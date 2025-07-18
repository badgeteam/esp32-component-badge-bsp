// Board support package API: Tanmatsu implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stdint.h>
#include "bsp/i2c.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "tanmatsu_hardware.h"

static char const* TAG = "BSP I2C";

// Primary I2C bus

static i2c_master_bus_handle_t i2c_bus_handle_internal   = NULL;
static SemaphoreHandle_t       i2c_concurrency_semaphore = NULL;

i2c_master_bus_config_t i2c_master_config_internal = {
    .clk_source                   = I2C_CLK_SRC_DEFAULT,
    .i2c_port                     = BSP_I2C_INTERNAL_BUS,
    .scl_io_num                   = BSP_I2C_INTERNAL_SCL_PIN,
    .sda_io_num                   = BSP_I2C_INTERNAL_SDA_PIN,
    .glitch_ignore_cnt            = 7,
    .flags.enable_internal_pullup = true,
};

esp_err_t bsp_i2c_primary_bus_initialize(void) {
    i2c_concurrency_semaphore = xSemaphoreCreateBinary();
    if (i2c_concurrency_semaphore == NULL) {
        return ESP_ERR_NO_MEM;
    }
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&i2c_master_config_internal, &i2c_bus_handle_internal), TAG,
                        "Failed to initialize I2C bus");
    xSemaphoreGive(i2c_concurrency_semaphore);
    return ESP_OK;
}

esp_err_t bsp_i2c_primary_bus_get_handle(i2c_master_bus_handle_t* handle) {
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *handle = i2c_bus_handle_internal;
    return ESP_OK;
}

esp_err_t bsp_i2c_primary_bus_get_semaphore(SemaphoreHandle_t* semaphore) {
    if (semaphore == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *semaphore = i2c_concurrency_semaphore;
    return ESP_OK;
}

esp_err_t bsp_i2c_primary_bus_claim(void) {
    if (i2c_concurrency_semaphore != NULL) {
        xSemaphoreTake(i2c_concurrency_semaphore, portMAX_DELAY);
    } else {
        ESP_LOGW(TAG, "No concurrency semaphore");
    }
    return ESP_OK;
}

esp_err_t bsp_i2c_primary_bus_release(void) {
    if (i2c_concurrency_semaphore != NULL) {
        xSemaphoreGive(i2c_concurrency_semaphore);
    }
    return ESP_OK;
}
