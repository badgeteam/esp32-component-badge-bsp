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
            include("${TARGETS_FOLDER}/${BSP_TARGET}/target.cmake")
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
)

target_link_libraries(${COMPONENT_LIB} PRIVATE bsp_stub)

include(package_manager)
cu_pkg_define_version(${CMAKE_CURRENT_LIST_DIR})
