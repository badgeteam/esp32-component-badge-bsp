// Board support package API: Hackaday 2025 implementation
// SPDX-FileCopyrightText: 2026 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include "bsp/usb_device.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/usb_serial_jtag_ll.h"
#include "hal/usb_wrap_ll.h"

static bsp_usb_mode_t current_mode = BSP_USB_MODE_DEBUG;

esp_err_t bsp_usb_device_mode_set(bsp_usb_mode_t mode) {
    const usb_serial_jtag_pull_override_vals_t debug_override_disable_usb = {
        .dm_pd = true, .dm_pu = false, .dp_pd = true, .dp_pu = false};
    const usb_serial_jtag_pull_override_vals_t debug_override_enable_usb = {
        .dm_pd = false, .dm_pu = false, .dp_pd = false, .dp_pu = true};

    const usb_wrap_pull_override_vals_t device_override_disable_usb = {
        .dp_pu = false, .dp_pd = true, .dm_pu = false, .dm_pd = true};
    const usb_wrap_pull_override_vals_t device_override_enable_usb = {
        .dp_pu = true, .dp_pd = false, .dm_pu = false, .dm_pd = false};

    usb_wrap_ll_phy_enable_pull_override(&USB_WRAP, &device_override_disable_usb);
    usb_serial_jtag_ll_phy_enable_pull_override(&debug_override_disable_usb);

    // Select USB mode by muxing the internal PHY to the appropriate controller
    switch (mode) {
        case BSP_USB_MODE_DEVICE:
            vTaskDelay(pdMS_TO_TICKS(500));                     // Wait for disconnect before switching to device
            usb_wrap_ll_phy_enable_external(&USB_WRAP, false);  // Internal PHY -> USB-OTG
            usb_wrap_ll_phy_enable_pad(&USB_WRAP, true);
            break;
        case BSP_USB_MODE_DEBUG:
        case BSP_USB_MODE_DISABLED:
        default:
            usb_serial_jtag_ll_phy_enable_external(false);  // Internal PHY -> USB-Serial-JTAG
            usb_serial_jtag_ll_phy_enable_pad(true);
            vTaskDelay(pdMS_TO_TICKS(500));  // Wait for disconnect after switching to debug
            break;
    }

    if (mode != BSP_USB_MODE_DISABLED) {
        // Put the device back onto the bus by re-enabling the pull-up on USB DP of the
        // now-active controller
        if (mode == BSP_USB_MODE_DEVICE) {
            usb_wrap_ll_phy_enable_pull_override(&USB_WRAP, &device_override_enable_usb);
            usb_wrap_ll_phy_disable_pull_override(&USB_WRAP);
        } else {
            usb_serial_jtag_ll_phy_enable_pull_override(&debug_override_enable_usb);
            usb_serial_jtag_ll_phy_disable_pull_override();
        }
    }

    current_mode = mode;

    return ESP_OK;
}

bsp_usb_mode_t bsp_usb_device_mode_get(void) {
    return current_mode;
}
