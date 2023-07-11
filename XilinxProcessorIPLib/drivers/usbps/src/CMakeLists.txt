# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(usbps)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xusbps.c)
collect (PROJECT_LIB_HEADERS xusbps.h)
collect (PROJECT_LIB_SOURCES xusbps_endpoint.c)
collect (PROJECT_LIB_HEADERS xusbps_endpoint.h)
collect (PROJECT_LIB_SOURCES xusbps_g.c)
collect (PROJECT_LIB_SOURCES xusbps_hw.c)
collect (PROJECT_LIB_HEADERS xusbps_hw.h)
collect (PROJECT_LIB_SOURCES xusbps_intr.c)
collect (PROJECT_LIB_SOURCES xusbps_sinit.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(usbps STATIC ${_sources})
set_target_properties(usbps PROPERTIES LINKER_LANGUAGE C)
