#include "bsp/catt.h"
#include "bsp/sao.h"
#include "esp_err.h"

/*

This is a generic implementation of the SAO API for devices which have a CATT port.

The CATT port offers both a PMOD and SAO port in one. This table translates between the PMOD and SAO pin naming.

| SAO pin | PMOD pin |
| 1. 3.3v | -        |
| 2. GND  | -        |
| 3. SDA  | 7. D4    |
| 4. SCL  | 1. D0    |
| 5. IO1  | 8. D5    |
| 6. IO2  | 2. D1    |

*/

esp_err_t __attribute__((weak)) bsp_sao_initialize(void) {
    return ESP_OK;
}

esp_err_t __attribute__((weak)) bsp_sao_i2c_bus_get_handle(i2c_master_bus_handle_t* out_handle) {
    return bsp_catt_i2c_bus_get_handle(out_handle);
}

gpio_num_t __attribute__((weak)) bsp_sao_get_gpio(sao_pin_num_t pin) {
    switch (pin) {
        case SAO_PIN_D0:
            return bsp_catt_get_gpio(CATT_PIN_D5);
        case SAO_PIN_D1:
            return bsp_catt_get_gpio(CATT_PIN_D1);
        case SAO_PIN_SDA:
            return bsp_catt_get_gpio(CATT_PIN_D4);
        case SAO_PIN_SCL:
            return bsp_catt_get_gpio(CATT_PIN_D0);
        default:
            return GPIO_NUM_NC;
    }
}
