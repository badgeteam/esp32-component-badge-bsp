// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include "bsp/audio.h"
#include "esp_err.h"

esp_err_t __attribute__((weak)) bsp_audio_initialize(void) {
    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_audio_set_rate(uint32_t rate) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_audio_get_volume(float* out_percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_audio_set_volume(float percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_audio_set_amplifier(bool enable) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_audio_get_i2s_handle(i2s_chan_handle_t* out_handle) {
    return ESP_ERR_NOT_SUPPORTED;
}
