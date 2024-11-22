#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_lcd_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "tanmatsu_coprocessor.h"

#include <stdbool.h>
#include <stdint.h>

// Badge BSP
// WHY2025 specific APIs (internal to BSP)

/// @brief Get coprocessor handle
/// @return ESP-IDF error code
esp_err_t bsp_why2025_coprocessor_get_handle(tanmatsu_coprocessor_handle_t *handle);
