#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "bsp/device.h"
#include "bsp/display.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_ldo_regulator.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "hal/lcd_types.h"

#define LCD_HSYNC_GPIO      GPIO_NUM_44   // LCD_H_SYNC
#define LCD_VSYNC_GPIO      GPIO_NUM_45   // LCD_V_SYNC
#define LCD_DE_GPIO         GPIO_NUM_43   // LCD_H_EN
#define LCD_PCLK_GPIO       GPIO_NUM_40   // LCD_PCLK
#define LCD_DATA0_GPIO      GPIO_NUM_8    // B3
#define LCD_DATA1_GPIO      GPIO_NUM_9    // B4
#define LCD_DATA2_GPIO      GPIO_NUM_10   // B5
#define LCD_DATA3_GPIO      GPIO_NUM_11   // B6
#define LCD_DATA4_GPIO      GPIO_NUM_12   // B7
#define LCD_DATA5_GPIO      GPIO_NUM_13   // G2
#define LCD_DATA6_GPIO      GPIO_NUM_14   // G3
#define LCD_DATA7_GPIO      GPIO_NUM_15   // G4
#define LCD_DATA8_GPIO      GPIO_NUM_16   // G5
#define LCD_DATA9_GPIO      GPIO_NUM_17   // G6
#define LCD_DATA10_GPIO     GPIO_NUM_18   // G7
#define LCD_DATA11_GPIO     GPIO_NUM_19   // R3
#define LCD_DATA12_GPIO     GPIO_NUM_33   // R4
#define LCD_DATA13_GPIO     GPIO_NUM_34   // R5
#define LCD_DATA14_GPIO     GPIO_NUM_35   // R6
#define LCD_DATA15_GPIO     GPIO_NUM_36   // R7

static char const* TAG = "BSP display";

static bool bsp_display_initialized = false;
static esp_lcd_panel_handle_t lcd_panel = NULL;
static SemaphoreHandle_t        flush_semaphore         = NULL;

IRAM_ATTR static bool bsp_display_flush_ready(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t* edata,
                                              void* user_ctx) {
    xSemaphoreGiveFromISR(flush_semaphore, NULL);
    return false;
}

static esp_err_t bsp_display_initialize_panel(const bsp_display_configuration_t* configuration) {
    esp_lcd_rgb_panel_config_t rgb_cfg = {
        .clk_src = LCD_CLK_SRC_PLL160M,
        .timings = {
            .pclk_hz = 18 * 1000 * 1000,          // 18 MHz
            .h_res = 800,
            .v_res = 480,
            .hsync_pulse_width = 40,
            .hsync_back_porch = 40,
            .hsync_front_porch = 48,
            .vsync_pulse_width = 23,
            .vsync_back_porch = 32,
            .vsync_front_porch = 13,
            .flags = {
                .pclk_active_neg = true,
            },
        },
        .data_width = 16,
        .in_color_format = LCD_COLOR_FMT_RGB565,
        .out_color_format = LCD_COLOR_FMT_RGB565,
        .num_fbs = configuration->num_fbs,
        .bounce_buffer_size_px = 0,
        .dma_burst_size = 64,
        .hsync_gpio_num = LCD_HSYNC_GPIO,
        .vsync_gpio_num = LCD_VSYNC_GPIO,
        .de_gpio_num = LCD_DE_GPIO,
        .pclk_gpio_num = LCD_PCLK_GPIO,
        .disp_gpio_num = -1,
        .data_gpio_nums = {
            LCD_DATA0_GPIO,
            LCD_DATA1_GPIO,
            LCD_DATA2_GPIO,
            LCD_DATA3_GPIO,
            LCD_DATA4_GPIO,
            LCD_DATA5_GPIO,
            LCD_DATA6_GPIO,
            LCD_DATA7_GPIO,
            LCD_DATA8_GPIO,
            LCD_DATA9_GPIO,
            LCD_DATA10_GPIO,
            LCD_DATA11_GPIO,
            LCD_DATA12_GPIO,
            LCD_DATA13_GPIO,
            LCD_DATA14_GPIO,
            LCD_DATA15_GPIO,
        },
        .flags = {
            .fb_in_psram = true,
        },
    };

    esp_err_t res = esp_lcd_new_rgb_panel(&rgb_cfg, &lcd_panel);
    if (res != ESP_OK) {
        return res;
    }

    res = esp_lcd_panel_reset(lcd_panel);
    if (res != ESP_OK) {
        return res;
    }

    res = esp_lcd_panel_init(lcd_panel);
    if (res != ESP_OK) {
        return res;
    }

    return res;
}


static esp_err_t bsp_display_initialize_flush(void) {
    flush_semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(flush_semaphore);
    esp_lcd_rgb_panel_event_callbacks_t callbacks = {
        .on_color_trans_done = bsp_display_flush_ready,
    };
    return esp_lcd_rgb_panel_register_event_callbacks(lcd_panel, &callbacks, NULL);
}

// Public functions

esp_err_t bsp_display_initialize(const bsp_display_configuration_t* configuration) {
    if (bsp_display_initialized) {
        return ESP_OK;
    }
    ESP_RETURN_ON_ERROR(bsp_display_initialize_panel(configuration), TAG, "Failed to initialize panel");
    ESP_RETURN_ON_ERROR(bsp_display_initialize_flush(), TAG, "Failed to initialize flush callback");
    bsp_display_initialized = true;
    return ESP_OK;
}

esp_err_t bsp_display_get_parameters(size_t* h_res, size_t* v_res, bsp_display_color_format_t* color_fmt,
                                     bsp_display_endianness_t* data_endian) {
    if (!bsp_display_initialized) {
        return ESP_FAIL;
    }

    if (h_res) {
        *h_res = 800;
    }

    if (v_res) {
        *v_res = 480;
    }

    if (color_fmt) {
        *color_fmt = BSP_DISPLAY_COLOR_FORMAT_16_565RGB;
    }

    if (data_endian) {
        *data_endian = BSP_DISPLAY_ENDIAN_LITTLE;
    }
    return ESP_OK;
}

esp_err_t bsp_display_get_panel(esp_lcd_panel_handle_t* panel) {
    if (!bsp_display_initialized) {
        return ESP_FAIL;
    }
    *panel = lcd_panel;
    return ESP_OK;
}

esp_err_t bsp_display_get_panel_io(esp_lcd_panel_io_handle_t* panel_io) {
    if (!bsp_display_initialized) {
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
    return esp_lcd_panel_draw_bitmap(lcd_panel, x_start, y_start, x_end, y_end, buffer);
}
