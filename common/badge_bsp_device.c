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
esp_err_t bsp_display_initialize(void);
esp_err_t bsp_i2c_primary_bus_initialize(void);
esp_err_t bsp_input_initialize(void);
esp_err_t bsp_led_initialize(void);
esp_err_t bsp_power_initialize(void);
esp_err_t bsp_rtc_initialize(void);

esp_err_t bsp_device_initialize(void) {
    // Install the ISR service for GPIO interrupts
    gpio_install_isr_service(0);

    // Initialize the primary I2C bus
    ESP_LOGI(TAG, "Initializing primary I2C bus...");
    BSP_RETURN_ON_FAILURE(bsp_i2c_primary_bus_initialize(), ESP_LOGE(TAG, "Failed to initialize primary I2C bus"));

    // Initialize the display
    ESP_LOGI(TAG, "Initializing display...");
    BSP_RETURN_ON_FAILURE(bsp_display_initialize(), ESP_LOGE(TAG, "Failed to initialize display"));

    // Initialize the input framework
    ESP_LOGI(TAG, "Initializing input framework...");
    BSP_RETURN_ON_FAILURE(bsp_input_initialize(), ESP_LOGE(TAG, "Failed to initialize input framework"));

    // Initialize device specific hardware
    ESP_LOGI(TAG, "Initializing device specific hardware...");
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
