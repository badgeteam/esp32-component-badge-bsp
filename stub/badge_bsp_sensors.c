#include "bsp/sensors.h"
#include "esp_err.h"

bool __attribute__((weak)) bsp_sensor_get_supported(bsp_sensor_type_t type) {
    (void)type;
    return false;
}

esp_err_t __attribute__((weak)) bsp_sensor_enable(bsp_sensor_type_t type) {
    (void)type;
    return ESP_ERR_NOT_SUPPORTED;
}

/// @brief Disable a sensor
/// @return ESP-IDF error code
esp_err_t __attribute__((weak)) bsp_sensor_disable(bsp_sensor_type_t type) {
    (void)type;
    return ESP_ERR_NOT_SUPPORTED;
}

/// @brief Check if a sensor is ready
/// @return ESP-IDF error code
esp_err_t __attribute__((weak)) bsp_sensor_status(bsp_sensor_type_t type, bool* out_ready) {
    (void)type;
    if (out_ready == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *out_ready = false;
    return ESP_OK;
}

/// @brief Read a sensor
/// @return ESP-IDF error code, returns ESP_ERR_NOT_FOUND if not supported
esp_err_t __attribute__((weak)) bsp_sensor_read(bsp_sensor_type_t type, float* out_value) {
    (void)type;
    return ESP_ERR_NOT_SUPPORTED;
}
