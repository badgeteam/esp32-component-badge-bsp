
// SPDX-License-Identifier: MIT

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

// Badge BSP
// Device information APIs

/// @brief Get the name of the device as a string
/// @details Returns the name as a null terminated string
/// @param[out] output Pointer to buffer to which the name string gets copied
/// @param[in] buffer_length Length of the output buffer
/// @return ESP-IDF error code
///          - ESP_OK if the output string fits in the buffer
///          - ESP_ERR_INVALID_ARG if the output pointer is NULL
esp_err_t bsp_device_get_name(char* output, uint8_t buffer_length);

/// @brief Get the name of the manufacturer as a string
/// @details Returns the name as a null terminated string
/// @param[out] output Pointer to buffer to which the name string gets copied
/// @param[in] buffer_length Length of the output buffer
/// @return ESP-IDF error code
///          - ESP_OK if the output string fits in the buffer
///          - ESP_ERR_INVALID_ARG if the output pointer is NULL
esp_err_t bsp_device_get_manufacturer(char* output, uint8_t buffer_length);


esp_err_t bsp_init();

esp_err_t bsp_set_display_backlight(uint8_t level);
