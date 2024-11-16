#pragma once

#include "esp_err.h"

#include <stdbool.h>
#include <stdint.h>

// Badge BSP
// Device related APIs

/// @brief Initialize the hardware managed by the BSP
/// @details Initialize the hardware and related drivers managed by the BSP
/// @return ESP-IDF error code
///          - ESP_OK if BSP initialized correctly
///          - ESP_FAIL if the BSP could not initialize
esp_err_t bsp_device_initialize(void);

/// @brief Get the name of the device as a string
/// @details Returns the name as a null terminated string
/// @param[out] output Pointer to buffer to which the name string gets copied
/// @param[in] buffer_length Length of the output buffer
/// @return ESP-IDF error code
///          - ESP_OK if the output string fits in the buffer
///          - ESP_ERR_INVALID_ARG if the output pointer is NULL
esp_err_t bsp_device_get_name(char *output, uint8_t buffer_length);

/// @brief Get the name of the manufacturer as a string
/// @details Returns the name as a null terminated string
/// @param[out] output Pointer to buffer to which the name string gets copied
/// @param[in] buffer_length Length of the output buffer
/// @return ESP-IDF error code
///          - ESP_OK if the output string fits in the buffer
///          - ESP_ERR_INVALID_ARG if the output pointer is NULL
esp_err_t bsp_device_get_manufacturer(char *output, uint8_t buffer_length);
