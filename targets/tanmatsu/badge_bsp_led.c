// Board support package API: Tanmatsu implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <math.h>
#include <stdint.h>
#include <string.h>
#include "bsp/led.h"
#include "bsp/tanmatsu.h"
#include "esp_check.h"
#include "esp_err.h"
#include "tanmatsu_coprocessor.h"
#include "tanmatsu_hardware.h"

static char const* TAG = "BSP: LEDs";

static uint8_t led_data[3 * BSP_LED_COUNT] = {0};

esp_err_t bsp_led_initialize(void) {
    return ESP_OK;
}

esp_err_t bsp_led_write(const uint8_t* data, uint32_t length) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    if (length > sizeof(led_data)) {
        return ESP_ERR_INVALID_SIZE;
    }
    memcpy(led_data, data, length);
    return tanmatsu_coprocessor_set_led_data(handle, data, length);
}

esp_err_t bsp_led_set_brightness(uint8_t percentage) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    uint8_t brightness = (255 * percentage) / 100;
    return tanmatsu_coprocessor_set_led_brightness(handle, brightness);
}

esp_err_t bsp_led_get_brightness(uint8_t* out_percentage) {
    if (out_percentage == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    uint8_t brightness = 0;
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_led_brightness(handle, &brightness), TAG,
                        "Failed to get LED brightness");
    *out_percentage = (brightness * 100) / 255;
    return ESP_OK;
}

esp_err_t bsp_led_set_mode(bool automatic) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    return tanmatsu_coprocessor_set_led_mode(handle, automatic);
}

esp_err_t bsp_led_get_mode(bool* out_automatic) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    return tanmatsu_coprocessor_get_led_mode(handle, out_automatic);
}

esp_err_t bsp_led_send(void) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    return tanmatsu_coprocessor_set_led_data(handle, led_data, sizeof(led_data));
}

esp_err_t bsp_led_clear(void) {
    memset(led_data, 0, sizeof(led_data));
    return bsp_led_send();
}

esp_err_t bsp_led_set_pixel(uint32_t index, uint32_t color) {
    uint8_t red   = (color >> 16) & 0xFF;
    uint8_t green = (color >> 8) & 0xFF;
    uint8_t blue  = color & 0xFF;
    return bsp_led_set_pixel_rgb(index, red, green, blue);
}

esp_err_t bsp_led_set_pixel_rgb(uint32_t index, uint8_t red, uint8_t green, uint8_t blue) {
    if (index >= BSP_LED_COUNT) {
        return ESP_ERR_INVALID_ARG;
    }
    led_data[index * 3 + 0] = green;
    led_data[index * 3 + 1] = red;
    led_data[index * 3 + 2] = blue;
    return ESP_OK;
}

esp_err_t bsp_led_set_pixel_rgbw(uint32_t index, uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
    if (index >= BSP_LED_COUNT) {
        return ESP_ERR_INVALID_ARG;
    }
    // Convert RGBW to RGB by adding white component to each color channel
    uint16_t r              = red + white;
    uint16_t g              = green + white;
    uint16_t b              = blue + white;
    // Clamp values to 255
    led_data[index * 3 + 0] = (g > 255) ? 255 : g;
    led_data[index * 3 + 1] = (r > 255) ? 255 : r;
    led_data[index * 3 + 2] = (b > 255) ? 255 : b;
    return ESP_OK;
}

esp_err_t bsp_led_set_pixel_hsv(uint32_t index, uint16_t hue, uint8_t saturation, uint8_t value) {
    uint8_t red, green, blue;

    // If saturation is zero, the color is a gray at the given value
    if (saturation == 0) {
        return bsp_led_set_pixel_rgb(index, value, value, value);
    }

    // Convert hue, saturation and value to RGB
    float h = (float)hue * 360.0f / 65536.0f;
    float s = (float)saturation / 255.0f;
    float v = (float)value / 255.0f;

    float hh     = h / 60.0f;
    int   sector = (int)floorf(hh) % 6;
    float f      = hh - floorf(hh);

    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);

    float rf, gf, bf;
    switch (sector) {
        case 0:
            rf = v;
            gf = t;
            bf = p;
            break;
        case 1:
            rf = q;
            gf = v;
            bf = p;
            break;
        case 2:
            rf = p;
            gf = v;
            bf = t;
            break;
        case 3:
            rf = p;
            gf = q;
            bf = v;
            break;
        case 4:
            rf = t;
            gf = p;
            bf = v;
            break;
        default:
            rf = v;
            gf = p;
            bf = q;
            break;
    }

    red   = (uint8_t)(rf * 255.0f + 0.5f);
    green = (uint8_t)(gf * 255.0f + 0.5f);
    blue  = (uint8_t)(bf * 255.0f + 0.5f);

    return bsp_led_set_pixel_rgb(index, red, green, blue);
}
