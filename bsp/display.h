#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_lcd_types.h"
#include "freertos/FreeRTOS.h"

/// @brief Display color format
/// @details The color format the display driver for the active target expects the pixel buffer to be in
typedef enum {
    BSP_DISPLAY_COLOR_FORMAT_1_PAL = 0,
    BSP_DISPLAY_COLOR_FORMAT_2_PAL,
    BSP_DISPLAY_COLOR_FORMAT_4_PAL,
    BSP_DISPLAY_COLOR_FORMAT_8_PAL,
    BSP_DISPLAY_COLOR_FORMAT_16_PAL,
    BSP_DISPLAY_COLOR_FORMAT_1_GREY,
    BSP_DISPLAY_COLOR_FORMAT_2_GREY,
    BSP_DISPLAY_COLOR_FORMAT_4_GREY,
    BSP_DISPLAY_COLOR_FORMAT_8_GREY,
    BSP_DISPLAY_COLOR_FORMAT_8_332RGB,
    BSP_DISPLAY_COLOR_FORMAT_16_565RGB,
    BSP_DISPLAY_COLOR_FORMAT_4_1111ARGB,
    BSP_DISPLAY_COLOR_FORMAT_8_2222ARGB,
    BSP_DISPLAY_COLOR_FORMAT_16_4444ARGB,
    BSP_DISPLAY_COLOR_FORMAT_24_888RGB,
    BSP_DISPLAY_COLOR_FORMAT_32_8888ARGB,
    BSP_DISPLAY_COLOR_FORMAT_18_666RGB,
} bsp_display_color_format_t;

/// @brief Display data endianness
/// @details Order of the bytes in each pixel
typedef enum {
    BSP_DISPLAY_ENDIAN_BIG = 0,
    BSP_DISPLAY_ENDIAN_LITTLE,
} bsp_display_endianness_t;

/// @brief Display rotation
/// @details Relative to how you would rotate the display clockwise
typedef enum {
    BSP_DISPLAY_ROTATION_0,
    BSP_DISPLAY_ROTATION_90,
    BSP_DISPLAY_ROTATION_180,
    BSP_DISPLAY_ROTATION_270,
} bsp_display_rotation_t;

/// @brief Tearing effect pin modi
/// @details Mode of tearing effect pin of display
typedef enum {
    BSP_DISPLAY_TE_DISABLED,
    BSP_DISPLAY_TE_V_BLANKING,
    BSP_DISPLAY_TE_V_AND_H_BLANKING,
} bsp_display_te_mode_t;

typedef struct {
    bsp_display_color_format_t requested_color_format;  // Requested color format, applied if supported by the display
    size_t                     num_fbs;                 // Number of framebuffers to allocate
} bsp_display_configuration_t;

/// @brief Get display parameters
/// @details Get display parameters
/// @return ESP-IDF error code
///          - ESP_OK if succesful
///          - ESP_FAIL if not initialized
esp_err_t bsp_display_get_parameters(size_t* h_res, size_t* v_res, bsp_display_color_format_t* color_fmt,
                                     bsp_display_endianness_t* data_endian);

/// @brief Get display panel
/// @details Get display panel
/// @return ESP-IDF error code
///          - ESP_OK if succesful
///          - ESP_FAIL if not initialized
esp_err_t bsp_display_get_panel(esp_lcd_panel_handle_t* panel);

/// @brief Get display panel IO
/// @details Get display panel IO
/// @return ESP-IDF error code
///          - ESP_OK if succesful
///          - ESP_FAIL if not initialized
esp_err_t bsp_display_get_panel_io(esp_lcd_panel_io_handle_t* io);

/// @brief Get the default display rotation
/// @return The default display rotation
bsp_display_rotation_t bsp_display_get_default_rotation();

/// @brief Get display brightness
/// @return ESP-IDF error code
esp_err_t bsp_display_get_backlight_brightness(uint8_t* out_percentage);

/// @brief Set display brightness
/// @return ESP-IDF error code
esp_err_t bsp_display_set_backlight_brightness(uint8_t percentage);

/// @brief Set tearing effect (TE) output mode
/// @return ESP-IDF error code
esp_err_t bsp_display_set_tearing_effect_mode(bsp_display_te_mode_t mode);

/// @brief Read current tearing effect (TE) output mode
/// @return ESP-IDF error code
esp_err_t bsp_display_get_tearing_effect_mode(bsp_display_te_mode_t* mode);

/// @brief Get handle for the tearing effect (TE) semaphore
/// @return ESP-IDF error code
esp_err_t bsp_display_get_tearing_effect_semaphore(SemaphoreHandle_t* semaphore);

/// @brief Send pixel data to the display
/// @return ESP-IDF error code
esp_err_t bsp_display_blit(size_t x, size_t y, size_t width, size_t height, const void* buffer);
