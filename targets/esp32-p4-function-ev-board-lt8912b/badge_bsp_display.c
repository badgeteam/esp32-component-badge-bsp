// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "bsp/device.h"
#include "bsp/display.h"
#include "bsp/i2c.h"
#include "devkit_hardware.h"
#include "driver/gpio.h"
#include "dsi_panel_espressif_lt8912b.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_ldo_regulator.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "hal/gpio_types.h"
#include "hal/lcd_types.h"

static char const* TAG = "BSP display";

static esp_ldo_channel_handle_t ldo_mipi_phy            = NULL;
static bool                     bsp_display_initialized = false;
static SemaphoreHandle_t        flush_semaphore         = NULL;

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

static esp_err_t bsp_display_initialize_flush(void) {
    flush_semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(flush_semaphore);
    esp_lcd_dpi_panel_event_callbacks_t callbacks = {
        .on_color_trans_done = bsp_display_flush_ready,
    };
    return esp_lcd_dpi_panel_register_event_callbacks(lt8912b_get_panel(), &callbacks, NULL);
}

static esp_err_t bsp_display_initialize_panel(void) {
    i2c_master_bus_handle_t i2c_handle = NULL;
    esp_err_t               res        = bsp_i2c_primary_bus_get_handle(&i2c_handle);
    if (res != ESP_OK) {
        return res;
    }
    lt8912b_initialize(BSP_LCD_RESET_PIN, i2c_handle, LT8912B_RESOLUTION_800X600);
    return ESP_OK;
}

// Public functions

esp_err_t bsp_display_initialize(void) {
    if (bsp_display_initialized) {
        return ESP_OK;
    }
    ESP_RETURN_ON_ERROR(bsp_display_enable_dsi_phy_power(), TAG, "Failed to enable DSI PHY power");
    ESP_RETURN_ON_ERROR(bsp_display_initialize_panel(), TAG, "Failed to initialize panel");
    ESP_RETURN_ON_ERROR(bsp_display_initialize_flush(), TAG, "Failed to initialize flush callback");
    bsp_display_initialized = true;
    return ESP_OK;
}

esp_err_t bsp_display_get_parameters(size_t* h_res, size_t* v_res, lcd_color_rgb_pixel_format_t* color_fmt,
                                     lcd_rgb_data_endian_t* data_endian) {
    if (!bsp_display_initialized) {
        ESP_LOGE(TAG, "Display not initialized");
        return ESP_FAIL;
    }
    lt8912b_get_parameters(h_res, v_res, color_fmt);
    if (data_endian) {
        *data_endian = LCD_RGB_DATA_ENDIAN_LITTLE;
    }
    return ESP_OK;
}

esp_err_t bsp_display_get_panel(esp_lcd_panel_handle_t* panel) {
    if (!bsp_display_initialized) {
        ESP_LOGE(TAG, "Display not initialized");
        return ESP_FAIL;
    }
    *panel = lt8912b_get_panel();
    return ESP_OK;
}

esp_err_t bsp_display_get_panel_io(esp_lcd_panel_io_handle_t* panel_io) {
    if (!bsp_display_initialized) {
        ESP_LOGE(TAG, "Display not initialised");
        return ESP_FAIL;
    }

    *panel_io = NULL;
    return ESP_OK;
}

bsp_display_rotation_t bsp_display_get_default_rotation() {
    return BSP_DISPLAY_ROTATION_0;
}

esp_err_t bsp_display_blit(size_t x_start, size_t y_start, size_t x_end, size_t y_end, const void* buffer) {
    xSemaphoreTake(flush_semaphore, pdMS_TO_TICKS(1000));
    return esp_lcd_panel_draw_bitmap(lt8912b_get_panel(), x_start, y_start, x_end, y_end, buffer);
}
