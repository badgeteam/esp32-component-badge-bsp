#include <stdbool.h>
#include <stdint.h>
#include "bsp/power.h"
#include "esp_err.h"

esp_err_t bsp_power_get_radio_state(bsp_radio_state_t* out_state) {
    if (out_state) {
        *out_state = BSP_POWER_RADIO_STATE_APPLICATION;
    }
    return ESP_OK;
}
