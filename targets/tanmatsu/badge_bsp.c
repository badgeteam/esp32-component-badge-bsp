// Board support package API: Tanmatsu implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "badge_bsp.h"
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

esp_err_t badge_bsp_example(uint8_t value) {
    printf("Hello from Tanmatsu BSP!\r\n");
    return ESP_OK;
}

void badge_bsp_blah(void) {
    printf("This is the not so weak function...\r\n");
}