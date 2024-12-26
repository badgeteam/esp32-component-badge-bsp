#pragma once

#include "esp_err.h"
#include "esp_lcd_types.h"

#include <stdbool.h>
#include <stdint.h>

// Badge BSP
// Display related APIs

/// @brief Display rotation
/// @details Relative to how you would rotate the display clockwise
typedef enum {
    BSP_DISPLAY_ROTATION_0,
    BSP_DISPLAY_ROTATION_90,
    BSP_DISPLAY_ROTATION_180,
    BSP_DISPLAY_ROTATION_270,
} bsp_display_rotation_t;


/// @brief Initialize the display
/// @details Initialize the display
/// @return ESP-IDF error code
///          - ESP_OK if BSP initialized correctly
///          - ESP_FAIL if the BSP could not initialize
esp_err_t bsp_display_initialize(void);

/// @brief Get display parameters
/// @details Get display parameters
/// @return ESP-IDF error code
///          - ESP_OK if succesful
///          - ESP_FAIL if not initialized
esp_err_t bsp_display_get_parameters(size_t *h_res, size_t *v_res, lcd_color_rgb_pixel_format_t *color_fmt);

/// @brief Get display panel
/// @details Get display panel
/// @return ESP-IDF error code
///          - ESP_OK if succesful
///          - ESP_FAIL if not initialized
esp_err_t bsp_display_get_panel(esp_lcd_panel_handle_t *panel);

/// @brief Get display brightness
/// @return ESP-IDF error code
esp_err_t bsp_display_get_backlight_brightness(float *percentage);

/// @brief Set display brightness
/// @return ESP-IDF error code
esp_err_t bsp_display_set_backlight_brightness(float percentage);

/// @brief Get display panel IO
/// @details Get display panel IO
/// @return ESP-IDF error code
///          - ESP_OK if succesful
///          - ESP_FAIL if not initialized
esp_err_t bsp_display_get_panel_io(esp_lcd_panel_io_handle_t *io);

/// @brief Get the default display rotation
/// @return The default display rotation
bsp_display_rotation_t bsp_display_get_default_rotation();
