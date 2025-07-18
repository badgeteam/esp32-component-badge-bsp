#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

typedef enum {
    BSP_POWER_CHARGING_FAULT_NONE        = 0,
    BSP_POWER_CHARGING_FAULT_TEMPERATURE = 1,
    BSP_POWER_CHARGING_FAULT_SAFETY      = 2,
    BSP_POWER_CHARGING_FAULT_INPUT       = 3,
} bsp_power_charging_fault_t;

typedef struct _bsp_power_battery_information {
    char const* type;                      // Battery type (string)
    bool        power_supply_available;    // Is a power supply attached
    bool        battery_available;         // Is a battery installed
    bool        charging_disabled;         // Is battery charging disabled
    bool        battery_charging;          // Is the battery being charged
    uint16_t    maximum_charging_current;  // Maximum charging current (mA)
    uint16_t    current_charging_current;  // Maximum charging current (mA)
    uint16_t    voltage;                   // Current battery voltage
    uint16_t    charging_target_voltage;   // Target voltage when full
    double      remaining_percentage;      // Remaining capacity (%)
} bsp_power_battery_information_t;

typedef enum {
    BSP_POWER_RADIO_STATE_OFF         = 0,
    BSP_POWER_RADIO_STATE_BOOTLOADER  = 1,
    BSP_POWER_RADIO_STATE_APPLICATION = 2,
} bsp_radio_state_t;

/// @brief Get battery information
/// @return ESP-IDF error code
esp_err_t bsp_power_get_battery_information(bsp_power_battery_information_t* out_information);

/// @brief Get battery voltage
/// @return ESP-IDF error code
esp_err_t bsp_power_get_battery_voltage(uint16_t* out_millivolt);

/// @brief Get system voltage
/// @return ESP-IDF error code
esp_err_t bsp_power_get_system_voltage(uint16_t* out_millivolt);

/// @brief Get charger input voltage
/// @return ESP-IDF error code
esp_err_t bsp_power_get_input_voltage(uint16_t* out_millivolt);

/// @brief Get battery charger configuration
/// @return ESP-IDF error code
esp_err_t bsp_power_get_charging_configuration(bool* out_disabled, uint16_t* out_current);

/// @brief Configure battery charger
/// @return ESP-IDF error code
esp_err_t bsp_power_configure_charging(bool disable, uint16_t current);

/// @brief Get USB host port boost enabled
/// @return ESP-IDF error code
esp_err_t bsp_power_get_usb_host_boost_enabled(bool* out_enabled);

/// @brief Set USB host port boost enabled
/// @return ESP-IDF error code
esp_err_t bsp_power_set_usb_host_boost_enabled(bool enable);

/// @brief Get radio enabled
/// @return ESP-IDF error code
esp_err_t bsp_power_get_radio_state(bsp_radio_state_t* out_state);

/// @brief Set radio enabled
/// @return ESP-IDF error code
esp_err_t bsp_power_set_radio_state(bsp_radio_state_t state);

/// @brief Power off
/// @return ESP-IDF error code
esp_err_t bsp_power_off(bool enable_alarm_wakeup);
