#include "bsp/catt.h"
#include "bsp/sao.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "tanmatsu_hardware.h"

static const char TAG[] = "CATT";

static const i2c_master_bus_config_t i2c_master_config_catt = {
    .clk_source                   = I2C_CLK_SRC_DEFAULT,
    .i2c_port                     = BSP_I2C_CATT_BUS,
    .scl_io_num                   = BSP_I2C_CATT_SCL_PIN,
    .sda_io_num                   = BSP_I2C_CATT_SDA_PIN,
    .glitch_ignore_cnt            = 7,
    .flags.enable_internal_pullup = true,
};

static i2c_master_bus_handle_t i2c_bus_handle_catt = NULL;

static bool bsp_catt_test(void) {
    // Pull-up on I2C bus pins
    gpio_set_level(BSP_I2C_CATT_SCL_PIN, 1);
    gpio_set_level(BSP_I2C_CATT_SDA_PIN, 1);
    gpio_config_t gpio_cfg = {
        .pin_bit_mask = BIT64(BSP_I2C_CATT_SCL_PIN) | BIT64(BSP_I2C_CATT_SDA_PIN),
        .mode         = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&gpio_cfg);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Test that pins become high
    if (!gpio_get_level(BSP_I2C_CATT_SCL_PIN)) {
        ESP_LOGW(TAG, "Attached add-on is forcing SCL low, I2C bus unavailable");
        return false;
    }

    if (!gpio_get_level(BSP_I2C_CATT_SDA_PIN)) {
        ESP_LOGW(TAG, "Attached add-on is forcing SDA low, I2C bus unavailable");
        return false;
    }

    // Pull-down on the I2C bus pins
    gpio_set_drive_capability(BSP_I2C_CATT_SCL_PIN, GPIO_DRIVE_CAP_0);
    gpio_set_drive_capability(BSP_I2C_CATT_SDA_PIN, GPIO_DRIVE_CAP_0);
    gpio_set_level(BSP_I2C_CATT_SCL_PIN, 0);
    gpio_set_level(BSP_I2C_CATT_SDA_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
    bool scl = gpio_get_level(BSP_I2C_CATT_SCL_PIN);
    bool sda = gpio_get_level(BSP_I2C_CATT_SDA_PIN);
    gpio_set_level(BSP_I2C_CATT_SCL_PIN, 1);
    gpio_set_level(BSP_I2C_CATT_SDA_PIN, 1);
    gpio_set_drive_capability(BSP_I2C_CATT_SCL_PIN, GPIO_DRIVE_CAP_DEFAULT);
    gpio_set_drive_capability(BSP_I2C_CATT_SDA_PIN, GPIO_DRIVE_CAP_DEFAULT);

    // Test that pins become low
    if (scl) {
        ESP_LOGW(TAG, "Attached add-on is forcing SCL high, I2C bus unavailable");
        return false;
    }

    if (sda) {
        ESP_LOGW(TAG, "Attached add-on is forcing SDA high, I2C bus unavailable");
        return false;
    }

    return true;
}

esp_err_t bsp_catt_initialize(void) {
    return bsp_catt_set_i2c_enabled(true);
}

esp_err_t bsp_catt_set_i2c_enabled(bool enable) {
    if (enable) {
        if (i2c_bus_handle_catt != NULL) {
            return ESP_OK;  // Already enabled
        }

        if (!bsp_catt_test()) {
            gpio_config_t gpio_cfg = {
                .pin_bit_mask = BIT64(BSP_I2C_CATT_SCL_PIN) | BIT64(BSP_I2C_CATT_SDA_PIN),
                .mode         = GPIO_MODE_INPUT,
                .pull_up_en   = GPIO_PULLUP_DISABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type    = GPIO_INTR_DISABLE,
            };
            gpio_config(&gpio_cfg);
            return ESP_ERR_INVALID_STATE;
        }

        return i2c_new_master_bus(&i2c_master_config_catt, &i2c_bus_handle_catt);
    } else {
        if (i2c_bus_handle_catt == NULL) {
            return ESP_OK;  // Already disabled
        }
        esp_err_t res = i2c_del_master_bus(i2c_bus_handle_catt);
        if (res == ESP_OK) {
            i2c_bus_handle_catt = NULL;

            // Set former I2C pins to input with pull-up
            gpio_config_t gpio_cfg = {
                .pin_bit_mask = BIT64(BSP_I2C_CATT_SCL_PIN) | BIT64(BSP_I2C_CATT_SDA_PIN),
                .mode         = GPIO_MODE_INPUT,
                .pull_up_en   = GPIO_PULLUP_ENABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type    = GPIO_INTR_DISABLE,
            };
            gpio_config(&gpio_cfg);
        }
        return res;
    }

    return ESP_OK;
}

esp_err_t bsp_catt_get_i2c_enabled(bool* out_enabled) {
    if (out_enabled) {
        *out_enabled = (i2c_bus_handle_catt != NULL);
    }
    return ESP_OK;
}

esp_err_t bsp_catt_i2c_bus_get_handle(i2c_master_bus_handle_t* out_handle) {
    if (out_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *out_handle = i2c_bus_handle_catt;
    return (i2c_bus_handle_catt == NULL) ? ESP_ERR_INVALID_STATE : ESP_OK;
}

gpio_num_t bsp_catt_get_gpio(catt_pin_num_t pin) {
    switch (pin) {
        case CATT_PIN_D0:
            return BSP_CATT_D0_PIN;
        case CATT_PIN_D1:
            return BSP_CATT_D1_PIN;
        case CATT_PIN_D2:
            return BSP_CATT_D2_PIN;
        case CATT_PIN_D3:
            return BSP_CATT_D3_PIN;
        case CATT_PIN_D4:
            return BSP_CATT_D4_PIN;
        case CATT_PIN_D5:
            return BSP_CATT_D5_PIN;
        case CATT_PIN_D6:
            return BSP_CATT_D6_PIN;
        case CATT_PIN_D7:
            return BSP_CATT_D7_PIN;
        default:
            return GPIO_NUM_NC;
    }
}
