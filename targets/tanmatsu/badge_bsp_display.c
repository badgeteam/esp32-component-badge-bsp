// Board support package API: Tanmatsu implementation
// SPDX-FileCopyrightText: 2024-2025 Nicolai Electronics
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
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_ldo_regulator.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "hal/lcd_types.h"
#include "tanmatsu_coprocessor.h"
#include "tanmatsu_hardware.h"

static char const* TAG = "BSP display";

static esp_ldo_channel_handle_t ldo_mipi_phy            = NULL;
static bool                     bsp_display_initialized = false;
static bsp_display_te_mode_t    display_te_mode         = BSP_DISPLAY_TE_DISABLED;
static SemaphoreHandle_t        te_semaphore            = NULL;
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

static esp_err_t bsp_display_reset(void) {
    gpio_config_t lcd_reset_conf = {
        .pin_bit_mask = BIT64(BSP_LCD_RESET_PIN),
        .mode         = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en   = 0,
        .pull_down_en = 0,
        .intr_type    = GPIO_INTR_DISABLE,
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

IRAM_ATTR static void te_gpio_interrupt_handler(void* pvParameters) {
    xSemaphoreGiveFromISR(te_semaphore, NULL);
}

static esp_err_t bsp_display_initialize_flush(void) {
    flush_semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(flush_semaphore);
    esp_lcd_dpi_panel_event_callbacks_t callbacks = {
        .on_color_trans_done = bsp_display_flush_ready,
    };
    return esp_lcd_dpi_panel_register_event_callbacks(st7701_get_panel(), &callbacks, NULL);
}

static esp_err_t bsp_display_initialize_te(void) {
    te_semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(te_semaphore);

    gpio_config_t te_pin_cfg = {
        .pin_bit_mask = BIT64(BSP_LCD_TE_PIN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = false,
        .pull_down_en = false,
        .intr_type    = GPIO_INTR_POSEDGE,
    };
    esp_err_t res = gpio_config(&te_pin_cfg);
    if (res != ESP_OK) {
        return res;
    }
    res = gpio_isr_handler_add(BSP_LCD_TE_PIN, te_gpio_interrupt_handler, NULL);
    return res;
}

// Public functions

esp_err_t bsp_display_initialize(void) {
    if (bsp_display_initialized) {
        return ESP_OK;
    }
    ESP_RETURN_ON_ERROR(bsp_display_enable_dsi_phy_power(), TAG, "Failed to enable DSI PHY power");
    ESP_RETURN_ON_ERROR(bsp_display_reset(), TAG, "Failed to reset display");
    ESP_RETURN_ON_ERROR(bsp_display_initialize_panel(), TAG, "Failed to initialize panel");
    ESP_RETURN_ON_ERROR(bsp_display_initialize_flush(), TAG, "Failed to initialize flush callback");
    ESP_RETURN_ON_ERROR(bsp_display_initialize_te(), TAG, "Failed to tearing effect callback");
    bsp_display_initialized = true;
    return ESP_OK;
}

esp_err_t bsp_display_get_parameters(size_t* h_res, size_t* v_res, lcd_color_rgb_pixel_format_t* color_fmt,
                                     lcd_rgb_data_endian_t* data_endian) {
    if (!bsp_display_initialized) {
        return ESP_FAIL;
    }
    st7701_get_parameters(h_res, v_res, color_fmt);
    if (data_endian) {
        *data_endian = LCD_RGB_DATA_ENDIAN_LITTLE;
    }
    return ESP_OK;
}

esp_err_t bsp_display_get_panel(esp_lcd_panel_handle_t* panel) {
    if (!bsp_display_initialized) {
        return ESP_FAIL;
    }
    *panel = st7701_get_panel();
    return ESP_OK;
}

esp_err_t bsp_display_get_panel_io(esp_lcd_panel_io_handle_t* panel_io) {
    if (!bsp_display_initialized) {
        return ESP_FAIL;
    }

    *panel_io = st7701_get_panel_io();
    return ESP_OK;
}

bsp_display_rotation_t bsp_display_get_default_rotation() {
    return BSP_DISPLAY_ROTATION_270;
}

esp_err_t bsp_display_get_backlight_brightness(uint8_t* out_percentage) {
    ESP_RETURN_ON_FALSE(out_percentage, ESP_ERR_INVALID_ARG, TAG, "Percentage output argument is NULL");
    uint8_t                       raw_value;
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

esp_err_t bsp_display_set_tearing_effect_mode(bsp_display_te_mode_t mode) {
    if (!bsp_display_initialized) {
        return ESP_FAIL;
    }

    esp_lcd_panel_io_handle_t panel_io = st7701_get_panel_io();

    esp_err_t res = ESP_OK;

    if (mode == BSP_DISPLAY_TE_DISABLED) {
        res = esp_lcd_panel_io_tx_param(panel_io, LCD_CMD_TEOFF, (uint8_t[]){0}, 0);
    } else {
        res = esp_lcd_panel_io_tx_param(panel_io, LCD_CMD_TEON,
                                        (uint8_t[]){(mode == BSP_DISPLAY_TE_V_AND_H_BLANKING) ? 1 : 0}, 1);
    }

    if (res != ESP_OK) {
        return res;
    }

    display_te_mode = mode;
    return ESP_OK;
}

esp_err_t bsp_display_get_tearing_effect_mode(bsp_display_te_mode_t* mode) {
    if (mode == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *mode = display_te_mode;
    return ESP_OK;
}

esp_err_t bsp_display_get_tearing_effect_semaphore(SemaphoreHandle_t* semaphore) {
    if (semaphore == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *semaphore = te_semaphore;
    return ESP_OK;
}

esp_err_t bsp_display_blit(size_t x_start, size_t y_start, size_t x_end, size_t y_end, const void* buffer) {
    xSemaphoreTake(flush_semaphore, pdMS_TO_TICKS(1000));
    return esp_lcd_panel_draw_bitmap(st7701_get_panel(), x_start, y_start, x_end, y_end, buffer);
}
