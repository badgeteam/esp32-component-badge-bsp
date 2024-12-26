// Board support package API: MCH2022 implementation
// SPDX-FileCopyrightText: 2024 Orange-Murker
// SPDX-License-Identifier: MIT

#include "bsp/display.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_lcd_ili9341.h"
#include "esp_lcd_io_spi.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_types.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "hal/lcd_types.h"
#include "hal/spi_types.h"
#include "mch2022_hardware.h"

#include <stdint.h>

static char const *TAG = "BSP display";

#define H_RES      320
#define V_RES      240
#define COLOUR_FMT LCD_COLOR_PIXEL_FORMAT_RGB565

static esp_lcd_panel_handle_t    panel_handle    = NULL;
static esp_lcd_panel_io_handle_t panel_io_handle = NULL;


static ili9341_lcd_init_cmd_t const lcd_init_cmds[] = {
    //  {cmd, { data }, data_size, delay_ms}
    /* Power contorl B */
    {0xCF, (uint8_t[]){0x00, 0xC1, 0x30}, 3, 0},
    /* Power on sequence control */
    {0xED, (uint8_t[]){0x64, 0x03, 0X12, 0X81}, 4, 0},
    /* Driver timing control A */
    {0xE8, (uint8_t[]){0x85, 0x00, 0x78}, 3, 0},
    /* Power control A */
    {0xCB, (uint8_t[]){0x39, 0x2C, 0x00, 0x34, 0x02}, 5, 0},
    /* Pump ratio control */
    {0xF7, (uint8_t[]){0x20}, 1, 0},
    /* Driver timing control B */
    {0xEA, (uint8_t[]){0x00, 0x00}, 2, 0},
    /* Power control 1 */
    {0xC0, (uint8_t[]){0x23}, 1, 0},
    /* Power control 2 */
    {0xC1, (uint8_t[]){0x10}, 1, 0},
    /* VCOM control 1 */
    {0xC5, (uint8_t[]){0x3E, 0x28}, 2, 0},
    /* VCOM control 2 */
    {0xC7, (uint8_t[]){0x86}, 1, 0},
    /* Frame rate control */
    {0xB1, (uint8_t[]){0x00, 0x18}, 2, 0},
    /* Display function control */
    {0xB6, (uint8_t[]){0x08, 0x82, 0x27}, 3, 0},
    /* Enable 3G, disabled */
    {0xF2, (uint8_t[]){0x00}, 1, 0},
    /* Gamma set */
    {0x26, (uint8_t[]){0x01}, 1, 0},
    /* Positive gamma correction */
    {0xE0, (uint8_t[]){0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00}, 15, 0},
    /* Negative gamma correction */
    {0xE1, (uint8_t[]){0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F}, 15, 0},
};


esp_err_t bsp_display_initialize(void) {
    ESP_RETURN_ON_ERROR(gpio_set_direction(BSP_LCD_MODE_PIN, GPIO_MODE_OUTPUT), TAG, "Could not set the LCD mode pin direction");

    ESP_RETURN_ON_ERROR(gpio_set_level(BSP_LCD_MODE_PIN, false), TAG, "Failed to set the LCD mode");

    spi_bus_config_t spi_bus_config = {
        .mosi_io_num     = BSP_SPI_MOSI,
        .miso_io_num     = BSP_SPI_MISO,
        .sclk_io_num     = BSP_SPI_SCLK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = H_RES * 80 * sizeof(uint16_t),
    };

    ESP_RETURN_ON_ERROR(spi_bus_initialize(SPI3_HOST, &spi_bus_config, SPI_DMA_CH_AUTO), TAG, "Failed to initialise the SPI bus");

    esp_lcd_panel_io_spi_config_t lcd_spi_config = {
        .cs_gpio_num       = BSP_LCD_CS_PIN,
        .dc_gpio_num       = BSP_LCD_DC_PIN,
        .spi_mode          = 0,
        .pclk_hz           = 40 * 1000 * 1000,
        .trans_queue_depth = 10,
        .lcd_cmd_bits      = 8,
        .lcd_param_bits    = 8,
    };

    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi(SPI3_HOST, &lcd_spi_config, &panel_io_handle), TAG, "Failed to create LCD panel io handle");

    ili9341_vendor_config_t vendor_config = {
        .init_cmds      = lcd_init_cmds,
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(ili9341_lcd_init_cmd_t),
    };

    esp_lcd_panel_dev_config_t lcd_panel_dev_config = {
        .reset_gpio_num = BSP_LCD_RESET_PIN,
        .data_endian    = LCD_RGB_ENDIAN_RGB,
        // RBG
        .bits_per_pixel = 16,
        .vendor_config  = &vendor_config,
    };

    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_ili9341(panel_io_handle, &lcd_panel_dev_config, &panel_handle), TAG, "Failed to create a new LCD panel");

    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(panel_handle), TAG, "Failed to reset the LCD panel");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(panel_handle), TAG, "Failed to init the LCD panel");
    // ESP_RETURN_ON_ERROR(esp_lcd_panel_mirror(panel_handle, true, true), TAG, "Failed to mirror the LCD");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_swap_xy(panel_handle, true), TAG, "Failed to swap x and y on the LCD");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(panel_handle, true), TAG, "Failed to turn on the LCD panel");


    return ESP_OK;
}

esp_err_t bsp_display_get_parameters(size_t *h_res, size_t *v_res, lcd_color_rgb_pixel_format_t *color_fmt) {
    *h_res     = H_RES;
    *v_res     = V_RES;
    *color_fmt = COLOUR_FMT;

    return ESP_OK;
}

esp_err_t bsp_display_get_panel(esp_lcd_panel_handle_t *panel) {
    if (!panel_handle) {
        ESP_LOGE(TAG, "Display not initialised");
        return ESP_FAIL;
    }

    *panel = panel_handle;
    return ESP_OK;
}

esp_err_t bsp_display_get_panel_io(esp_lcd_panel_io_handle_t *panel_io) {
    if (!panel_handle) {
        ESP_LOGE(TAG, "Display IO not initialised");
        return ESP_FAIL;
    }

    *panel_io = panel_io_handle;
    return ESP_OK;
}

bsp_display_rotation_t bsp_display_get_default_rotation() {
    return BSP_DISPLAY_ROTATION_0;
}
