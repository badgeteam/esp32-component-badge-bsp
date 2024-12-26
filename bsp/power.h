#pragma once

#include "esp_err.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BSP_POWER_CHARGING_FAULT_NONE        = 0,
    BSP_POWER_CHARGING_FAULT_TEMPERATURE = 1,
    BSP_POWER_CHARGING_FAULT_SAFETY      = 2,
    BSP_POWER_CHARGING_FAULT_INPUT       = 3,
} bsp_power_charging_fault_t;

typedef struct _bsp_power_battery_information {
    char const *type;                     // Battery type (string)
    bool        installed;                // Is a battery installed
    uint32_t    maximum_charging_current; // Maximum charging current (mA)
    uint32_t    voltage;                  // Current battery voltage
    uint32_t    charging_target_voltage;  // Target voltage when full
    float       remaining_percentage;     // Remaining capacity (%)
} bsp_power_battery_information_t;

/// @brief Initialize BSP power subsystem
/// @return ESP-IDF error code
esp_err_t bsp_power_initialize(void);

/// @brief Get power button state
/// @return ESP-IDF error code
esp_err_t bsp_power_get_button_state(bool *pressed);

/// @brief Get battery information
/// @return ESP-IDF error code
esp_err_t bsp_power_get_battery_information(bsp_power_battery_information_t *out_information);

/// @brief Get battery voltage
/// @return ESP-IDF error code
esp_err_t bsp_power_get_battery_voltage(uint32_t *millivolt);

/// @brief Get system voltage
/// @return ESP-IDF error code
esp_err_t bsp_power_get_system_voltage(uint32_t *millivolt);

/// @brief Get USB device port voltage
/// @return ESP-IDF error code
esp_err_t bsp_power_get_usb_device_voltage(uint32_t *millivolt);

/// @brief Get charging current
/// @return ESP-IDF error code
esp_err_t bsp_power_get_charging_state(bool *out_enabled, bool *out_battery_attached, bool *out_active, bool *out_done, uint32_t *out_current, uint32_t *out_voltage, float *out_percentage);

/// @brief Set charging current
/// @return ESP-IDF error code
esp_err_t bsp_power_configure_charging(bool enable, uint32_t current);

/// @brief Get battery charging enabled
/// @return ESP-IDF error code
esp_err_t bsp_power_get_charging_enabled(bool *enabled);

/// @brief Set battery charging enabled
/// @return ESP-IDF error code
esp_err_t bsp_power_set_charging_enabled(bool enable);

/// @brief Get USB host port boost enabled
/// @return ESP-IDF error code
esp_err_t bsp_power_get_usb_host_boost_enabled(bool *enabled);

/// @brief Set USB host port boost enabled
/// @return ESP-IDF error code
esp_err_t bsp_power_set_usb_host_boost_enabled(bool enable);
