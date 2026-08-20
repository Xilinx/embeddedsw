# Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(amd_xspi)

find_package(common)

collector_create(PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create(PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${CMAKE_BINARY_DIR}/include)

collect(PROJECT_LIB_HEADERS amd_xspi.h)
collect(PROJECT_LIB_HEADERS amd_xspi_hw.h)
collect(PROJECT_LIB_HEADERS amd_xspi_control.h)

collect(PROJECT_LIB_SOURCES amd_xspi.c)
collect(PROJECT_LIB_SOURCES amd_xspi_control.c)
collect(PROJECT_LIB_SOURCES amd_xspi_xfer.c)
collect(PROJECT_LIB_SOURCES amd_xspi_hw.c)
collect(PROJECT_LIB_SOURCES amd_xspi_options.c)
collect(PROJECT_LIB_SOURCES amd_xspi_sinit.c)
collect(PROJECT_LIB_SOURCES amd_xspi_g.c)

collector_list(_sources PROJECT_LIB_SOURCES)
collector_list(_headers PROJECT_LIB_HEADERS)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

add_library(amd_xspi STATIC ${_sources})

set_target_properties(amd_xspi PROPERTIES LINKER_LANGUAGE C)
