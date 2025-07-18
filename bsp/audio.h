#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "driver/i2s_types.h"
#include "esp_err.h"

/// @brief BSP audio set sample rate
/// @return ESP-IDF error code
esp_err_t bsp_audio_set_rate(uint32_t rate);

/// @brief BSP audio get volume
/// @return ESP-IDF error code
esp_err_t bsp_audio_get_volume(float* out_percentage);

/// @brief BSP audio set volume
/// @return ESP-IDF error code
esp_err_t bsp_audio_set_volume(float percentage);

/// @brief Enable or disable speaker amplifier
/// @return ESP-IDF error code
esp_err_t bsp_audio_set_amplifier(bool enable);

/// @brief Get I2S handle
/// @param out_handle Pointer to I2S handle
/// @return ESP-IDF error code
esp_err_t bsp_audio_get_i2s_handle(i2s_chan_handle_t* out_handle);
