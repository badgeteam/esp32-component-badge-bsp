// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include "bsp/display.h"
#include "circle_hardware.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_lcd_io_spi.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_types.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "hal/lcd_types.h"
#include "hal/spi_types.h"

static char const* TAG = "BSP display";

esp_err_t bsp_display_initialize(const bsp_display_configuration_t* configuration) {
    (void)configuration;
    return ESP_OK;
}

esp_err_t bsp_display_get_parameters(size_t* h_res, size_t* v_res, lcd_color_rgb_pixel_format_t* color_fmt,
                                     lcd_rgb_data_endian_t* data_endian) {
    if (h_res) *h_res = 0;
    if (v_res) *v_res = 0;
    if (color_fmt) *color_fmt = 0;
    if (data_endian) *data_endian = LCD_RGB_DATA_ENDIAN_LITTLE;
    return ESP_OK;
}

esp_err_t bsp_display_get_panel(esp_lcd_panel_handle_t* panel) {
    *panel = NULL;
    return ESP_OK;
}

esp_err_t bsp_display_get_panel_io(esp_lcd_panel_io_handle_t* panel_io) {
    *panel_io = NULL;
    return ESP_OK;
}

bsp_display_rotation_t bsp_display_get_default_rotation() {
    return BSP_DISPLAY_ROTATION_0;
}

esp_err_t bsp_display_blit(size_t x, size_t y, size_t width, size_t height, const void* buffer) {
    return ESP_OK;
}
