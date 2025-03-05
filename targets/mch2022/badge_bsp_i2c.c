// Board support package API: MCH2022 implementation
// SPDX-FileCopyrightText: 2024 Orange-Murker
// SPDX-License-Identifier: MIT

#include "driver/i2c_master.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "mch2022_hardware.h"

static char const* TAG = "BSP I2C";

static i2c_master_bus_handle_t i2c_bus_handle = NULL;
static SemaphoreHandle_t       i2c_semaphore  = NULL;

i2c_master_bus_config_t i2c_master_config_internal = {
    .i2c_port                     = BSP_I2C_BUS,
    .sda_io_num                   = BSP_I2C_SDA_PIN,
    .scl_io_num                   = BSP_I2C_SCL_PIN,
    .clk_source                   = I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt            = 7,
    .flags.enable_internal_pullup = false,
};

esp_err_t bsp_i2c_primary_bus_initialize(void) {
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&i2c_master_config_internal, &i2c_bus_handle), TAG,
                        "Failed to initialize I2C bus");
    i2c_semaphore = xSemaphoreCreateBinary();
    if (i2c_semaphore == NULL) {
        return ESP_ERR_NO_MEM;
    }
    xSemaphoreGive(i2c_semaphore);
    return ESP_OK;
}

esp_err_t bsp_i2c_primary_bus_get_handle(i2c_master_bus_handle_t* handle) {
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *handle = i2c_bus_handle;
    return ESP_OK;
}

esp_err_t bsp_i2c_primary_bus_get_semaphore(SemaphoreHandle_t* semaphore) {
    if (semaphore == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *semaphore = i2c_semaphore;
    return ESP_OK;
}

esp_err_t bsp_i2c_primary_bus_claim(void) {
    if (i2c_semaphore != NULL) {
        xSemaphoreTake(i2c_semaphore, portMAX_DELAY);
    } else {
        ESP_LOGW(TAG, "No concurrency semaphore");
    }
    return ESP_OK;
}

esp_err_t bsp_i2c_primary_bus_release(void) {
    if (i2c_semaphore != NULL) {
        xSemaphoreGive(i2c_semaphore);
    }
    return ESP_OK;
}
