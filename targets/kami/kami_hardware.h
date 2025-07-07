// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#pragma once

#include "hal/adc_types.h"

// ADC
#define BSP_ADC_PIN_VBATT     34
#define BSP_ADC_CHANNEL_VBATT ADC_CHANNEL_6
#define BSP_ADC_ATTEN_VBATT   ADC_ATTEN_DB_11
#define BSP_ADC_PIN_VUSB      35
#define BSP_ADC_CHANNEL_VUSB  ADC_CHANNEL_7
#define BSP_ADC_ATTEN_VUSB    ADC_ATTEN_DB_11

// I2C bus
#define BSP_I2C_BUS     0
#define BSP_I2C_SDA_PIN 26
#define BSP_I2C_SCL_PIN 27
#define BSP_I2C_SPEED   400000  // 400 kHz
#define BSP_I2C_TIMEOUT 250     // us

// SSD1619 epaper display
#define BSP_EPAPER_SPI_BUS   SPI2_HOST
#define BSP_EPAPER_DATA_PIN  5
#define BSP_EPAPER_CLK_PIN   18
#define BSP_EPAPER_CS_PIN    19
#define BSP_EPAPER_DCX_PIN   21
#define BSP_EPAPER_BUSY_PIN  22
#define BSP_EPAPER_RESET_PIN 23
#define BSP_EPAPER_SPEED     10000000
#define BSP_EPAPER_WIDTH     128
#define BSP_EPAPER_HEIGHT    296

// Addressable LEDs
#define BSP_LED_DATA_PIN 32
#define BSP_LED_NUM      6

// Expansion port
#define BSP_EXP_E0_PIN 39
#define BSP_EXP_E1_PIN 13
#define BSP_EXP_E2_PIN 12
#define BSP_EXP_E3_PIN 4
#define BSP_EXP_E4_PIN 36

// SAO port
#define BSP_SAO_IO1_PIN BSP_EXP_E0_PIN
#define BSP_SAO_IO2_PIN BSP_EXP_E1_PIN

// MPR121 touch controller
#define BSP_MPR121_I2C_BUS     BSP_I2C_BUS
#define BSP_MPR121_I2C_ADDRESS 0x5A
#define BSP_MPR121_INT_PIN     25

#define BSP_MPR121_PIN_BTN_A        0
#define BSP_MPR121_PIN_BTN_B        1
#define BSP_MPR121_PIN_BTN_START    2
#define BSP_MPR121_PIN_BTN_SELECT   3
#define BSP_MPR121_PIN_BTN_DOWN     4
#define BSP_MPR121_PIN_BTN_RIGHT    5
#define BSP_MPR121_PIN_BTN_UP       6
#define BSP_MPR121_PIN_BTN_LEFT     7
#define BSP_MPR121_PIN_BTN_MENU     8
#define BSP_MPR121_PIN_BTN_HOME     9
#define BSP_MPR121_PIN_INPUT_CHRG   10
#define BSP_MPR121_PIN_INPUT_SD_DET 11

// Power management
#define BSP_POWER_ENABLE_PIN 33