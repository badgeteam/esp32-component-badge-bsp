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

static char const TAG[] = "BSP: device";

// Internal BSP functions to initialize the subsystems
esp_err_t bsp_device_initialize_custom(void);
esp_err_t bsp_audio_initialize(void);
esp_err_t bsp_display_initialize(const bsp_display_configuration_t* configuration);
esp_err_t bsp_i2c_primary_bus_initialize(void);
esp_err_t bsp_input_initialize(void);
esp_err_t bsp_led_initialize(void);
esp_err_t bsp_power_initialize(void);
esp_err_t bsp_rtc_initialize(void);
esp_err_t bsp_orientation_initialize(void);
esp_err_t bsp_sensor_initialize(void);
esp_err_t bsp_input_hooks_initialize(void);
esp_err_t bsp_catt_initialize(void);
esp_err_t bsp_sao_initialize(void);

esp_err_t bsp_device_initialize(const bsp_configuration_t* configuration) {
    // Install the ISR service for GPIO interrupts
    gpio_install_isr_service(0);

    esp_err_t return_value = ESP_OK;

    // Initialize the primary I2C bus
    esp_err_t res = bsp_i2c_primary_bus_initialize();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize primary I2C bus");
        return res;  // Fatal error
    }

    // Initialize device specific hardware
    res = bsp_device_initialize_custom();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize device specific hardware");
        return res;  // Fatal error
    }

    // Initialize the display
    res = bsp_display_initialize(configuration != NULL ? &configuration->display : NULL);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize display");
        return_value = res;
    }

    // Initialize the input framework
    res = bsp_input_initialize();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize input framework");
        return_value = res;
    } else {
        // Initialize input hooks
        bsp_input_hooks_initialize();
    }

    // Initialize power
    res = bsp_power_initialize();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize power subsystem");
        return_value = res;
    }

    // Initialize the RTC
    res = bsp_rtc_initialize();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize RTC subsystem");
        return_value = res;
    }

    // Initialize audio
    res = bsp_audio_initialize();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize audio subsystem");
        return_value = res;
    }

    // Initialize LEDs
    res = bsp_led_initialize();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LED subsystem");
        return_value = res;
    }

    // Initialize orientation sensor
    res = bsp_orientation_initialize();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize orientation sensor");
        return_value = res;
    }

    // Initialize sensors
    res = bsp_sensor_initialize();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize sensors");
        return_value = res;
    }

    // Initialize CATT expansion port
    res = bsp_catt_initialize();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize CATT port");
        return_value = res;
    }

    // Initialize SAO expansion port
    res = bsp_sao_initialize();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SAO port");
        return_value = res;
    }

    return return_value;
}
