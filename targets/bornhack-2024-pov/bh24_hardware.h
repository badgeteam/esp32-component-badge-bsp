
// SPDX-FileCopyrightText: 2025 Julian Scheffers
// SPDX-License-Identifier: MIT

#pragma once

// I2C bus
#define BSP_I2C_BUS     0
#define BSP_I2C_SDA_PIN 6
#define BSP_I2C_SCL_PIN 7
#define BSP_I2C_SPEED   400000  // 400 kHz
#define BSP_I2C_TIMEOUT 250     // us

// Buttons.
#define BSP_GPIO_BTN_UP     2
#define BSP_GPIO_BTN_DOWN   8
#define BSP_GPIO_BTN_SELECT 9

// Addressable LEDs
#define BSP_LED_NUM      16
#define BSP_LED_DATA_PIN 10
