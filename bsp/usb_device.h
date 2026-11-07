#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BSP_USB_MODE_DEBUG    = 0,
    BSP_USB_MODE_DEVICE   = 1,
    BSP_USB_MODE_DISABLED = 2,
} bsp_usb_mode_t;

/// @brief Switch the on-board USB PHY between debug (serial/JTAG) and device mode
/// @details Takes the peripheral off the bus, selects which controller the PHY is
///          routed to and, unless disabling, puts the peripheral back onto the bus
/// @return ESP-IDF error code
///          - ESP_OK if the USB mode was switched
///          - ESP_ERR_NOT_SUPPORTED if this board has no USB mode to switch
esp_err_t bsp_usb_device_mode_set(bsp_usb_mode_t mode);

/// @brief Get the USB mode last set with bsp_usb_device_mode_set
bsp_usb_mode_t bsp_usb_device_mode_get(void);

#ifdef __cplusplus
}
#endif
