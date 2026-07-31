// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include "bsp/i2c.h"
#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "why2025_hardware.h"

static char const* TAG = "BSP I2C";

// Primary I2C bus

static i2c_master_bus_handle_t i2c_bus_handle_internal   = NULL;
static SemaphoreHandle_t       i2c_concurrency_semaphore = NULL;

static uint8_t scan_i2c_bus(uint8_t* buf, uint8_t num) {
    uint8_t device_count = 0;
    esp_err_t ret = ESP_OK;
    for (uint8_t dev_address = 1; dev_address < 127; dev_address++) {
        ret = i2c_master_probe(i2c_bus_handle_internal, dev_address, 100);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "found i2c device address = 0x%02x", dev_address);
            if (buf != NULL && device_count < num) {
                *(buf + device_count) = dev_address;
            }
            device_count++;
        }
    }
    return device_count;
}

i2c_master_bus_config_t i2c_master_config_internal = {
    .clk_source                   = I2C_CLK_SRC_DEFAULT,
    .i2c_port                     = BSP_I2C_BUS,
    .scl_io_num                   = BSP_I2C_SCL_PIN,
    .sda_io_num                   = BSP_I2C_SDA_PIN,
    .glitch_ignore_cnt            = 7,
    .flags.enable_internal_pullup = false,
};

esp_err_t bsp_i2c_primary_bus_initialize(void) {
    i2c_concurrency_semaphore = xSemaphoreCreateBinary();
    if (i2c_concurrency_semaphore == NULL) {
        return ESP_ERR_NO_MEM;
    }
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&i2c_master_config_internal, &i2c_bus_handle_internal), TAG,
                        "Failed to initialize I2C bus");
    uint8_t addresses_found[20]= {0};

    uint8_t                    device_count = scan_i2c_bus(addresses_found, sizeof(addresses_found));
    ESP_LOGI(TAG, "Found %d I2C devices", device_count);
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
    }
    return ESP_OK;
}

esp_err_t bsp_i2c_primary_bus_release(void) {
    if (i2c_concurrency_semaphore != NULL) {
        xSemaphoreGive(i2c_concurrency_semaphore);
    }
    return ESP_OK;
}
