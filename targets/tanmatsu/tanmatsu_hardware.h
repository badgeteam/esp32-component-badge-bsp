
// SPDX-License-Identifier: MIT

// Private include file, not intended to be included by end users

#pragma once

// GPIO buttons
#define BSP_GPIO_BTN_VOLUME_DOWN 35

// Internal I2C bus
#define BSP_I2C_INTERNAL_BUS     0
#define BSP_I2C_INTERNAL_SDA_PIN 9
#define BSP_I2C_INTERNAL_SCL_PIN 10

// Coprocessor
#if defined(OLD_PROTOTYPE)
#define BSP_COPROCESSOR_INTERRUPT_PIN 6
#else
#define BSP_COPROCESSOR_INTERRUPT_PIN 1
#endif
#define BSP_COPROCESSOR_I2C_ADDRESS 0x5f

// ESP32-C6 radio
#define BSP_SDIO_CLK   17
#define BSP_SDIO_CMD   16
#define BSP_SDIO_D0    18
#define BSP_SDIO_D1    19
#define BSP_SDIO_D2    20
#define BSP_SDIO_D3    21
#define BSP_SDIO_D4    -1
#define BSP_SDIO_D5    -1
#define BSP_SDIO_D6    -1
#define BSP_SDIO_D7    -1
#define BSP_SDIO_CD    -1
#define BSP_SDIO_WP    -1
#define BSP_SDIO_WIDTH 4
#define BSP_SDIO_FLAGS 0

// SD card slot
#define BSP_SDCARD_CLK   43
#define BSP_SDCARD_CMD   44
#define BSP_SDCARD_D0    39
#define BSP_SDCARD_D1    40
#define BSP_SDCARD_D2    41
#define BSP_SDCARD_D3    42
#define BSP_SDCARD_D4    -1
#define BSP_SDCARD_D5    -1
#define BSP_SDCARD_D6    -1
#define BSP_SDCARD_D7    -1
#define BSP_SDCARD_CD    -1
#define BSP_SDCARD_WP    -1
#define BSP_SDCARD_WIDTH 4
#define BSP_SDCARD_FLAGS 0

// MIPI DSI display
#define BSP_DSI_LDO_CHAN       3
#define BSP_DSI_LDO_VOLTAGE_MV 2500
#define BSP_LCD_RESET_PIN      14

// BMI270 IMU
#define BSP_BMI270_I2C_ADDRESS 0x68

// ES8156 audio codec
#define BSP_ES8156_I2C_ADDRESS 0x08
