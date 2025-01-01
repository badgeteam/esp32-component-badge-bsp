// Board support package API: Tanmatsu implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-FileCopyrightText: 2024 Orange-Murker
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "bsp/device.h"
#include "bsp/display.h"
#include "bsp/tanmatsu.h"
#include "driver/gpio.h"
#include "dsi_panel_nicolaielectronics_st7701.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_ops.h"
#include "esp_ldo_regulator.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "tanmatsu_coprocessor.h"
#include "tanmatsu_hardware.h"

static char const* TAG = "BSP display";

static esp_ldo_channel_handle_t ldo_mipi_phy = NULL;

static bool bsp_display_initialized = false;

static esp_err_t bsp_display_enable_dsi_phy_power(void) {
    if (ldo_mipi_phy != NULL) {
        return ESP_OK;
    }
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id = BSP_DSI_LDO_CHAN,
        .voltage_mv = BSP_DSI_LDO_VOLTAGE_MV,
    };
    return esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy);
}

static esp_err_t bsp_display_reset(void) {
    gpio_config_t lcd_reset_conf = {
        .pin_bit_mask = BIT64(BSP_LCD_RESET_PIN),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&lcd_reset_conf);

    gpio_set_level(BSP_LCD_RESET_PIN, false);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(BSP_LCD_RESET_PIN, true);
    vTaskDelay(pdMS_TO_TICKS(100));

    return ESP_OK;
}

static esp_err_t bsp_display_initialize_panel(void) {
    // TODO: extend ST7701 driver in MIPI DSI abstraction component to support error handling and fix broken display
    // reset
    st7701_initialize(-1);
    return ESP_OK;
}

// Public functions

esp_err_t bsp_display_initialize(void) {
    if (bsp_display_initialized) {
        return ESP_OK;
    }
    ESP_RETURN_ON_ERROR(bsp_display_enable_dsi_phy_power(), TAG, "Failed to enable DSI PHY power");
    ESP_RETURN_ON_ERROR(bsp_display_reset(), TAG, "Failed to reset display");
    ESP_RETURN_ON_ERROR(bsp_display_initialize_panel(), TAG, "Failed to initialize panel");
    bsp_display_initialized = true;
    return ESP_OK;
}

esp_err_t bsp_display_get_parameters(size_t* h_res, size_t* v_res, lcd_color_rgb_pixel_format_t* color_fmt) {
    if (!bsp_display_initialized) {
        ESP_LOGE(TAG, "Display not initialized");
        return ESP_FAIL;
    }
    st7701_get_parameters(h_res, v_res, color_fmt);
    return ESP_OK;
}

esp_err_t bsp_display_get_panel(esp_lcd_panel_handle_t* panel) {
    if (!bsp_display_initialized) {
        ESP_LOGE(TAG, "Display not initialized");
        return ESP_FAIL;
    }
    *panel = st7701_get_panel();
    return ESP_OK;
}

bsp_display_rotation_t bsp_display_get_default_rotation() {
    return BSP_DISPLAY_ROTATION_270;
}

esp_err_t bsp_display_get_backlight_brightness(uint8_t* out_percentage) {
    ESP_RETURN_ON_FALSE(out_percentage, ESP_ERR_INVALID_ARG, TAG, "Percentage output argument is NULL");
    uint8_t raw_value;
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_display_backlight(handle, &raw_value), TAG,
                        "Failed to get display backlight brightness");
    *out_percentage = (raw_value * 100) / 255;
    return ESP_OK;
}

esp_err_t bsp_display_set_backlight_brightness(uint8_t percentage) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_set_display_backlight(handle, (percentage * 255) / 100), TAG,
                        "Failed to configure display backlight brightness");
    return ESP_OK;
}
