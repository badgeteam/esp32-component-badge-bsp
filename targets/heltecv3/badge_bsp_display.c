// SPDX-FileCopyrightText: 2026 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/display.h"
#include "bsp/i2c.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_types.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "heltecv3_hardware.h"

#define SSD1306_HEIGHT 64

static const char* TAG = "BSP DISPLAY";

static i2c_master_bus_handle_t   i2c_bus_handle = NULL;
static esp_lcd_panel_io_handle_t io_handle      = NULL;
static esp_lcd_panel_handle_t    panel_handle   = NULL;

esp_err_t bsp_display_initialize(const bsp_display_configuration_t* configuration) {
    (void)configuration;
    ESP_LOGI(TAG, "Get I2C handle");
    esp_err_t res = bsp_i2c_primary_bus_get_handle(&i2c_bus_handle);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get I2C bus handle: %d", res);
        return res;
    }

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr            = BSP_OLED_I2C_ADDRESS,
        .scl_speed_hz        = (400 * 1000),
        .control_phase_bytes = 1,
        .lcd_cmd_bits        = 8,
        .lcd_param_bits      = 8,
        .dc_bit_offset       = 6,
    };
    res = esp_lcd_new_panel_io_i2c(i2c_bus_handle, &io_config, &io_handle);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize panel IO: %d", res);
        return res;
    }

    ESP_LOGI(TAG, "Install SSD1306 driver");
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = BSP_OLED_RESET,
    };

    esp_lcd_panel_ssd1306_config_t ssd1306_config = {
        .height = SSD1306_HEIGHT,
    };
    panel_config.vendor_config = &ssd1306_config;
    res                        = esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize driver: %d", res);
        return res;
    }

    gpio_config_t int_pin_cfg = {
        .pin_bit_mask = BIT64(36),
        .mode         = GPIO_MODE_OUTPUT,
    };
    res = gpio_config(&int_pin_cfg);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure vext pin: %d", res);
        return res;
    }

    gpio_set_level(36, 0);

    ESP_LOGI(TAG, "Reset");
    res = esp_lcd_panel_reset(panel_handle);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset panel: %d", res);
        return res;
    }

    ESP_LOGI(TAG, "Init");
    res = esp_lcd_panel_init(panel_handle);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize panel: %d", res);
        return res;
    }

    ESP_LOGI(TAG, "On");
    res = esp_lcd_panel_disp_on_off(panel_handle, true);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to turn on display: %d", res);
        return res;
    }

    /*
        Normally SSD1306 displays use one byte per 8 vertical pixels arranged horizontally.
        That doesn't work well with graphics stacks that expect the bytes to be 8 horizontal pixels.
        To work around this problem the commands below activate vertical addressing and reverse COM
        output direction. The result is a native portrait screen that has the pixels aligned in a usable
        manner. The graphics stack is responsible for rotating for landscape use.
    */

    // Reconfigure the SSD1306 to use vertical addressing
    res = esp_lcd_panel_io_tx_param(io_handle, 0x20, (uint8_t[]){0x01}, 1);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set addressing mode: %d", res);
        return res;
    }

    // Reconfigure the SSD1306 to use reverse COM output direction
    res = esp_lcd_panel_io_tx_param(io_handle, 0xC8, NULL, 0);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set addressing mode: %d", res);
        return res;
    }

    ESP_LOGI(TAG, "Initialized the display");

    return ESP_OK;
}

esp_err_t bsp_display_get_parameters(size_t* h_res, size_t* v_res, bsp_display_color_format_t* color_fmt,
                                     bsp_display_endianness_t* data_endian) {
    if (h_res != NULL) {
        *h_res = 64;
    }
    if (v_res != NULL) {
        *v_res = 128;
    }
    if (color_fmt != NULL) {
        *color_fmt = BSP_DISPLAY_COLOR_FORMAT_1_GREY;
    }
    if (data_endian != NULL) {
        *data_endian = BSP_DISPLAY_ENDIAN_BIG;
    }
    return ESP_OK;
}

esp_err_t bsp_display_get_panel(esp_lcd_panel_handle_t* panel) {
    if (panel == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (panel_handle == NULL) {
        return ESP_FAIL;
    }
    *panel = panel_handle;
    return ESP_OK;
}

esp_err_t bsp_display_get_panel_io(esp_lcd_panel_io_handle_t* panel) {
    if (panel == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (io_handle == NULL) {
        return ESP_FAIL;
    }
    *panel = io_handle;
    return ESP_OK;
}

bsp_display_rotation_t bsp_display_get_default_rotation() {
    return BSP_DISPLAY_ROTATION_90;
}

esp_err_t bsp_display_get_backlight_brightness(uint8_t* out_percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_display_set_backlight_brightness(uint8_t percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_display_set_tearing_effect_mode(bsp_display_te_mode_t mode) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_display_get_tearing_effect_mode(bsp_display_te_mode_t* mode) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_display_get_tearing_effect_semaphore(SemaphoreHandle_t* semaphore) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_display_blit(size_t x_start, size_t y_start, size_t x_end, size_t y_end, const void* buffer) {
    return esp_lcd_panel_draw_bitmap(panel_handle, y_start, x_start, y_end, x_end, buffer);
}
