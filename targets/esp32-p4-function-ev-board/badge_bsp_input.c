// Board support package API: ESP32-P4 Function EV board implementation
// SPDX-FileCopyrightText: 2026 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "bsp/input.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "hal/gpio_types.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_lcd_panel_io_interface.h"
#include "driver/i2c_master.h"
#include "bsp/display.h"
#include "bsp/i2c.h"

static char const* TAG = "BSP input";
static QueueHandle_t event_queue = NULL;
static esp_lcd_panel_io_t tp_io_handle;
static i2c_master_bus_handle_t touch_i2c_bus_handle = NULL;
static SemaphoreHandle_t touch_i2c_semaphore  = NULL;
static esp_lcd_touch_handle_t tp;
static i2c_master_dev_handle_t tp_i2c_dev;

static void claim_i2c_bus(void) {
    // Claim I2C bus
    if (touch_i2c_semaphore != NULL) {
        xSemaphoreTake(touch_i2c_semaphore, portMAX_DELAY);
    } else {
        ESP_LOGW(TAG, "No concurrency semaphore");
    }
}

static void release_i2c_bus(void) {
    // Release I2C bus
    if (touch_i2c_semaphore != NULL) {
        xSemaphoreGive(touch_i2c_semaphore);
    }
}

static esp_err_t ts_i2c_master_transmit_receive(const uint8_t* write_buffer, size_t write_size,
                                                uint8_t* read_buffer, size_t read_size) {
    claim_i2c_bus();
    printf("Reading from touch ic...\r\n");
    esp_err_t res = i2c_master_transmit_receive(tp_i2c_dev, write_buffer, write_size, read_buffer, read_size, 100);
    release_i2c_bus();
    return res;
}

static esp_err_t ts_i2c_master_transmit(const uint8_t* write_buffer, size_t write_size) {
    claim_i2c_bus();
    printf("Writing to touch ic...\r\n");
    esp_err_t res = i2c_master_transmit(tp_i2c_dev, write_buffer, write_size, 100);
    release_i2c_bus();
    return res;
}

static esp_err_t touch_rx(esp_lcd_panel_io_t *io, int lcd_cmd, void *param, size_t param_size) {
    uint16_t cmd = (uint16_t) lcd_cmd;
    return ts_i2c_master_transmit_receive((uint8_t*)&cmd, sizeof(uint16_t), param, param_size);
}

static esp_err_t touch_tx(esp_lcd_panel_io_t *io, int lcd_cmd, const void *param, size_t param_size) {
    uint8_t buffer[17] = {0};
    memcpy(buffer, (uint8_t*)&lcd_cmd, sizeof(uint16_t));
    if (param_size > sizeof(buffer) - 1) {
        return ESP_FAIL;
    }
    memcpy(&buffer[2], param, param_size);
    return ts_i2c_master_transmit(buffer, param_size + 2);
}

static esp_err_t initialize_touch(void) {
    size_t h_res = 0;
    size_t v_res = 0;
    esp_err_t res = bsp_display_get_parameters(&h_res, &v_res, NULL, NULL);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get display parameters: %s", esp_err_to_name(res));
        return res;
    }

    tp_io_handle.rx_param = touch_rx;
    tp_io_handle.tx_param = touch_tx;
    tp_io_handle.tx_color = NULL;
    tp_io_handle.del = NULL;
    tp_io_handle.register_event_callbacks = NULL;

    res = bsp_i2c_primary_bus_get_handle(&touch_i2c_bus_handle);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get I2C bus handle: %s", esp_err_to_name(res));
        return res;
    }

    for (uint8_t i = 1; i < 0xFF; i++) {
        esp_err_t r = i2c_master_probe(touch_i2c_bus_handle, i, 100);
        if (r == ESP_OK) {
            printf("Found device %02x on I2C bus!\r\n", i);
        }
    }

    res = bsp_i2c_primary_bus_get_semaphore(&touch_i2c_semaphore);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get I2C bus semaphore: %s", esp_err_to_name(res));
        return res;
    }

    esp_lcd_panel_io_i2c_config_t io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();

    esp_lcd_touch_io_gt911_config_t tp_gt911_config = {
        .dev_addr = io_config.dev_addr,
    };

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = h_res,
        .y_max = v_res,
        .rst_gpio_num = -1,
        .int_gpio_num = -1,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
        .driver_data = &tp_gt911_config,
    };

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = io_config.dev_addr,
        .scl_speed_hz    = 400000,
    };

    printf("Address: %02lx\r\n", io_config.dev_addr);

    res = i2c_master_bus_add_device(touch_i2c_bus_handle, &dev_cfg, &tp_i2c_dev);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add i2c device: %s", esp_err_to_name(res));
        return res;
    }

    return esp_lcd_touch_new_i2c_gt911(&tp_io_handle, &tp_cfg, &tp);
}

esp_err_t bsp_input_initialize(void) {
    if (event_queue == NULL) {
        event_queue = xQueueCreate(32, sizeof(bsp_input_event_t));
        ESP_RETURN_ON_FALSE(event_queue, ESP_ERR_NO_MEM, TAG, "Failed to create input event queue");
    }

    esp_err_t res = initialize_touch();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize touch panel: %s", esp_err_to_name(res));
        return res;
    }

    while (1) {
        esp_lcd_touch_point_data_t touch_point_data[1];
        uint8_t touch_cnt = 0;
        ESP_ERROR_CHECK(esp_lcd_touch_get_data(tp, touch_point_data, &touch_cnt, 1));
        if (touch_cnt > 0) {
            printf("X: %04u, Y: %04u, S: %04u\r\n", touch_point_data[0].x, touch_point_data[0].y, touch_point_data[0].strength);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
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
