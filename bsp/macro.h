#pragma once

#include "esp_err.h"

#define BSP_RETURN_ON_FAILURE(x, ...)                                          \
    do {                                                                       \
        esp_err_t err_rc_ = (x);                                               \
        if (unlikely(err_rc_ != ESP_OK && err_rc_ != ESP_ERR_NOT_SUPPORTED)) { \
            __VA_ARGS__;                                                       \
            return err_rc_;                                                    \
        }                                                                      \
    } while (0)
