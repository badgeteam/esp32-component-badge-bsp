#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

// Badge BSP
// Battery related APIs

/// @brief Get whether or not the battery is charging
/// @return ESP-IDF error code
esp_err_t bsp_battery_is_charging(bool *charging);

/// @brief Get the battery voltage in mV
/// @return ESP-IDF error code
esp_err_t bsp_battery_get_voltage(uint16_t *bat_mv);
