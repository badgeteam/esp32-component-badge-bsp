#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"

typedef enum {
    SAO_PIN_D0  = 0,
    SAO_PIN_D1  = 1,
} sao_pin_num_t;

/// @brief Get the handle of the I2C bus on SAO connector
/// @return ESP-IDF error code
esp_err_t bsp_sao_i2c_bus_get_handle(i2c_master_bus_handle_t* out_handle);

/// @brief Get the GPIO pin number of a SAO data pin
/// @return ESP-IDF error code
gpio_num_t bsp_sao_get_gpio(sao_pin_num_t pin);
