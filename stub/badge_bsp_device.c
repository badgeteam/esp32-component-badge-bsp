// Board support package API: Generic stub implementation
// SPDX-FileCopyrightText: 2024 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "bootloader_common.h"
#include "bsp/audio.h"
#include "bsp/device.h"
#include "bsp/display.h"
#include "bsp/i2c.h"
#include "bsp/input.h"
#include "bsp/macro.h"
#include "bsp/power.h"
#include "bsp/rtc.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"

static char const TAG[] = "BSP: device";

// Internal BSP functions to initialize the subsystems
esp_err_t bsp_device_initialize_custom(void);
esp_err_t bsp_audio_initialize(void);
esp_err_t bsp_display_initialize(void);
esp_err_t bsp_i2c_primary_bus_initialize(void);
esp_err_t bsp_input_initialize(void);
esp_err_t bsp_led_initialize(void);
esp_err_t bsp_power_initialize(void);
esp_err_t bsp_rtc_initialize(void);

static char const device_name[]         = "Generic board";
static char const device_manufacturer[] = "Unknown";

esp_err_t bsp_device_initialize(void) {
    // Install the ISR service for GPIO interrupts
    gpio_install_isr_service(0);

    // Initialize the primary I2C bus
    BSP_RETURN_ON_FAILURE(bsp_i2c_primary_bus_initialize(), ESP_LOGE(TAG, "Failed to initialize primary I2C bus"));

    // Initialize the display
    BSP_RETURN_ON_FAILURE(bsp_display_initialize(), ESP_LOGE(TAG, "Failed to initialize display"));

    // Initialize the input framework
    BSP_RETURN_ON_FAILURE(bsp_input_initialize(), ESP_LOGE(TAG, "Failed to initialize input framework"));

    // Initialize device specific hardware
    BSP_RETURN_ON_FAILURE(bsp_device_initialize_custom(),
                          ESP_LOGE(TAG, "Failed to initialize device specific hardware"));

    // Initialize power
    BSP_RETURN_ON_FAILURE(bsp_power_initialize(), ESP_LOGE(TAG, "Failed to initialize power subsystem"));

    // Initialize the RTC
    BSP_RETURN_ON_FAILURE(bsp_rtc_initialize(), ESP_LOGE(TAG, "Failed to initialize RTC subsystem"));

    // Initialize audio
    BSP_RETURN_ON_FAILURE(bsp_audio_initialize(), ESP_LOGE(TAG, "Failed to initialize audio subsystem"));

    // Initialize LEDs
    BSP_RETURN_ON_FAILURE(bsp_led_initialize(), ESP_LOGE(TAG, "Failed to initialize LED subsystem"));

    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_device_initialize_custom(void) {
    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_device_get_name(char* output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_name, buffer_length);
    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_device_get_manufacturer(char* output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_manufacturer, buffer_length);
    return ESP_OK;
}

bool __attribute__((weak)) bsp_device_get_initialized_without_coprocessor(void) {
    return false;
}

void __attribute__((weak)) bsp_device_restart_to_launcher(void) {
    // This function is common to all supported devices, but it can still be overridden if needed
    rtc_retain_mem_t* mem = bootloader_common_get_rtc_retain_mem();

    // Remove the magic value set by the launcher to invalidated appfs bootloader struct
    memset(mem->custom, 0, sizeof(uint64_t));

    // Restart the device
    esp_restart();
}
