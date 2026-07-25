#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "bsp/display.h"
#include "bsp/i2c.h"
#include "bsp/input.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "hal/gpio_types.h"

static char const* TAG = "BSP: INPUT";

static QueueHandle_t             event_queue  = NULL;
static esp_lcd_panel_io_handle_t tp_io_handle = NULL;
static esp_lcd_touch_handle_t    tp_handle    = NULL;

esp_err_t bsp_input_initialize(void) {
    if (event_queue == NULL) {
        event_queue = xQueueCreate(32, sizeof(bsp_input_event_t));
        ESP_RETURN_ON_FALSE(event_queue, ESP_ERR_NO_MEM, TAG, "Failed to create input event queue");
    }

    // GT911 touch screen
    i2c_master_bus_handle_t i2c_bus = NULL;
    esp_err_t               res     = bsp_i2c_primary_bus_get_handle(&i2c_bus);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get I2C bus: %s", esp_err_to_name(res));
        return res;
    }

    esp_lcd_panel_io_i2c_config_t io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    res                                     = esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &tp_io_handle);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize GT911 touch panel IO handle: %s", esp_err_to_name(res));
        return res;
    }

    esp_lcd_panel_io_i2c_config_t tp_gt911_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();

    esp_lcd_touch_io_gt911_config_t tp_gt911_config = {
        .dev_addr = io_config.dev_addr,
    };

    size_t display_h_res = 0;
    size_t display_v_res = 0;
    res                  = bsp_display_get_parameters(&display_h_res, &display_v_res, NULL, NULL);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get display parameters: %s", esp_err_to_name(res));
        return res;
    }

    esp_lcd_touch_config_t tp_cfg = {
        .x_max        = display_h_res,
        .y_max        = display_v_res,
        .rst_gpio_num = -1,
        .int_gpio_num = -1,
        .levels =
            {
                .reset     = 0,
                .interrupt = 0,
            },
        .flags =
            {
                .swap_xy  = 0,
                .mirror_x = 1,
                .mirror_y = 1,
            },
        .driver_data = &tp_gt911_config,
    };

    res = esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &tp_handle);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize GT911 touch panel: %s", esp_err_to_name(res));
        return res;
    }

    return ESP_OK;
}

esp_err_t bsp_input_get_queue(QueueHandle_t* out_queue) {
    if (out_queue == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (event_queue == NULL) {
        return ESP_FAIL;
    }
    *out_queue = event_queue;
    return ESP_OK;
}

bool bsp_input_needs_on_screen_keyboard(void) {
    return false;
}

esp_err_t bsp_input_read_navigation_key(bsp_input_navigation_key_t key, bool* out_state) {
    *out_state = false;
    return ESP_OK;
}

esp_err_t bsp_input_read_action(bsp_input_action_type_t action, bool* out_state) {
    *out_state = false;
    return ESP_OK;
}

esp_err_t bsp_input_get_touch_coordinates(uint16_t* out_x, uint16_t* out_y, uint16_t* out_strength, uint8_t* out_count,
                                          uint8_t max_count) {
    esp_err_t res = esp_lcd_touch_read_data(tp_handle);
    if (res != ESP_OK) {
        return res;
    }

    bool pressed = esp_lcd_touch_get_coordinates(tp_handle, out_x, out_y, out_strength, out_count, max_count);

    return ESP_OK;
}
