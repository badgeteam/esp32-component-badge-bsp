// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include "bsp/audio.h"
#include "bsp/i2c.h"
#include "bsp/tanmatsu.h"
#include "driver/i2s_std.h"
#include "es8156.h"
#include "esp_check.h"
#include "esp_err.h"
#include "tanmatsu_coprocessor.h"
#include "tanmatsu_hardware.h"

static char const* TAG = "BSP: audio";

static i2c_master_bus_handle_t codec_i2c_bus_handle    = NULL;
static SemaphoreHandle_t       codec_i2c_bus_semaphore = NULL;
static es8156_handle_t         codec_handle            = NULL;
static i2s_chan_handle_t       i2s_handle              = NULL;

static esp_err_t initialize_i2s(uint32_t rate) {
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(0, I2S_ROLE_MASTER);

    esp_err_t res = i2s_new_channel(&chan_cfg, &i2s_handle, NULL);
    if (res != ESP_OK) {
        return res;
    }

    i2s_std_config_t i2s_config = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(rate),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg =
            {
                .mclk = BSP_I2S_MCLK,
                .bclk = BSP_I2S_BCLK,
                .ws   = BSP_I2S_WS,
                .dout = BSP_I2S_DOUT,
                .din  = I2S_GPIO_UNUSED,
                .invert_flags =
                    {
                        .mclk_inv = false,
                        .bclk_inv = false,
                        .ws_inv   = false,
                    },
            },
    };

    res = i2s_channel_init_std_mode(i2s_handle, &i2s_config);
    if (res != ESP_OK) {
        return res;
    }

    res = i2s_channel_enable(i2s_handle);
    if (res != ESP_OK) {
        return res;
    }

    return ESP_OK;
}

esp_err_t bsp_audio_set_rate(uint32_t rate) {
    i2s_std_clk_config_t clk_config = I2S_STD_CLK_DEFAULT_CONFIG(rate);
    return i2s_channel_reconfig_std_clock(i2s_handle, &clk_config);
}

esp_err_t bsp_audio_initialize(uint32_t rate) {
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
    res = es8156_configure(codec_handle);
    if (res != ESP_OK) return res;

    return initialize_i2s(44100);
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

esp_err_t bsp_audio_get_i2s_handle(i2s_chan_handle_t* out_handle) {
    if (out_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *out_handle = i2s_handle;
    return ESP_OK;
}

/* For future use when implementing I2S power saving:
   es8156_powerdown(codec_handle);
   es8156_standby_nopop(codec_handle);
   es8156_reset(codec_handle); */
