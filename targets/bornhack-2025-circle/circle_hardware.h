// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#pragma once

// I2C bus
#define BSP_I2C_BUS     0
#define BSP_I2C_SDA_PIN 9
#define BSP_I2C_SCL_PIN 8
#define BSP_I2C_SPEED   400000  // 400 kHz
#define BSP_I2C_TIMEOUT 250     // us

// Addressable LEDs
#define BSP_LED_NUM 5
#define BSP_LED_DATA_PIN 0

// SAO port
#define BSP_SAO_IO1_PIN 20
#define BSP_SAO_IO2_PIN 21

// LoRa module
#define BSP_LORA_DIO1_PIN      1
#define BSP_LORA_MISO_PIN      2
#define BSP_LORA_BUSY_PIN      3
#define BSP_LORA_RESET_PIN     4
#define BSP_LORA_RF_SWITCH_PIN 5
#define BSP_LORA_SCK_PIN       6
#define BSP_LORA_MOSI_PIN      7
#define BSP_LORA_NSS_PIN       10
