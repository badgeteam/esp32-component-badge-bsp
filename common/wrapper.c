#include "bsp/wrapper.h"
#include "bsp/wrapped/bsp_err.h"

bsp_err_t bsp_wrapper_convert_error(esp_err_t error) {
    if (error == ESP_OK) return BSP_OK;
    if (error == ESP_FAIL) return BSP_FAIL;

    if (error == ESP_ERR_NO_MEM) return BSP_ERR_NO_MEM;
    if (error == ESP_ERR_INVALID_ARG) return BSP_ERR_INVALID_ARG;
    if (error == ESP_ERR_INVALID_STATE) return BSP_ERR_INVALID_STATE;
    if (error == ESP_ERR_INVALID_SIZE) return BSP_ERR_INVALID_SIZE;
    if (error == ESP_ERR_NOT_FOUND) return BSP_ERR_NOT_FOUND;
    if (error == ESP_ERR_NOT_SUPPORTED) return BSP_ERR_NOT_SUPPORTED;
    if (error == ESP_ERR_TIMEOUT) return BSP_ERR_TIMEOUT;
    if (error == ESP_ERR_INVALID_RESPONSE) return BSP_ERR_INVALID_RESPONSE;
    if (error == ESP_ERR_INVALID_CRC) return BSP_ERR_INVALID_CRC;
    if (error == ESP_ERR_INVALID_VERSION) return BSP_ERR_INVALID_VERSION;
    if (error == ESP_ERR_INVALID_MAC) return BSP_ERR_INVALID_MAC;
    if (error == ESP_ERR_NOT_FINISHED) return BSP_ERR_NOT_FINISHED;
    if (error == ESP_ERR_NOT_ALLOWED) return BSP_ERR_NOT_ALLOWED;

    if (error == ESP_ERR_WIFI_BASE) return BSP_ERR_WIFI_BASE;
    if (error == ESP_ERR_MESH_BASE) return BSP_ERR_MESH_BASE;
    if (error == ESP_ERR_FLASH_BASE) return BSP_ERR_FLASH_BASE;
    if (error == ESP_ERR_HW_CRYPTO_BASE) return BSP_ERR_HW_CRYPTO_BASE;
    if (error == ESP_ERR_MEMPROT_BASE) return BSP_ERR_MEMPROT_BASE;
    return BSP_FAIL;
}
