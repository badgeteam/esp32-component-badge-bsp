// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#pragma once

#include "hal/adc_types.h"

// ADC
#define BSP_ADC_PIN_VBATT     2
#define BSP_ADC_CHANNEL_VBATT ADC_CHANNEL_2
#define BSP_ADC_ATTEN_VBATT   ADC_ATTEN_DB_11

// I2C bus
#define BSP_I2C_BUS     0
#define BSP_I2C_SDA_PIN 6
#define BSP_I2C_SCL_PIN 7
#define BSP_I2C_SPEED   400000  // 400 kHz
#define BSP_I2C_TIMEOUT 250     // us

// SSD1619 epaper display
#define BSP_EPAPER_SPI_BUS   SPI2_HOST
#define BSP_EPAPER_DATA_PIN  19
#define BSP_EPAPER_CLK_PIN   21
#define BSP_EPAPER_CS_PIN    8
#define BSP_EPAPER_DCX_PIN   5
#define BSP_EPAPER_BUSY_PIN  10
#define BSP_EPAPER_RESET_PIN 16
#define BSP_EPAPER_SPEED     10000000
#define BSP_EPAPER_WIDTH     128
#define BSP_EPAPER_HEIGHT    296

// Addressable LEDs
#define BSP_LED_DATA_PIN 22
#define BSP_LED_NUM      1

// SAO port
#define BSP_SAO_IO1_PIN 0
#define BSP_SAO_IO2_PIN 1
