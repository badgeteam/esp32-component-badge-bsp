// Board support package API: Tanmatsu implementation
// SPDX-FileCopyrightText: 2026 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/usb_device.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/usb_serial_jtag_ll.h"

static bsp_usb_mode_t current_mode = BSP_USB_MODE_DEBUG;

esp_err_t bsp_usb_device_mode_set(bsp_usb_mode_t mode) {
    const usb_serial_jtag_pull_override_vals_t override_disable_usb = {
        .dm_pd = true, .dm_pu = false, .dp_pd = true, .dp_pu = false};
    const usb_serial_jtag_pull_override_vals_t override_enable_usb = {
        .dm_pd = false, .dm_pu = false, .dp_pd = false, .dp_pu = true};

    // Drop off the bus by removing the pull-up on USB DP
    usb_serial_jtag_ll_phy_enable_pull_override(&override_disable_usb);

    // Select USB mode by swapping and un-swapping the two PHYs
    switch (mode) {
        case BSP_USB_MODE_DEVICE:
            vTaskDelay(pdMS_TO_TICKS(500));  // Wait for disconnect before switching to device
            usb_serial_jtag_ll_phy_select(1);
            break;
        case BSP_USB_MODE_DEBUG:
        case BSP_USB_MODE_DISABLED:
        default:
            usb_serial_jtag_ll_phy_select(0);
            vTaskDelay(pdMS_TO_TICKS(500));  // Wait for disconnect after switching to debug
            break;
    }

    if (mode != BSP_USB_MODE_DISABLED) {
        // Put the device back onto the bus by re-enabling the pull-up on USB DP
        usb_serial_jtag_ll_phy_enable_pull_override(&override_enable_usb);
        usb_serial_jtag_ll_phy_disable_pull_override();
    }

    current_mode = mode;

    return ESP_OK;
}

bsp_usb_mode_t bsp_usb_device_mode_get(void) {
    return current_mode;
}
