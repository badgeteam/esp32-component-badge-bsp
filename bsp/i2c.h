#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Badge BSP
// I2C APIs

#ifdef ESP_PLATFORM
/// @brief Initialize the primary I2C bus
/// @return ESP-IDF error code
esp_err_t bsp_i2c_primary_bus_initialize(void);

/// @brief Get the primary I2C bus handle
/// @return ESP-IDF error code
esp_err_t bsp_i2c_primary_bus_get_handle(i2c_master_bus_handle_t* handle);

/// @brief Get the primary I2C bus concurrency semaphore handle
/// @return ESP-IDF error code
esp_err_t bsp_i2c_primary_bus_get_semaphore(SemaphoreHandle_t* semaphore);
#endif

/// @brief Claim private access to the primary I2C bus
/// @return ESP-IDF error code
esp_err_t bsp_i2c_primary_bus_claim(void);

/// @brief Release private access to the primary I2C bus
/// @return ESP-IDF error code
esp_err_t bsp_i2c_primary_bus_release(void);
