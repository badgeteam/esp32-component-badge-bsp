// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "badge_bsp.h"
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

// ALL FUNCTIONS IN THIS FILE MUST BE DEFINED AS WEAK

esp_err_t __attribute__((weak)) badge_bsp_example(uint8_t value) {
    printf("Hello from stub BSP!\r\n");
    return ESP_OK;
}

void __attribute__((weak)) badge_bsp_blah(void) {
    printf("This is the weak function...\r\n");
}
