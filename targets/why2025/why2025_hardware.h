
// SPDX-License-Identifier: MIT

// Private include file, not intended to be included by end users

#pragma once

// Internal I2C bus
#define BSP_I2C_BUS     0
#define BSP_I2C_SDA_PIN 18
#define BSP_I2C_SCL_PIN 20


#define BSP_KBD_INT       2
#define BSP_KBD_SCL       BSP_I2C_SCL_PIN
#define BSP_KBD_SDA       BSP_I2C_SDA_PIN
#define BSP_KBD_RST       -1


// MIPI DSI display
#define BSP_DSI_LDO_CHAN       3
#define BSP_DSI_LDO_VOLTAGE_MV 2500
#define BSP_LCD_RESET_PIN      17  // Note: low for normal operation, high for reset
