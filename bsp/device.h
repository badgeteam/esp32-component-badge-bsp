
// SPDX-License-Identifier: MIT

#pragma once

#include "esp_err.h"

#include <stdbool.h>
#include <stdint.h>

// Badge BSP
// Device information APIs

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

/// @brief Initialise the BSP, should be called early on in `app_main`
/// @return ESP-IDF error code
///          - ESP_OK if the BSP has been initialised successfully
///          - ESP_FAIL if the BSP could not be initialised
esp_err_t bsp_init();

/// Set the display backlight brightness
/// @param[in] brightness The backlight brightness
/// @return ESP-IDF error code
///          - ESP_OK if the backlight brightness has ben been set successfully
///          - ESP_FAIL if the BSP could not be set
esp_err_t bsp_set_display_backlight(uint8_t brightness);

/// Send new image data to a device's display.
/// @param[in] framebuffer The image data to send to the display
void bsp_disp_update(void const *framebuffer);

/// Send new image data to part of a device's display.
/// @param[in] framebuffer The image data to send to the display
/// @param[in] x The starting position for the partial update on the x axis
/// @param[in] y The starting position for the partial update on the y axis
/// @param[in] w The width of the drawn image
/// @param[in] h The height of the drawn image
void bsp_disp_update_part(void const *framebuffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
