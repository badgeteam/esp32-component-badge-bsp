#include "bsp/usb_device.h"
#include "esp_err.h"

esp_err_t __attribute__((weak)) bsp_usb_device_mode_set(bsp_usb_mode_t mode) {
    return ESP_ERR_NOT_SUPPORTED;
}

bsp_usb_mode_t __attribute__((weak)) bsp_usb_device_mode_get(void) {
    return BSP_USB_MODE_DISABLED;
}
