// Board support package API: Tanmatsu implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/rtc.h"
#include "bsp/tanmatsu.h"
#include "esp_check.h"
#include "tanmatsu_coprocessor.h"

#include <stdint.h>

#include <sys/time.h>
#include <time.h>

static char const *TAG = "BSP RTC";

esp_err_t bsp_rtc_get_time(uint32_t *value) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_get_real_time(handle, value), TAG, "Failed to get RTC value");
    return ESP_OK;
}

esp_err_t bsp_rtc_set_time(uint32_t value) {
    tanmatsu_coprocessor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(bsp_tanmatsu_coprocessor_get_handle(&handle), TAG, "Failed to get coprocessor handle");
    ESP_RETURN_ON_ERROR(tanmatsu_coprocessor_set_real_time(handle, value), TAG, "Failed to set RTC value");
    return ESP_OK;
}

esp_err_t bsp_rtc_update_time(void) {
    uint32_t value;
    ESP_RETURN_ON_ERROR(bsp_rtc_get_time(&value), TAG, "Failed to get RTC value");
    struct timeval rtc_timeval = {
        .tv_sec  = value,
        .tv_usec = 0,
    };
    settimeofday(&rtc_timeval, NULL);
    return ESP_OK;
}
