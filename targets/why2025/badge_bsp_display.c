// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "bsp/display.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_st7703.h"
#include "esp_ldo_regulator.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "hal/lcd_types.h"
#include "why2025_hardware.h"
#include "why2025_lcd_init_cmds.h"

static char const* TAG = "BSP display";

static esp_ldo_channel_handle_t  ldo_mipi_phy    = NULL;
static SemaphoreHandle_t         flush_semaphore = NULL;
static esp_lcd_dsi_bus_handle_t  mipi_dsi_bus    = NULL;
static esp_lcd_panel_io_handle_t mipi_dbi_io     = NULL;
static esp_lcd_panel_handle_t    panel_handle    = NULL;

IRAM_ATTR static bool bsp_display_flush_ready(esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t* edata,
                                              void* user_ctx) {
    xSemaphoreGiveFromISR(flush_semaphore, NULL);
    return false;
}

static esp_err_t bsp_display_enable_dsi_phy_power(void) {
    if (ldo_mipi_phy != NULL) {
        return ESP_OK;
    }
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id    = BSP_DSI_LDO_CHAN,
        .voltage_mv = BSP_DSI_LDO_VOLTAGE_MV,
    };
    return esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy);
}

static esp_err_t bsp_display_reset(void) {
    return ESP_OK;
}

static esp_err_t bsp_display_initialize_panel(void) {
    ESP_LOGI(TAG, "Initialize MIPI DSI bus");
    esp_lcd_dsi_bus_config_t bus_config = {
        .bus_id             = 0,
        .num_data_lanes     = 2,
        .phy_clk_src        = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
        .lane_bit_rate_mbps = 500,
    };
    ESP_ERROR_CHECK(esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_dbi_io_config_t dbi_config = {
        .virtual_channel = 0,
        .lcd_cmd_bits    = 8,
        .lcd_param_bits  = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &mipi_dbi_io));

    ESP_LOGI(TAG, "Install ST7703 panel driver");
    const esp_lcd_dpi_panel_config_t dpi_config = ST7703_720_720_PANEL_60HZ_DPI_CONFIG();

    st7703_vendor_config_t vendor_config = {
        .mipi_config =
            {
                .dsi_bus    = mipi_dsi_bus,
                .dpi_config = &dpi_config,
            },
        .init_cmds      = custom_init,
        .init_cmds_size = sizeof(custom_init) / sizeof(st7703_lcd_init_cmd_t),
    };
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num          = BSP_LCD_RESET_PIN,
        .rgb_ele_order           = LCD_RGB_ELEMENT_ORDER_BGR,  // Implemented by LCD command `36h`
        .bits_per_pixel          = 16,                         // Implemented by LCD command `3Ah` (16/18/24)
        .vendor_config           = &vendor_config,
        .flags.reset_active_high = 1,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7703(mipi_dbi_io, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    // ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    return ESP_OK;
}

static esp_err_t bsp_display_initialize_flush(void) {
    flush_semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(flush_semaphore);
    esp_lcd_dpi_panel_event_callbacks_t callbacks = {
        .on_color_trans_done = bsp_display_flush_ready,
    };
    return esp_lcd_dpi_panel_register_event_callbacks(panel_handle, &callbacks, NULL);
}

// Public functions

esp_err_t bsp_display_initialize(void) {
    ESP_RETURN_ON_ERROR(bsp_display_enable_dsi_phy_power(), TAG, "Failed to enable DSI PHY power");
    ESP_RETURN_ON_ERROR(bsp_display_reset(), TAG, "Failed to reset display");
    ESP_RETURN_ON_ERROR(bsp_display_initialize_panel(), TAG, "Failed to initialize panel");
    ESP_RETURN_ON_ERROR(bsp_display_initialize_flush(), TAG, "Failed to initialize flush callback");
    return ESP_OK;
}

esp_err_t bsp_display_get_parameters(size_t* h_res, size_t* v_res, lcd_color_rgb_pixel_format_t* color_fmt,
                                     lcd_rgb_data_endian_t* data_endian) {
    if (h_res) {
        *h_res = 720;
    }
    if (v_res) {
        *v_res = 720;
    }
    if (color_fmt) {
        *color_fmt = LCD_COLOR_PIXEL_FORMAT_RGB565;
    }
    if (data_endian) {
        *data_endian = LCD_RGB_DATA_ENDIAN_LITTLE;
    }
    return ESP_OK;
}

esp_err_t bsp_display_get_panel(esp_lcd_panel_handle_t* panel) {
    *panel = panel_handle;
    return ESP_OK;
}

esp_err_t bsp_display_get_panel_io(esp_lcd_panel_io_handle_t* panel_io) {
    *panel_io = mipi_dbi_io;
    return ESP_OK;
}

bsp_display_rotation_t bsp_display_get_default_rotation() {
    return BSP_DISPLAY_ROTATION_0;
}

esp_err_t bsp_display_get_backlight_brightness(uint8_t* out_percentage) {
    ESP_RETURN_ON_FALSE(out_percentage, ESP_ERR_INVALID_ARG, TAG, "Percentage output argument is NULL");
    *out_percentage = 100;
    return ESP_OK;
}

esp_err_t bsp_display_set_backlight_brightness(uint8_t percentage) {
    return ESP_OK;
}

esp_err_t bsp_display_blit(size_t x_start, size_t y_start, size_t x_end, size_t y_end, const void* buffer) {
    xSemaphoreTake(flush_semaphore, pdMS_TO_TICKS(1000));
    printf("UPDATE %zu %zu %zu %zu\n", x_start, y_start, x_end, y_end);
    return esp_lcd_panel_draw_bitmap(panel_handle, x_start, y_start, x_end, y_end, buffer);
}
