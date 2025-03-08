// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include "bsp/audio.h"
#include "bsp/i2c.h"
#include "bsp/tanmatsu.h"
#include "es8156.h"
#include "esp_check.h"
#include "esp_err.h"
#include "freertos/projdefs.h"
#include "tanmatsu_coprocessor.h"
#include "tanmatsu_hardware.h"

static char const* TAG = "BSP: audio";

static i2c_master_bus_handle_t codec_i2c_bus_handle    = NULL;
static SemaphoreHandle_t       codec_i2c_bus_semaphore = NULL;
static es8156_handle_t         codec_handle            = {0};

esp_err_t bsp_audio_initialize(void) {
    ESP_RETURN_ON_ERROR(bsp_i2c_primary_bus_get_handle(&codec_i2c_bus_handle), TAG, "Failed to get I2C bus handle");
    ESP_RETURN_ON_ERROR(bsp_i2c_primary_bus_get_semaphore(&codec_i2c_bus_semaphore), TAG,
                        "Failed to get I2C bus semaphore");

    es8156_config_t configuration = {
        .i2c_bus               = codec_i2c_bus_handle,
        .i2c_address           = BSP_ES8156_I2C_ADDRESS,
        .concurrency_semaphore = codec_i2c_bus_semaphore,
    };

    esp_err_t res = es8156_initialize(&configuration, &codec_handle);
    if (res != ESP_OK) return res;
    return es8156_configure(codec_handle);
}

esp_err_t bsp_audio_get_volume(float* out_percentage) {
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_audio_set_volume(float percentage) {
    float value = 180.0 * (percentage / 100.0);
    return es8156_write_volume_control(codec_handle, value);
}

esp_err_t bsp_audio_set_amplifier(bool enable) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get the coprocessor handle");
    return tanmatsu_coprocessor_set_amplifier_enable(handle, enable);
}

void bsp_audio_test(void) {
    printf("--- configure ---\r\n");
    es8156_configure(codec_handle);
    printf("--- powerdown ---\r\n");
    es8156_powerdown(codec_handle);
    printf("--- standby ---\r\n");
    es8156_standby_nopop(codec_handle);
    printf("--- reset ---\r\n");
    es8156_reset(codec_handle);
}
