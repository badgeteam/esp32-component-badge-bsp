// Board support package API: WHY2025 implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/device.h"
#include "driver/gpio.h"
#include "dsi_panel_nicolaielectronics_st7701.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_ops.h"
#include "esp_ldo_regulator.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include <stdbool.h>
#include <stdint.h>

#include <string.h>

static char const *TAG = "BSP display";

#define PIN_DISPLAY_RESET 14

static esp_ldo_channel_handle_t ldo_mipi_phy = NULL;

static bool bsp_display_initialized = false;

static esp_err_t bsp_display_enable_dsi_phy_power(void) {
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id    = 3,
        .voltage_mv = 2500,
    };
    return esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy);
}

static esp_err_t bsp_display_reset(void) {
    gpio_config_t lcd_reset_conf = {
        .pin_bit_mask = BIT64(PIN_DISPLAY_RESET),
        .mode         = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en   = 0,
        .pull_down_en = 0,
        .intr_type    = GPIO_INTR_DISABLE,
    };

    gpio_config(&lcd_reset_conf);

    gpio_set_level(PIN_DISPLAY_RESET, false);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(PIN_DISPLAY_RESET, true);
    vTaskDelay(pdMS_TO_TICKS(100));

    return ESP_OK;
}

static esp_err_t bsp_display_initialize_panel(void) {
    st7701_initialize(-1);
    return ESP_OK;
}

// Public functions

esp_err_t bsp_display_initialize(void) {
    if (bsp_display_initialized) {
        ESP_LOGE(TAG, "Display already initialized");
        return ESP_FAIL;
    }
    ESP_RETURN_ON_ERROR(bsp_display_enable_dsi_phy_power(), TAG, "Failed to enable DSI PHY power");
    ESP_RETURN_ON_ERROR(bsp_display_reset(), TAG, "Failed to reset display");
    ESP_RETURN_ON_ERROR(bsp_display_initialize_panel(), TAG, "Failed to initialize panel");
    bsp_display_initialized = true;
    return ESP_OK;
}

esp_err_t bsp_display_get_parameters(size_t *h_res, size_t *v_res, lcd_color_rgb_pixel_format_t *color_fmt) {
    if (!bsp_display_initialized) {
        ESP_LOGE(TAG, "Display not initialized");
        return ESP_FAIL;
    }
    st7701_get_parameters(h_res, v_res, color_fmt);
    return ESP_OK;
}

esp_err_t bsp_display_get_panel(esp_lcd_panel_handle_t *panel) {
    if (!bsp_display_initialized) {
        ESP_LOGE(TAG, "Display not initialized");
        return ESP_FAIL;
    }
    *panel = st7701_get_panel();
    return ESP_OK;
}
