#include "bsp/i2c.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include <stdbool.h>
#include <stdint.h>

static char const *TAG = "BSP I2C";

// Primary I2C bus

static i2c_master_bus_handle_t i2c_bus_handle_internal   = NULL;
static SemaphoreHandle_t       i2c_concurrency_semaphore = NULL;

i2c_master_bus_config_t i2c_master_config_internal = {
    .clk_source                   = I2C_CLK_SRC_DEFAULT,
    .i2c_port                     = 0,
    .scl_io_num                   = 10,
    .sda_io_num                   = 9,
    .glitch_ignore_cnt            = 7,
    .flags.enable_internal_pullup = true,
};

esp_err_t bsp_i2c_primary_bus_initialize(void) {
    ESP_RETURN_ON_ERROR(
        i2c_new_master_bus(&i2c_master_config_internal, &i2c_bus_handle_internal),
        TAG,
        "Failed to initialize I2C bus"
    );
    i2c_concurrency_semaphore = xSemaphoreCreateBinary();
    if (i2c_concurrency_semaphore == NULL) {
        return ESP_ERR_NO_MEM;
    }
    xSemaphoreGive(i2c_concurrency_semaphore);
    return ESP_OK;
}

esp_err_t bsp_i2c_primary_bus_get_handle(i2c_master_bus_handle_t *handle) {
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *handle = i2c_bus_handle_internal;
    return ESP_OK;
}

esp_err_t bsp_i2c_primary_bus_get_semaphore(SemaphoreHandle_t *semaphore) {
    if (semaphore == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *semaphore = i2c_concurrency_semaphore;
    return ESP_OK;
}
