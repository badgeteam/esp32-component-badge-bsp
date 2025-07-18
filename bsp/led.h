#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

/// @brief Write data to LEDs
/// @return ESP-IDF error code
esp_err_t bsp_led_write(const uint8_t* data, uint32_t length);
