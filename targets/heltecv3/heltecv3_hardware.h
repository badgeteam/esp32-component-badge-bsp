// SPDX-FileCopyrightText: 2026 Nicolai Electronics
// SPDX-License-Identifier: MIT

#pragma once

// Buttons
#define BSP_GPIO_BTN 0

// LEDs
#define BSP_GPIO_LED 35

// Internal I2C bus
#define BSP_I2C_INTERNAL_BUS     0
#define BSP_I2C_INTERNAL_SDA_PIN 17
#define BSP_I2C_INTERNAL_SCL_PIN 18

// OLED
#define BSP_OLED_I2C_ADDRESS 0x3c
#define BSP_OLED_RESET       21

// LoRa radio module pins
#define BSP_LORA_CS    8
#define BSP_LORA_SCK   9
#define BSP_LORA_MOSI  10
#define BSP_LORA_MISO  11
#define BSP_LORA_RESET 12
#define BSP_LORA_BUSY  13
#define BSP_LORA_DIO1  14
#define BSP_LORA_BUS   SPI2_HOST
