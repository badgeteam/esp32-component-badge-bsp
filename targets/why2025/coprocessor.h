
// SPDX-License-Identifier: MIT

#pragma once

#include "esp_err.h"
#include "tanmatsu_coprocessor.h"

#include <stdbool.h>

/* ==== platform-specific functions ==== */

// Enable the WHY2025 badge internal I²C bus.
extern bool why2025_enable_i2cint;

// Get the C6 version.
esp_err_t bsp_ch32_version(uint16_t *ver);
// Initialise the co-processor drivers.
esp_err_t bsp_why2025_coproc_init(tanmatsu_coprocessor_handle_t* coprocessor_handle);

// Set the display backlight value.
esp_err_t ch32_set_display_backlight(uint8_t value);
// Set the keyboard backlight value.
esp_err_t ch32_set_keyboard_backlight(uint8_t value);
// Get the display backlight value.
esp_err_t ch32_get_display_backlight(uint8_t *value);
// Get the keyboard backlight value.
esp_err_t ch32_get_keyboard_backlight(uint8_t *value);
// Enable/disable the audio amplifier.
esp_err_t bsp_amplifier_control(bool enable);
// Enable/disable the ESP32-C6 via RESET and BOOT pins.
esp_err_t bsp_c6_control(bool enable, bool boot);
// Initialize the ESP32-C6 after it was (re-)enabled.
esp_err_t bsp_c6_init();
