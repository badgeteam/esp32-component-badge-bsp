#pragma once

#include "esp_err.h"

#include <stdbool.h>
#include <stdint.h>

// Badge BSP
// Real Time Clock APIs

/// @brief Get time
/// @return ESP-IDF error code
esp_err_t bsp_rtc_get_time(uint32_t *value);

/// @brief Set time
/// @return ESP-IDF error code
esp_err_t bsp_rtc_set_time(uint32_t value);

/// @brief Update system time from RTC
/// @return ESP-IDF error code
esp_err_t bsp_rtc_update_time(void);
