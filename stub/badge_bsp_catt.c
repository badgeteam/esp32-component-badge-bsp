#include "bsp/catt.h"
#include "bsp/sao.h"
#include "esp_err.h"

esp_err_t __attribute__((weak)) bsp_catt_initialize(void) {
    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_catt_set_i2c_enabled(bool enable) {
    (void)enable;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_catt_get_i2c_enabled(bool* out_enabled) {
    if (out_enabled) {
        *out_enabled = false;
    }
    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_catt_i2c_bus_get_handle(i2c_master_bus_handle_t* out_handle) {
    (void)out_handle;
    return ESP_ERR_NOT_SUPPORTED;
}

gpio_num_t __attribute__((weak)) bsp_catt_get_gpio(catt_pin_num_t pin) {
    return GPIO_NUM_NC;
}
