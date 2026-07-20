#pragma once

#include <stdbool.h>
#include "esp_err.h"

typedef enum {
    BSP_SENSOR_TYPE_TEMPERATURE         = 0,   // Degrees Celsius
    BSP_SENSOR_TYPE_RELATIVE_HUMIDITY   = 1,   // Percentage
    BSP_SENSOR_TYPE_BAROMETRIC_PRESSURE = 2,   // Pa
    BSP_SENSOR_TYPE_GAS_RESISTANCE      = 3,   // Ohm
    BSP_SENSOR_TYPE_CO2                 = 4,   // ppm
    BSP_SENSOR_TYPE_VOC                 = 5,   // ug/m3
    BSP_SENSOR_TYPE_VOC_PARTS           = 6,   // ppm
    BSP_SENSOR_TYPE_PARTICULATE_MATTER  = 7,   // ug/m3
    BSP_SENSOR_TYPE_LIGHT_LUX           = 8,   // Lux
    BSP_SENSOR_TYPE_LIGHT_UV            = 9,   // UVI
    BSP_SENSOR_TYPE_PROXIMITY           = 10,  // Meters
} bsp_sensor_type_t;

/// @brief Check if a specific sensor type is supported
/// @return ESP-IDF error code
bool bsp_sensor_get_supported(bsp_sensor_type_t type);

/// @brief Enable a sensor
/// @return ESP-IDF error code
esp_err_t bsp_sensor_enable(bsp_sensor_type_t type);

/// @brief Disable a sensor
/// @return ESP-IDF error code
esp_err_t bsp_sensor_disable(bsp_sensor_type_t type);

/// @brief Check if a sensor is ready
/// @return ESP-IDF error code
esp_err_t bsp_sensor_status(bsp_sensor_type_t type, bool* out_ready);

/// @brief Read a sensor
/// @return ESP-IDF error code, returns ESP_ERR_NOT_FOUND if not supported
esp_err_t bsp_sensor_read(bsp_sensor_type_t type, float* out_value);
