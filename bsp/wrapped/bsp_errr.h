#pragma once

typedef int bsp_err_t;

#define BSP_OK   0
#define BSP_FAIL -1

#define BSP_ERR_NO_MEM           0x101
#define BSP_ERR_INVALID_ARG      0x102
#define BSP_ERR_INVALID_STATE    0x103
#define BSP_ERR_INVALID_SIZE     0x104
#define BSP_ERR_NOT_FOUND        0x105
#define BSP_ERR_NOT_SUPPORTED    0x106
#define BSP_ERR_TIMEOUT          0x107
#define BSP_ERR_INVALID_RESPONSE 0x108
#define BSP_ERR_INVALID_CRC      0x109
#define BSP_ERR_INVALID_VERSION  0x10A
#define BSP_ERR_INVALID_MAC      0x10B
#define BSP_ERR_NOT_FINISHED     0x10C
#define BSP_ERR_NOT_ALLOWED      0x10D

#define BSP_ERR_WIFI_BASE      0x3000
#define BSP_ERR_MESH_BASE      0x4000
#define BSP_ERR_FLASH_BASE     0x6000
#define BSP_ERR_HW_CRYPTO_BASE 0xc000
#define BSP_ERR_MEMPROT_BASE   0xd000
