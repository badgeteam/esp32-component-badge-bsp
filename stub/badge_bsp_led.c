// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include "bsp/led.h"
#include "esp_err.h"

esp_err_t __attribute__((weak)) bsp_led_initialize(void) {
    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_led_write(const uint8_t* data, uint32_t length) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_led_set_brightness(uint8_t percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_led_get_brightness(uint8_t* out_percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_led_set_mode(bool automatic) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_led_get_mode(bool* out_automatic) {
    (void)out_automatic;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_led_send(void) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_led_clear(void) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_led_set_pixel(uint32_t index, uint32_t color) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_led_set_pixel_rgb(uint32_t index, uint8_t red, uint8_t green, uint8_t blue) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_led_set_pixel_rgbw(uint32_t index, uint8_t red, uint8_t green, uint8_t blue,
                                                       uint8_t white) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_led_set_pixel_hsv(uint32_t index, uint16_t hue, uint8_t saturation, uint8_t value) {
    return ESP_ERR_NOT_SUPPORTED;
}
