#pragma once

#include <stdbool.h>
#include "esp_err.h"

typedef enum {
    BSP_STORAGE_TYPE_INTERNAL = 0,
    BSP_STORAGE_TYPE_SDCARD   = 1,
} bsp_storage_type_t;

typedef enum {
    BSP_STORAGE_STATUS_NOT_SUPPORTED = 0,  // Not supported by the hardware platform
    BSP_STORAGE_STATUS_NOT_PRESENT   = 1,  // Card not inserted or partition not available
    BSP_STORAGE_STATUS_NOT_MOUNTED   = 2,  // Card present or partition available, but not mounted
    BSP_STORAGE_STATUS_MOUNTED       = 3,  // Card or partition mounted
    BSP_STORAGE_STATUS_ERROR         = 4,  // Card or partition error
} bsp_storage_status_t;

/// @brief Check if a storage type is supported
/// @return Supported
bool bsp_storage_get_supported(bsp_storage_type_t type);

/// @brief Get the status of a storage type
/// @return ESP-IDF error code
esp_err_t bsp_storage_get_status(bsp_storage_type_t type, bsp_storage_status_t* out_status);

/// @brief Mount a storage device
/// @return ESP-IDF error code
esp_err_t bsp_storage_mount(bsp_storage_type_t type, const char* mountpoint);

/// @brief Unmount a storage device
/// @return ESP-IDF error code
esp_err_t bsp_storage_unmount(bsp_storage_type_t type);

/// @brief Check if a storage device is mounted
/// @return ESP-IDF error code
esp_err_t bsp_storage_get_mountpoint(bsp_storage_type_t type, bool* out_mounted);

/// @brief Get the mountpoint of a storage device
/// @return ESP-IDF error code
esp_err_t bsp_storage_get_mountpoint(bsp_storage_type_t type, char* out_mountpoint, size_t max_length);
