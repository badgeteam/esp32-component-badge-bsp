#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "bsp/device.h"
#include "esp_err.h"
#include "esp_log.h"

static char const* TAG = "BSP device";

static char const device_name[]         = "ESP32-S31 Korvo 1";
static char const device_manufacturer[] = "Espressif";

esp_err_t bsp_device_get_name(char* output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_name, buffer_length);
    return ESP_OK;
}

esp_err_t bsp_device_get_manufacturer(char* output, uint8_t buffer_length) {
    if (output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strlcpy(output, device_manufacturer, buffer_length);
    return ESP_OK;
}
