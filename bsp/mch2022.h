// MCH2022 specific APIs

#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "rp2040.h"

/// @brief Get coprocessor handle
/// @return ESP-IDF error code
esp_err_t bsp_mch2022_coprocessor_get_handle(RP2040* handle);

/// @brief Get coprocessor input queue
/// @return ESP-IDF error code
esp_err_t bsp_mch2022_coprocessor_get_queue(QueueHandle_t* queue);

/// @brief Coprocessor input callback
void bsp_mch2022_coprocessor_input_callback(rp2040_input_t input, bool state);
