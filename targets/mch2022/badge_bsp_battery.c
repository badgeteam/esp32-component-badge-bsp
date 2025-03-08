#include "bsp/mch2022.h"
#include "esp_check.h"
#include "esp_err.h"
#include "rp2040.h"

static char const* TAG = "BSP Battery";

esp_err_t bsp_battery_get_voltage(uint16_t* bat_mv) {
    if (bat_mv == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    RP2040 handle;
    ESP_RETURN_ON_ERROR(bsp_mch2022_coprocessor_get_handle(&handle), TAG, "Failed to get the coprocessor handle");

    uint16_t vbat_raw;
    ESP_RETURN_ON_ERROR(rp2040_read_vbat_raw(&handle, &vbat_raw), TAG, "Failed to get the battery voltage");

    // 12-bit ADC with 3.3v vref
    // Connected through 100k/100k divider
    uint32_t vbat_mv = (((uint32_t)vbat_raw) * 2 * 3300) / (1 << 12);

    *bat_mv = (uint16_t)vbat_mv;

    return ESP_OK;
}
