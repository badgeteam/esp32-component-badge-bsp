// SPDX-FileCopyrightText: 2024 Orange-Murker
// SPDX-License-Identifier: MIT

#pragma once

// SPI
#define BSP_SPI_MOSI 23
#define BSP_SPI_MISO 35
#define BSP_SPI_SCLK 18

// LCD
#define BSP_LCD_RESET_PIN 25
#define BSP_LCD_MODE_PIN  26
#define BSP_LCD_CS_PIN    32
#define BSP_LCD_DC_PIN    33

// I2C
#define BSP_I2C_BUS     0
#define BSP_I2C_SDA_PIN 22
#define BSP_I2C_SCL_PIN 21

// RP2040
#define BSP_RP2040_I2C_ADDR 0x17
#define BSP_RP2040_INTR_PIN 34

// SD card (1-bit mode, D1/D2/D3 are repurposed as I2S pins)
#define BSP_SDCARD_CLK_PIN 14
#define BSP_SDCARD_CMD_PIN 15
#define BSP_SDCARD_D0_PIN  2

// Shared power switch for the SD card and the addressable LEDs
#define BSP_SDCARD_LED_POWER_PIN 19

// Addressable LEDs
#define BSP_LED_DATA_PIN 5
#define BSP_LED_NUM      5

// ES8156 audio codec
#define BSP_I2S_MCLK 0
#define BSP_I2S_BCLK 4
#define BSP_I2S_WS   12
#define BSP_I2S_DOUT 13
