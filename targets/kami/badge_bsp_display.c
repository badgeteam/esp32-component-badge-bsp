// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include "bsp/display.h"
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
#include "kami_hardware.h"
#include "ssd1619.h"
#include "ssd1619_lut.h"

static char const* TAG = "BSP display";

#define H_RES      BSP_EPAPER_WIDTH
#define V_RES      BSP_EPAPER_HEIGHT
#define COLOUR_FMT 999

static ssd1619_t epaper = {
    .spi_bus               = BSP_EPAPER_SPI_BUS,
    .pin_cs                = BSP_EPAPER_CS_PIN,
    .pin_dcx               = BSP_EPAPER_DCX_PIN,
    .pin_reset             = BSP_EPAPER_RESET_PIN,
    .pin_busy              = BSP_EPAPER_BUSY_PIN,
    .spi_speed             = BSP_EPAPER_SPEED,
    .spi_max_transfer_size = SOC_SPI_MAXIMUM_BUFFER_SIZE,
    .screen_width          = BSP_EPAPER_WIDTH,
    .screen_height         = BSP_EPAPER_HEIGHT,
};
static ssd1619_lut_t cur_lut = lut_1s;

static esp_err_t bsp_display_initialize_epaper_lut() {
    esp_err_t res = ESP_OK;

    if (!ssd1619_get_lut_populated()) {
        ESP_LOGW(TAG, "Display LUT table not initialized");
        res = ssd1619_read_lut(BSP_EPAPER_DATA_PIN, BSP_EPAPER_CLK_PIN, epaper.pin_cs, epaper.pin_dcx, epaper.pin_reset,
                               epaper.pin_busy);
        if (res != ESP_OK) {
            return res;
        }
    }

    return res;
}

esp_err_t bsp_display_initialize(void) {
    ESP_RETURN_ON_ERROR(bsp_display_initialize_epaper_lut(), TAG, "Failed to initialize e-paper LUT");

    spi_bus_config_t spi_bus_config = {
        .mosi_io_num     = BSP_EPAPER_DATA_PIN,
        .miso_io_num     = -1,
        .sclk_io_num     = BSP_EPAPER_CLK_PIN,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = SOC_SPI_MAXIMUM_BUFFER_SIZE,
    };

    ESP_RETURN_ON_ERROR(spi_bus_initialize(BSP_EPAPER_SPI_BUS, &spi_bus_config, SPI_DMA_CH_AUTO), TAG,
                        "Failed to initialise the SPI bus");
    ESP_RETURN_ON_ERROR(ssd1619_init(&epaper), TAG, "Failed to initialize e-paper display");
    ESP_RETURN_ON_ERROR(ssd1619_apply_lut(&epaper, lut_1s), TAG, "Failed to apply e-paper LUT");
    return ESP_OK;
}

esp_err_t bsp_display_get_parameters(size_t* h_res, size_t* v_res, lcd_color_rgb_pixel_format_t* color_fmt,
                                     lcd_rgb_data_endian_t* data_endian) {
    if (h_res) *h_res = H_RES;
    if (v_res) *v_res = V_RES;
    if (color_fmt) *color_fmt = COLOUR_FMT;
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
    return BSP_DISPLAY_ROTATION_270;
}

esp_err_t bsp_display_blit(size_t x, size_t y, size_t width, size_t height, const void* buffer) {
    if (x != 0 || y != 0 || width != H_RES || height != V_RES) {
        ESP_LOGE(TAG, "Display does not support partial updates");
        return ESP_FAIL;
    }
    return ssd1619_write(&epaper, buffer);
}
