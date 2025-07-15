// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include "bsp/led.h"
#include "circle_hardware.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "led_strip.h"

static char const* TAG = "BSP: LEDs";

static led_strip_handle_t led_strip = NULL;

esp_err_t bsp_led_initialize(void) {
    led_strip_config_t strip_config = {
        .strip_gpio_num         = BSP_LED_DATA_PIN,
        .max_leds               = BSP_LED_NUM,
        .led_model              = LED_MODEL_SK6812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags =
            {
                .invert_out = false,
            },
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src           = RMT_CLK_SRC_DEFAULT,
        .resolution_hz     = (10 * 1000 * 1000),
        .mem_block_symbols = 0,
        .flags =
            {
                .with_dma = false,
            },
    };

    return led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
}

esp_err_t bsp_led_write(const uint8_t* data, uint32_t length) {
    if (led_strip == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    if (length % 3 != 0) {
        return ESP_ERR_INVALID_ARG;
    }
    for (uint32_t i = 0; i < length; i += 3) {
        led_strip_set_pixel(led_strip, i / 3, data[i], data[i + 1], data[i + 2]);
    }
    return led_strip_refresh(led_strip);
}
