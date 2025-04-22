// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/display.h"
#include "esp_err.h"
#include "esp_lcd_types.h"
#include "freertos/FreeRTOS.h"

esp_err_t __attribute__((weak)) bsp_display_initialize(void) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_display_get_parameters(size_t* h_res, size_t* v_res,
                                                           lcd_color_rgb_pixel_format_t* color_fmt,
                                                           lcd_rgb_data_endian_t*        data_endian) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_display_get_panel(esp_lcd_panel_handle_t* panel) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_display_get_panel_io(esp_lcd_panel_io_handle_t* panel) {
    return ESP_ERR_NOT_SUPPORTED;
}

bsp_display_rotation_t __attribute__((weak)) bsp_display_get_default_rotation() {
    return BSP_DISPLAY_ROTATION_0;
}

esp_err_t __attribute__((weak)) bsp_display_get_backlight_brightness(uint8_t* out_percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_display_set_backlight_brightness(uint8_t percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_display_set_tearing_effect_mode(bsp_display_te_mode_t mode) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_display_get_tearing_effect_mode(bsp_display_te_mode_t* mode) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_display_get_tearing_effect_semaphore(SemaphoreHandle_t* semaphore) {
    return ESP_ERR_NOT_SUPPORTED;
}
