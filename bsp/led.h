#pragma once

#include "esp_err.h"

#include <stdbool.h>
#include <stdint.h>

// Badge BSP
// LED APIs

/// @brief Initialize BSP led subsystem
/// @return ESP-IDF error code
esp_err_t bsp_led_initialize(void);

/// @brief Write data to LEDs
/// @return ESP-IDF error code
esp_err_t bsp_led_write(uint8_t *data, uint32_t length);
