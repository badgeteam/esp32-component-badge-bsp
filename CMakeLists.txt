# BSP target selection

if (CONFIG_BSP_TARGET_TANMATSU)
    set(BSP_TARGET "tanmatsu")
elseif (CONFIG_BSP_TARGET_KONSOOL)
    set(BSP_TARGET "tanmatsu")
elseif (CONFIG_BSP_TARGET_HACKERHOTEL_2026)
    set(BSP_TARGET "tanmatsu")
elseif(CONFIG_BSP_TARGET_ESP32_P4_FUNCTION_EV_BOARD)
    set(BSP_TARGET "esp32-p4-function-ev-board")
elseif(CONFIG_BSP_TARGET_MCH2022)
    set(BSP_TARGET "mch2022")
elseif(CONFIG_BSP_TARGET_HACKERHOTEL_2024)
    set(BSP_TARGET "hackerhotel-2024")
elseif(CONFIG_BSP_TARGET_KAMI)
    set(BSP_TARGET "kami")
elseif(CONFIG_BSP_TARGET_BORNHACK_2025_CIRCLE)
    set(BSP_TARGET "bornhack-2025-circle")
elseif(CONFIG_BSP_TARGET_BORNHACK_2024_POV)
    set(BSP_TARGET "bornhack-2024-pov")
else()
    set(BSP_TARGET "stub")
endif()

# BSP requirements for all targets

set(COMMON_BSP_REQUIREMENTS
    "esp_lcd"
    "driver"
    "ssd1619"
    "bootloader_support"
)

# BSP requirements only used for ESP32 targets

set(ESP32_BSP_REQUIREMENTS
    "espressif__esp_lcd_ili9341"
    "badgeteam__mch2022-rp2040"
)

# BSP requirements only used on for ESP32-P4 targets

set(ESP32P4_BSP_REQUIREMENTS
    "nicolaielectronics__tanmatsu_coprocessor"
    "nicolaielectronics__mipi_dsi_abstraction"
    "es8156"
)

# -------------------------

set(BSP_REQUIRES ${COMMON_BSP_REQUIREMENTS})

if("${IDF_TARGET}" STREQUAL "esp32")
    list(APPEND BSP_REQUIRES ${ESP32_BSP_REQUIREMENTS})
elseif("${IDF_TARGET}" STREQUAL "esp32p4")
    list(APPEND BSP_REQUIRES ${ESP32P4_BSP_REQUIREMENTS})
else()
    message(WARNING "BSP: unsupported target ${target}")
endif()

if (NOT DEFINED BSP_SRCS)
    file(GLOB BSP_SRCS
        "targets/${BSP_TARGET}/*.c"
    )
endif()

if (NOT DEFINED COMMON_BSP_SRCS)
    file(GLOB COMMON_BSP_SRCS
        "common/*.c"
    )
endif()

idf_component_register(
    SRCS ${BSP_SRCS} ${COMMON_BSP_SRCS}
    INCLUDE_DIRS "."
    REQUIRES ${BSP_REQUIRES}
    WHOLE_ARCHIVE
)

file(GLOB BSP_STUB_SRCS
    "stub/*.c"
)

add_library(
    bsp_stub
    STATIC
    ${BSP_STUB_SRCS}
)

target_include_directories(
    bsp_stub
    PUBLIC
    "."
    "$<TARGET_PROPERTY:${COMPONENT_LIB},INTERFACE_INCLUDE_DIRECTORIES>"
)

target_link_libraries(${COMPONENT_LIB} PRIVATE bsp_stub)

include(package_manager)
cu_pkg_define_version(${CMAKE_CURRENT_LIST_DIR})
