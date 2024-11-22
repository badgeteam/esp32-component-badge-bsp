#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_lcd_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "tanmatsu_coprocessor.h"

#include <stdbool.h>
#include <stdint.h>

// Badge BSP
// WHY2025 specific APIs (internal to BSP)

/// @brief Get coprocessor handle
/// @return ESP-IDF error code
esp_err_t bsp_why2025_coprocessor_get_handle(tanmatsu_coprocessor_handle_t *handle);

// Coprocessor callbacks
void bsp_internal_coprocessor_keyboard_callback(tanmatsu_coprocessor_handle_t handle, tanmatsu_coprocessor_keys_t *prev_keys, tanmatsu_coprocessor_keys_t *keys);
void bsp_internal_coprocessor_input_callback(tanmatsu_coprocessor_handle_t handle, tanmatsu_coprocessor_inputs_t *prev_inputs, tanmatsu_coprocessor_inputs_t *inputs);
void bsp_internal_coprocessor_faults_callback(tanmatsu_coprocessor_handle_t handle, tanmatsu_coprocessor_pmic_faults_t *prev_faults, tanmatsu_coprocessor_pmic_faults_t *faults);
