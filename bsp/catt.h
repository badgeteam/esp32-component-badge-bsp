#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"

typedef enum {
    CATT_PIN_D0  = 0,  // I2C SCL
    CATT_PIN_D1  = 1,
    CATT_PIN_D2  = 2,
    CATT_PIN_D3  = 3,
    CATT_PIN_D4  = 4,  // I2C SDA
    CATT_PIN_D5  = 5,
    CATT_PIN_D6  = 6,
    CATT_PIN_D7  = 7,
    CATT_PIN_SCL = 0,
    CATT_PIN_SDA = 4,
} catt_pin_num_t;

/// @brief Enable or disable (use as GPIOs) the I2C bus on the D0 (SCL) and D4 (SDA) pins
/// @return ESP-IDF error code
esp_err_t bsp_catt_set_i2c_enabled(bool enable);

/// @brief Get the state of the I2C bus on the D0 (SCL) and D4 (SDA) pins
/// @return ESP-IDF error code
esp_err_t bsp_catt_get_i2c_enabled(bool* out_enabled);

/// @brief Get the handle of the I2C bus on the D0 (SCL) and D4 (SDA) pins
/// @return ESP-IDF error code
esp_err_t bsp_catt_i2c_bus_get_handle(i2c_master_bus_handle_t* out_handle);

/// @brief Get the GPIO pin number of a CATT data pin
/// @return ESP-IDF error code
gpio_num_t bsp_catt_get_gpio(catt_pin_num_t pin);
