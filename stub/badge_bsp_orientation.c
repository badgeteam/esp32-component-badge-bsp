// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/orientation.h"
#include "esp_err.h"

esp_err_t __attribute__((weak)) bsp_orientation_initialize(void) {
    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_orientation_enable_gyroscope(void) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_orientation_disable_gyroscope(void) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_orientation_enable_accelerometer(void) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_orientation_disable_accelerometer(void) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_orientation_get(bool* out_gyro_ready, bool* out_accel_ready, float* out_gyro_x,
                                                    float* out_gyro_y, float* out_gyro_z, float* out_accel_x,
                                                    float* out_accel_y, float* out_accel_z) {
    (void)out_gyro_ready;
    (void)out_accel_ready;
    (void)out_gyro_x;
    (void)out_gyro_y;
    (void)out_gyro_z;
    (void)out_accel_x;
    (void)out_accel_y;
    (void)out_accel_z;
    return ESP_ERR_NOT_SUPPORTED;
}
