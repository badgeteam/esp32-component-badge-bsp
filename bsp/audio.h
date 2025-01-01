#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

// Badge BSP
// Audio APIs

/// @brief Initialize BSP audio subsystem
/// @return ESP-IDF error code
esp_err_t bsp_audio_initialize(void);

/// @brief BSP audio get volume
/// @return ESP-IDF error code
esp_err_t bsp_audio_get_volume(float* out_percentage);

/// @brief BSP audio set volume
/// @return ESP-IDF error code
esp_err_t bsp_audio_set_volume(float percentage);
