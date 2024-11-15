#include "driver/gpio.h"
#include "dsi_panel_nicolaielectronics_st7701.h"
#include "esp_err.h"
#include "esp_ldo_regulator.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "hardware.h"

static char const TAG[] = "display";

static void enable_dsi_phy_power(void) {
    // Turn on the power for MIPI DSI PHY, so it can go from "No Power" state to "Shutdown" state
    esp_ldo_channel_handle_t ldo_mipi_phy        = NULL;
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id    = BSP_DSI_LDO_CHAN,
        .voltage_mv = BSP_DSI_LDO_VOLTAGE_MV,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy));
    ESP_LOGI(TAG, "MIPI DSI PHY Powered on");
}

void init_display() {
    enable_dsi_phy_power();

    gpio_config_t pmod_conf = {
        .pin_bit_mask = BIT64(BSP_LCD_RESET_PIN),
        .mode         = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en   = 0,
        .pull_down_en = 0,
        .intr_type    = GPIO_INTR_DISABLE,
    };

    gpio_config(&pmod_conf);

    gpio_set_level(BSP_LCD_RESET_PIN, false);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(BSP_LCD_RESET_PIN, true);
    vTaskDelay(pdMS_TO_TICKS(100));

    st7701_initialize(-1);
}
