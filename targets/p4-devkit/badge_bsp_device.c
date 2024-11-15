#include "esp_log.h"
#include "hardware.h"
#include "dsi_panel_espressif_ek79007.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_types.h"
#include "esp_ldo_regulator.h"

#include <stdint.h>
#include <stddef.h>

static char const TAG[] = "p4-devkit-bsp";

static void enable_dsi_phy_power(void) {
    // Turn on the power for MIPI DSI PHY, so it can go from "No Power" state to "Shutdown" state
    esp_ldo_channel_handle_t ldo_mipi_phy        = NULL;
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id    = BSP_DSI_LDO_CHAN,
        .voltage_mv = BSP_DSI_LDO_VOLTAGE_MV,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy));
    ESP_LOGI(TAG, "MIPI DSI PHY Powered on");
}

esp_err_t bsp_init() {
    enable_dsi_phy_power();

    ek79007_initialize(BSP_LCD_RESET_PIN);
}

void bsp_disp_update(void const *framebuffer) {
    esp_lcd_panel_handle_t mipi_dpi_panel = ek79007_get_panel();

    if (mipi_dpi_panel) {
        size_t                       h_res     = 0;
        size_t                       v_res     = 0;
        lcd_color_rgb_pixel_format_t color_fmt = LCD_COLOR_PIXEL_FORMAT_RGB565;
        ek79007_get_parameters(&h_res, &v_res, &color_fmt);
        esp_lcd_panel_draw_bitmap(mipi_dpi_panel, 0, 0, h_res, v_res, framebuffer);
    }
}


void bsp_disp_update_part(void const *framebuffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    esp_lcd_panel_handle_t mipi_dpi_panel = ek79007_get_panel();

    if (mipi_dpi_panel) {
        esp_lcd_panel_draw_bitmap(mipi_dpi_panel, x, y, w, h, framebuffer);
    }
}
