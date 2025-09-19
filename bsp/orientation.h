#pragma once

#include <stdbool.h>
#include "esp_err.h"

/// @brief Enable the gyroscope
/// @return ESP-IDF error code
esp_err_t bsp_orientation_enable_gyroscope(void);

/// @brief Disable the gyroscope
/// @return ESP-IDF error code
esp_err_t bsp_orientation_disable_gyroscope(void);

/// @brief Enable the accelerometer
/// @return ESP-IDF error code
esp_err_t bsp_orientation_enable_accelerometer(void);

/// @brief Disable the accelerometer
/// @return ESP-IDF error code
esp_err_t bsp_orientation_disable_accelerometer(void);

/// @brief Get the current orientation data
/// @return ESP-IDF error code
esp_err_t bsp_orientation_get(bool* out_gyro_ready, bool* out_accel_ready, float* out_gyro_x, float* out_gyro_y,
                              float* out_gyro_z, float* out_accel_x, float* out_accel_y, float* out_accel_z);
