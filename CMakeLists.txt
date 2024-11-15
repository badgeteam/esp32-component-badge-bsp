include(CMakePrintHelpers)

set(BSP_REQUIRES
		"nicolaielectronics__tanmatsu_coprocessor"
		"nicolaielectronics__mipi_dsi_abstraction"
		"esp32-component-ch32v203prog"
		"driver"
		"fatfs"
)

set(BSP_EMBED_FILES "ch32_firmware.bin")

if (CONFIG_BSP_PLATFORM_WHY2025)
	set(BSP_TARGET "why2025")
elseif(CONFIG_BSP_PLATFORM_P4_DEVKIT)
	set(BSP_TARGET "why2025")
elseif(CONFIG_BSP_PLATFORM_MCH2022)
	set(BSP_TARGET "mch2022")
endif()


# BSP target selection
if (NOT DEFINED BSP_TARGET)
    set(BSP_TARGET "stub")
endif()

set(TARGETS_FOLDER "${CMAKE_CURRENT_LIST_DIR}/targets")

if("${BSP_TARGET}" STREQUAL "stub")
    message(WARNING "No BSP target selected")
else()
    IF(EXISTS "${TARGETS_FOLDER}/${BSP_TARGET}" AND IS_DIRECTORY "${TARGETS_FOLDER}/${BSP_TARGET}")
        IF(EXISTS "${TARGETS_FOLDER}/${BSP_TARGET}/target.cmake")
					# include("${TARGETS_FOLDER}/${BSP_TARGET}/target.cmake")
        endif()
    else()
        message(FATAL_ERROR "Unsupported BSP target ${BSP_TARGET}")
    endif()
endif()

if (NOT DEFINED BSP_REQUIRES)
  set(BSP_REQUIRES "")
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

cmake_print_variables(BSP_REQUIRES)

idf_component_register(
    SRCS ${BSP_SRCS} ${COMMON_BSP_SRCS}
    INCLUDE_DIRS "."
    REQUIRES ${BSP_REQUIRES}
		EMBED_FILES ${BSP_EMBED_FILES}
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
)

target_link_libraries(${COMPONENT_LIB} PRIVATE bsp_stub)

include(package_manager)
cu_pkg_define_version(${CMAKE_CURRENT_LIST_DIR})
