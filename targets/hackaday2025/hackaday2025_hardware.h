// SPDX-FileCopyrightText: 2026 Nicolai Electronics
// SPDX-License-Identifier: MIT

#pragma once

// Buttons
#define BSP_GPIO_BTN 0

// LCD
#define BSP_LCD_BACKLIGHT 2
#define BSP_LCD_DATA      21
#define BSP_LCD_SCK       38
#define BSP_LCD_DC        39
#define BSP_LCD_RESET     40
#define BSP_LCD_CS        41
#define BSP_LCD_TE        42

// SAO
#define BSP_SAO_SDA   4
#define BSP_SAO_SCL   5
#define BSP_SAO_IO2   6
#define BSP_SAO_IO1   7
#define BSP_GPIO_EXP0 11
#define BSP_GPIO_EXP1 12

// LEDs
#define BSP_GPIO_IR_LED 1

// Keyboard
#define BSP_KBD_INT 13
#define BSP_KBD_SCL 14
#define BSP_KBD_SDA 47
#define BSP_KBD_RST 48

#define BSP_KBD_BUS 0

// LoRa radio module pins
#define BSP_LORA_CS    17
#define BSP_LORA_SCK   8
#define BSP_LORA_MOSI  3
#define BSP_LORA_MISO  9
#define BSP_LORA_RESET 18
#define BSP_LORA_BUSY  15
#define BSP_LORA_DIO1  16
#define BSP_LORA_RFSW  10
#define BSP_LORA_BUS   SPI2_HOST
