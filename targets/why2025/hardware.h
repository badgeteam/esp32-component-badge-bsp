// SPDX-License-Identifier: MIT

#pragma once

// I²C peripheral used for internal I²C bus.
#define BSP_I2CINT_NUM     0
// Internal I²C bus SDA pin.
#define BSP_I2CINT_SDA_PIN 9
// Internal I²C bus SCL pin.
#define BSP_I2CINT_SCL_PIN 10

// Interrupt pin from the CH32V203 to the ESP32P4.
#define BSP_CH32_IRQ_PIN 6
// I²C address for the CH32V203.
#define BSP_CH32_ADDR    0x5f
// Expected CH32 version.
#define BSP_CH32_VERSION 0x08

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

/* ==== Mipi DSI parameters ==== */
#define BSP_DSI_DPI_CLK_MHZ    30
#define BSP_DSI_LCD_H_RES      480
#define BSP_DSI_LCD_V_RES      800
#define BSP_DSI_LCD_HSYNC      40
#define BSP_DSI_LCD_HBP        40
#define BSP_DSI_LCD_HFP        30
#define BSP_DSI_LCD_VSYNC      16
#define BSP_DSI_LCD_VBP        16
#define BSP_DSI_LCD_VFP        16
#define BSP_DSI_LANES          2
#define BSP_DSI_BITRATE_MBPS   250
#define BSP_DSI_LDO_CHAN       3
#define BSP_DSI_LDO_VOLTAGE_MV 2500
#define BSP_LCD_RESET_PIN      0
