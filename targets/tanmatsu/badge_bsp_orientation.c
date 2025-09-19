// SPDX-FileCopyrightText: 2025 Nicolai Electronics
// SPDX-License-Identifier: MIT

#include <math.h>
#include <stdint.h>
#include "bmi270.h"
#include "bmi270_interface.h"
#include "bsp/i2c.h"
#include "bsp/orientation.h"
#include "esp_err.h"
#include "esp_log.h"

#define GRAVITY_EARTH (9.80665f)

static const char* TAG = "BSP: orientation";

static i2c_master_bus_handle_t i2c_handle;
static SemaphoreHandle_t       i2c_semaphore;
static struct bmi2_dev         bmi         = {0};
static struct bmi2_sens_data   sensor_data = {0};

static float lsb_to_dps(int16_t val, float dps, uint8_t bit_width) {
    double power      = 2;
    float  half_scale = (float)((pow((double)power, (double)bit_width) / 2.0f));
    return (dps / (half_scale)) * (val);
}

static float lsb_to_mps2(int16_t val, float g_range, uint8_t bit_width) {
    double power      = 2;
    float  half_scale = (float)((pow((double)power, (double)bit_width) / 2.0f));
    return (GRAVITY_EARTH * val * g_range) / half_scale;
}

esp_err_t bsp_orientation_initialize(void) {
    // I2C bus configuration
    bsp_i2c_primary_bus_get_handle(&i2c_handle);
    bsp_i2c_primary_bus_get_semaphore(&i2c_semaphore);
    bmi2_set_i2c_configuration(i2c_handle, 0x68, i2c_semaphore);

    // Library interface initialization
    int8_t rslt = bmi2_interface_init(&bmi, BMI2_I2C_INTF);
    bmi2_error_codes_print_result(rslt);
    if (rslt != BMI2_OK) {
        ESP_LOGE(TAG, "Failed to initialize interface");
        return ESP_FAIL;
    }

    // Chip initialization
    rslt = bmi270_init(&bmi);
    bmi2_error_codes_print_result(rslt);
    if (rslt != BMI2_OK) {
        ESP_LOGE(TAG, "Failed to initialize chip\r\n");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t bsp_orientation_enable_gyroscope(void) {
    int8_t rslt;

    struct bmi2_sens_config config;
    config.type = BMI2_GYRO;

    rslt = bmi2_get_sensor_config(&config, 1, &bmi);
    if (rslt != BMI2_OK) {
        bmi2_error_codes_print_result(rslt);
        return ESP_FAIL;
    }

    rslt = bmi2_map_data_int(BMI2_DRDY_INT, BMI2_INT2, &bmi);
    if (rslt != BMI2_OK) {
        bmi2_error_codes_print_result(rslt);
        return ESP_FAIL;
    }

    if (rslt == BMI2_OK) {
        config.cfg.gyr.odr         = BMI2_GYR_ODR_100HZ;
        config.cfg.gyr.range       = BMI2_GYR_RANGE_2000;
        config.cfg.gyr.bwp         = BMI2_GYR_NORMAL_MODE;
        config.cfg.gyr.noise_perf  = BMI2_POWER_OPT_MODE;
        config.cfg.gyr.filter_perf = BMI2_PERF_OPT_MODE;
        rslt                       = bmi2_set_sensor_config(&config, 1, &bmi);
    }

    uint8_t sensor_list[] = {BMI2_GYRO};
    rslt                  = bmi2_sensor_enable(sensor_list, sizeof(sensor_list), &bmi);
    if (rslt != BMI2_OK) {
        bmi2_error_codes_print_result(rslt);
        return ESP_FAIL;
    }

    return rslt;
}

esp_err_t bsp_orientation_disable_gyroscope(void) {
    int8_t rslt;

    uint8_t sensor_list[] = {BMI2_GYRO};
    rslt                  = bmi2_sensor_disable(sensor_list, sizeof(sensor_list), &bmi);
    if (rslt != BMI2_OK) {
        bmi2_error_codes_print_result(rslt);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t bsp_orientation_enable_accelerometer(void) {
    int8_t rslt;

    struct bmi2_sens_config config;
    config.type = BMI2_ACCEL;

    rslt = bmi2_get_sensor_config(&config, 1, &bmi);
    if (rslt != BMI2_OK) {
        bmi2_error_codes_print_result(rslt);
        return ESP_FAIL;
    }

    config.cfg.acc.odr         = BMI2_ACC_ODR_200HZ;
    config.cfg.acc.range       = BMI2_ACC_RANGE_2G;
    config.cfg.acc.bwp         = BMI2_ACC_NORMAL_AVG4;
    config.cfg.acc.filter_perf = BMI2_PERF_OPT_MODE;

    rslt = bmi2_set_sensor_config(&config, 1, &bmi);
    if (rslt != BMI2_OK) {
        bmi2_error_codes_print_result(rslt);
        return ESP_FAIL;
    }

    rslt = bmi2_map_data_int(BMI2_DRDY_INT, BMI2_INT1, &bmi);
    if (rslt != BMI2_OK) {
        bmi2_error_codes_print_result(rslt);
        return ESP_FAIL;
    }

    uint8_t sensor_list[] = {BMI2_ACCEL};
    rslt                  = bmi2_sensor_enable(sensor_list, sizeof(sensor_list), &bmi);
    if (rslt != BMI2_OK) {
        bmi2_error_codes_print_result(rslt);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t bsp_orientation_disable_accelerometer(void) {
    int8_t rslt;

    uint8_t sensor_list[] = {BMI2_ACCEL};
    rslt                  = bmi2_sensor_disable(sensor_list, sizeof(sensor_list), &bmi);
    if (rslt != BMI2_OK) {
        bmi2_error_codes_print_result(rslt);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t bsp_orientation_get(bool* out_gyro_ready, bool* out_accel_ready, float* out_gyro_x, float* out_gyro_y,
                              float* out_gyro_z, float* out_accel_x, float* out_accel_y, float* out_accel_z) {
    int8_t rslt = bmi2_get_sensor_data(&sensor_data, &bmi);
    if (rslt != BMI2_OK) {
        bmi2_error_codes_print_result(rslt);
        return ESP_FAIL;
    }

    if (out_gyro_ready) {
        *out_gyro_ready = (sensor_data.status & BMI2_DRDY_GYR) ? true : false;
    }

    if (out_accel_ready) {
        *out_accel_ready = (sensor_data.status & BMI2_DRDY_ACC) ? true : false;
    }

    if (sensor_data.status & BMI2_DRDY_GYR) {
        /* Converting lsb to degree per second for 16 bit gyro at 2000dps range. */
        float gyro_x = lsb_to_dps(sensor_data.gyr.x, (float)2000, bmi.resolution);
        float gyro_y = lsb_to_dps(sensor_data.gyr.y, (float)2000, bmi.resolution);
        float gyro_z = lsb_to_dps(sensor_data.gyr.z, (float)2000, bmi.resolution);

        if (out_gyro_x) {
            *out_gyro_x = gyro_x;
        }
        if (out_gyro_y) {
            *out_gyro_y = gyro_y;
        }
        if (out_gyro_z) {
            *out_gyro_z = gyro_z;
        }
    }

    if (sensor_data.status & BMI2_DRDY_ACC) {
        /* Converting lsb to meter per second squared for 16 bit accelerometer at 2G range. */
        float accel_x = lsb_to_mps2(sensor_data.acc.x, (float)2, bmi.resolution);
        float accel_y = lsb_to_mps2(sensor_data.acc.y, (float)2, bmi.resolution);
        float accel_z = lsb_to_mps2(sensor_data.acc.z, (float)2, bmi.resolution);

        if (out_accel_x) {
            *out_accel_x = accel_x;
        }
        if (out_accel_y) {
            *out_accel_y = accel_y;
        }
        if (out_accel_z) {
            *out_accel_z = accel_z;
        }
    }

    return ESP_OK;
}
