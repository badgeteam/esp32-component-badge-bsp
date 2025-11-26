#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

/// @brief Write data to LEDs (deprecated)
/// @return ESP-IDF error code
esp_err_t bsp_led_write(const uint8_t* data, uint32_t length);

/// @brief Set LED brightness (0-100%)
/// @return ESP-IDF error code
esp_err_t bsp_led_set_brightness(uint8_t percentage);

/// @brief Set LED brightness (0-100%)
/// @return ESP-IDF error code
esp_err_t bsp_led_get_brightness(uint8_t* out_percentage);

/// @brief Set LED mode (automatic or manual control)
/// @return ESP-IDF error code
esp_err_t bsp_led_set_mode(bool automatic);

/// @brief Get the LED mode (automatic or manual control)
/// @return true if automatic mode is enabled, false otherwise
esp_err_t bsp_led_get_mode(bool* out_automatic);

/// @brief Write data to LEDs
/// @return ESP-IDF error code
esp_err_t bsp_led_send(void);

/// @brief Clear LED data
/// @return ESP-IDF error code
esp_err_t bsp_led_clear(void);

/// @brief Set LED pixel color (using 0xRRGGBB format)
/// @return ESP-IDF error code
esp_err_t bsp_led_set_pixel(uint32_t index, uint32_t color);

/// @brief Set LED pixel color (RGB)
/// @return ESP-IDF error code
esp_err_t bsp_led_set_pixel_rgb(uint32_t index, uint8_t red, uint8_t green, uint8_t blue);

/// @brief Set LED pixel color (RGBW)
/// @return ESP-IDF error code
esp_err_t bsp_led_set_pixel_rgbw(uint32_t index, uint8_t red, uint8_t green, uint8_t blue, uint8_t white);

/// @brief Set LED pixel color (HSV)
/// @return ESP-IDF error code
esp_err_t bsp_led_set_pixel_hsv(uint32_t index, uint16_t hue, uint8_t saturation, uint8_t value);
