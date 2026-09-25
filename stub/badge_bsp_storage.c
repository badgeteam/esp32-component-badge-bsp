#include "bsp/storage.h"
#include "esp_err.h"

esp_err_t __attribute__((weak)) bsp_storage_initialize(void) {
    return ESP_OK;
}

bool __attribute__((weak)) bsp_storage_get_supported(bsp_storage_type_t type) {
    (void)type;
    return false;
}

bsp_storage_status_t __attribute__((weak)) bsp_storage_get_status(bsp_storage_type_t type) {
    (void)type;
    return BSP_STORAGE_STATUS_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_storage_mount(bsp_storage_type_t type, const char* mountpoint) {
    (void)type;
    (void)mountpoint;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_storage_unmount(bsp_storage_type_t type) {
    (void)type;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_storage_get_mounted(bsp_storage_type_t type, bool* out_mounted) {
    (void)type;
    (void)out_mounted;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t __attribute__((weak)) bsp_storage_get_mountpoint(bsp_storage_type_t type, char* out_mountpoint,
                                                           size_t max_length) {
    (void)out_mountpoint;
    (void)max_length;
    return ESP_ERR_NOT_SUPPORTED;
}
