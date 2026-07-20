// SPDX-FileCopyrightText: 2026 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/display.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "hackaday2025_hardware.h"

#define MAX_TRANSFER_SIZE (4096 * 8)

static const char* TAG = "BSP DISPLAY";

static spi_device_handle_t device;

static esp_err_t spi_initialize(void) {
    spi_bus_config_t buscfg = {
        .miso_io_num     = -1,
        .mosi_io_num     = BSP_LCD_DATA,
        .sclk_io_num     = BSP_LCD_SCK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = MAX_TRANSFER_SIZE,
    };

    return spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
}

static esp_err_t send_command(uint8_t command, uint8_t num_params, uint8_t* params) {
    esp_err_t res;

    res = spi_device_acquire_bus(device, portMAX_DELAY);
    if (res != ESP_OK) {
        return res;
    }

    gpio_set_level(BSP_LCD_DC, 0);
    spi_transaction_t cmd_trans = {
        .flags     = 0,
        .length    = 8,
        .tx_buffer = &command,
    };
    res = spi_device_transmit(device, &cmd_trans);
    // printf("Command 0x%02x with %u parameters: 0x%02x\r\n", command, num_params, res);
    if (res != ESP_OK || num_params == 0) {
        spi_device_release_bus(device);
        return res;
    }

    gpio_set_level(BSP_LCD_DC, 1);
    spi_transaction_t data_trans = {
        .flags     = 0,
        .length    = num_params * 8,
        .tx_buffer = params,
    };
    res = spi_device_transmit(device, &data_trans);
    spi_device_release_bus(device);
    return res;
}

static esp_err_t send_data(size_t length, const uint8_t* data) {
    gpio_set_level(BSP_LCD_DC, 1);
    esp_err_t res = ESP_OK;
    while (length > 0 && res == ESP_OK) {
        size_t            chunk       = length < MAX_TRANSFER_SIZE ? length : MAX_TRANSFER_SIZE;
        spi_transaction_t transaction = {
            .length    = chunk * 8,
            .tx_buffer = data,
        };
        res     = spi_device_transmit(device, &transaction);
        data   += chunk;
        length -= chunk;
    }
    return res;
}

esp_err_t bsp_display_initialize(const bsp_display_configuration_t* configuration) {
    (void)configuration;

    esp_err_t res = spi_initialize();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init display spi");
        return res;
    }

    gpio_config_t int_pin_cfg = {
        .pin_bit_mask = BIT64(BSP_LCD_BACKLIGHT) | BIT64(BSP_LCD_DC) | BIT64(BSP_LCD_RESET),
        .mode         = GPIO_MODE_OUTPUT,
    };
    res = gpio_config(&int_pin_cfg);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init display gpio");
        return res;
    }

    spi_device_interface_config_t devcfg = {
        .command_bits   = 0,
        .clock_speed_hz = 40000000,
        .mode           = 0,
        .spics_io_num   = BSP_LCD_CS,
        .queue_size     = 8,
    };
    res = spi_bus_add_device(SPI2_HOST, &devcfg, &device);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init display spi device");
        return res;
    }

    gpio_set_level(BSP_LCD_BACKLIGHT, 1);

    gpio_set_level(BSP_LCD_RESET, 0);
    vTaskDelay(pdMS_TO_TICKS(30));
    gpio_set_level(BSP_LCD_RESET, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    send_command(0xFF, 1, (uint8_t[]){0xA5});

    send_command(0x9a, 1, (uint8_t[]){0x08});
    send_command(0x9b, 1, (uint8_t[]){0x08});
    send_command(0x9c, 1, (uint8_t[]){0xb0});
    send_command(0x9d, 1, (uint8_t[]){0x16});
    send_command(0x9e, 1, (uint8_t[]){0xc4});
    send_command(0x8f, 2, (uint8_t[]){0x55, 0x04});
    send_command(0x84, 1, (uint8_t[]){0x90});
    send_command(0x83, 1, (uint8_t[]){0x7b});
    send_command(0x85, 1, (uint8_t[]){0x33});
    send_command(0x60, 1, (uint8_t[]){0x00});
    send_command(0x70, 1, (uint8_t[]){0x00});
    send_command(0x61, 1, (uint8_t[]){0x02});
    send_command(0x71, 1, (uint8_t[]){0x02});
    send_command(0x62, 1, (uint8_t[]){0x04});
    send_command(0x72, 1, (uint8_t[]){0x04});
    send_command(0x6c, 1, (uint8_t[]){0x29});
    send_command(0x7c, 1, (uint8_t[]){0x29});
    send_command(0x6d, 1, (uint8_t[]){0x31});
    send_command(0x7d, 1, (uint8_t[]){0x31});
    send_command(0x6e, 1, (uint8_t[]){0x0f});
    send_command(0x7e, 1, (uint8_t[]){0x0f});
    send_command(0x66, 1, (uint8_t[]){0x21});
    send_command(0x76, 1, (uint8_t[]){0x21});
    send_command(0x68, 1, (uint8_t[]){0x3A});
    send_command(0x78, 1, (uint8_t[]){0x3A});
    send_command(0x63, 1, (uint8_t[]){0x07});
    send_command(0x73, 1, (uint8_t[]){0x07});
    send_command(0x64, 1, (uint8_t[]){0x05});
    send_command(0x74, 1, (uint8_t[]){0x05});
    send_command(0x65, 1, (uint8_t[]){0x02});
    send_command(0x75, 1, (uint8_t[]){0x02});
    send_command(0x67, 1, (uint8_t[]){0x23});
    send_command(0x77, 1, (uint8_t[]){0x23});
    send_command(0x69, 1, (uint8_t[]){0x08});
    send_command(0x79, 1, (uint8_t[]){0x08});
    send_command(0x6a, 1, (uint8_t[]){0x13});
    send_command(0x7a, 1, (uint8_t[]){0x13});
    send_command(0x6b, 1, (uint8_t[]){0x13});
    send_command(0x7b, 1, (uint8_t[]){0x13});
    send_command(0x6f, 1, (uint8_t[]){0x00});
    send_command(0x7f, 1, (uint8_t[]){0x00});
    send_command(0x50, 1, (uint8_t[]){0x00});
    send_command(0x52, 1, (uint8_t[]){0xd6});
    send_command(0x53, 1, (uint8_t[]){0x08});
    send_command(0x54, 1, (uint8_t[]){0x08});
    send_command(0x55, 1, (uint8_t[]){0x1e});
    send_command(0x56, 1, (uint8_t[]){0x1c});

    send_command(0xa0, 3, (uint8_t[]){0x2b, 0x24, 0x00});

    send_command(0xa1, 1, (uint8_t[]){0x87});
    send_command(0xa2, 1, (uint8_t[]){0x86});
    send_command(0xa5, 1, (uint8_t[]){0x00});
    send_command(0xa6, 1, (uint8_t[]){0x00});
    send_command(0xa7, 1, (uint8_t[]){0x00});
    send_command(0xa8, 1, (uint8_t[]){0x36});
    send_command(0xa9, 1, (uint8_t[]){0x7e});
    send_command(0xaa, 1, (uint8_t[]){0x7e});
    send_command(0xB9, 1, (uint8_t[]){0x85});
    send_command(0xBA, 1, (uint8_t[]){0x84});
    send_command(0xBB, 1, (uint8_t[]){0x83});
    send_command(0xBC, 1, (uint8_t[]){0x82});
    send_command(0xBD, 1, (uint8_t[]){0x81});
    send_command(0xBE, 1, (uint8_t[]){0x80});
    send_command(0xBF, 1, (uint8_t[]){0x01});
    send_command(0xC0, 1, (uint8_t[]){0x02});
    send_command(0xc1, 1, (uint8_t[]){0x00});
    send_command(0xc2, 1, (uint8_t[]){0x00});
    send_command(0xc3, 1, (uint8_t[]){0x00});
    send_command(0xc4, 1, (uint8_t[]){0x33});
    send_command(0xc5, 1, (uint8_t[]){0x7e});
    send_command(0xc6, 1, (uint8_t[]){0x7e});
    send_command(0xC8, 2, (uint8_t[]){0x33, 0x33});
    send_command(0xC9, 1, (uint8_t[]){0x68});
    send_command(0xCA, 1, (uint8_t[]){0x69});
    send_command(0xCB, 1, (uint8_t[]){0x6a});
    send_command(0xCC, 1, (uint8_t[]){0x6b});
    send_command(0xCD, 2, (uint8_t[]){0x33, 0x33});
    send_command(0xCE, 1, (uint8_t[]){0x6c});
    send_command(0xCF, 1, (uint8_t[]){0x6d});
    send_command(0xD0, 1, (uint8_t[]){0x6e});
    send_command(0xD1, 1, (uint8_t[]){0x6f});
    send_command(0xAB, 2, (uint8_t[]){0x03, 0x67});
    send_command(0xAC, 2, (uint8_t[]){0x03, 0x6b});
    send_command(0xAD, 2, (uint8_t[]){0x03, 0x68});
    send_command(0xAE, 2, (uint8_t[]){0x03, 0x6c});
    send_command(0xb3, 1, (uint8_t[]){0x00});
    send_command(0xb4, 1, (uint8_t[]){0x00});
    send_command(0xb5, 1, (uint8_t[]){0x00});
    send_command(0xB6, 1, (uint8_t[]){0x32});
    send_command(0xB7, 1, (uint8_t[]){0x7e});
    send_command(0xB8, 1, (uint8_t[]){0x7e});
    send_command(0xe0, 1, (uint8_t[]){0x00});
    send_command(0xe1, 2, (uint8_t[]){0x03, 0x0f});
    send_command(0xe2, 1, (uint8_t[]){0x04});
    send_command(0xe3, 1, (uint8_t[]){0x01});
    send_command(0xe4, 1, (uint8_t[]){0x0e});
    send_command(0xe5, 1, (uint8_t[]){0x01});
    send_command(0xe6, 1, (uint8_t[]){0x19});
    send_command(0xe7, 1, (uint8_t[]){0x10});
    send_command(0xe8, 1, (uint8_t[]){0x10});
    send_command(0xea, 1, (uint8_t[]){0x12});
    send_command(0xeb, 1, (uint8_t[]){0xd0});
    send_command(0xec, 1, (uint8_t[]){0x04});
    send_command(0xed, 1, (uint8_t[]){0x07});
    send_command(0xee, 1, (uint8_t[]){0x07});
    send_command(0xef, 1, (uint8_t[]){0x09});
    send_command(0xf0, 1, (uint8_t[]){0xd0});
    send_command(0xf1, 1, (uint8_t[]){0x0e});
    send_command(0xF9, 1, (uint8_t[]){0x17});

    send_command(0xf2, 4, (uint8_t[]){0x2c, 0x1b, 0x0b, 0x20});

    send_command(0xe9, 1, (uint8_t[]){0x29});
    send_command(0xec, 1, (uint8_t[]){0x04});
    send_command(0x35, 1, (uint8_t[]){0x00});
    send_command(0x44, 2, (uint8_t[]){0x00, 0x10});
    send_command(0x46, 1, (uint8_t[]){0x10});

    vTaskDelay(pdMS_TO_TICKS(10));

    send_command(0xFF, 1, (uint8_t[]){0x00});

    send_command(0x3a, 1, (uint8_t[]){0x05});
    send_command(0x11, 0, NULL);
    vTaskDelay(pdMS_TO_TICKS(30));
    send_command(0x29, 0, NULL);

    ESP_LOGI(TAG, "Display init done");

    return ESP_OK;
}

esp_err_t bsp_display_get_parameters(size_t* h_res, size_t* v_res, bsp_display_color_format_t* color_fmt,
                                     bsp_display_endianness_t* data_endian) {
    if (h_res != NULL) {
        *h_res = 142;
    }
    if (v_res != NULL) {
        *v_res = 428;
    }
    if (color_fmt != NULL) {
        *color_fmt = BSP_DISPLAY_COLOR_FORMAT_16_565RGB;
    }
    if (data_endian != NULL) {
        *data_endian = BSP_DISPLAY_ENDIAN_BIG;
    }
    return ESP_OK;
}

esp_err_t bsp_display_get_panel(esp_lcd_panel_handle_t* panel) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_display_get_panel_io(esp_lcd_panel_io_handle_t* panel) {
    return ESP_ERR_NOT_SUPPORTED;
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
    size_t offset  = 12;
    x_start       += offset;
    x_end         += offset;

    send_command(0x2A, 4,
                 (uint8_t[]){
                     (x_start >> 8) & 0xFF,
                     x_start & 0xFF,
                     ((x_end - 1) >> 8) & 0xFF,
                     (x_end - 1) & 0xFF,
                 });
    send_command(0x2B, 4,
                 (uint8_t[]){
                     (y_start >> 8) & 0xFF,
                     y_start & 0xFF,
                     ((y_end - 1) >> 8) & 0xFF,
                     (y_end - 1) & 0xFF,
                 });
    send_command(0x2C, 0, NULL);
    size_t pixel_count = (x_end - x_start) * (y_end - y_start);
    return send_data(pixel_count * 2, (const uint8_t*)buffer);
}
