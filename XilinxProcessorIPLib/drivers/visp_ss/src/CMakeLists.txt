# Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(visp_ss)
include(CheckSymbolExists)
find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
include(${CMAKE_CURRENT_SOURCE_DIR}/VISP_SSConfig.cmake)
collect (PROJECT_LIB_HEADERS xvisp_ss.h)
collect (PROJECT_LIB_SOURCES xvisp_ss_g.c)
collect (PROJECT_LIB_SOURCES xvisp_ss_sinit.c)
collect (PROJECT_LIB_SOURCES xvisp_ss.c)
string(REGEX MATCH "cortex-r52" r52 "${CMAKE_CXX_FLAGS}")

# Exclude for R52 cores 6,7,8,9

if (NOT ${YOCTO})
file(COPY ${CMAKE_BINARY_DIR}/include/bspconfig.h DESTINATION ${CMAKE_INCLUDE_PATH}/)
endif()

string(REGEX MATCH "#define[ \t]+XPAR_CPU_ID[ \t]+([0-9]+)"
       _match "${BSP_CONTENT}")

set(CPU_ID "${CMAKE_MATCH_1}")

message(STATUS "CPU_ID='${CPU_ID}'")


if(NOT (r52 AND
         (CPU_ID EQUAL 6 OR
          CPU_ID EQUAL 7 OR
          CPU_ID EQUAL 8 OR
          CPU_ID EQUAL 9)))

    add_subdirectory(mailbox)
    add_subdirectory(visp_ss)
    collect (PROJECT_LIB_HEADERS wdt_notifier.h)
    collect (PROJECT_LIB_SOURCES wdt_notifier.c)
    check_symbol_exists(XPAR_I2C0_BASEADDR "xparameters.h" HAVE_XIICPS1)
    if(HAVE_XIICPS1)
        add_subdirectory(sensor)
    endif()

endif()

add_compile_options(-w)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(visp_ss STATIC ${_sources})
set_target_properties(visp_ss PROPERTIES LINKER_LANGUAGE C)

# Apply user compile definitions
if(DEFINED USER_COMPILE_DEFINITIONS)
    target_compile_definitions(visp_ss PRIVATE ${USER_COMPILE_DEFINITIONS})
endif()
