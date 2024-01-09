# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(hdcp22_rx_dp)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xhdcp22_rx_dp_crypt.c)
collect (PROJECT_LIB_SOURCES xhdcp22_rx_dp_g.c)
collect (PROJECT_LIB_HEADERS xhdcp22_rx_dp_i.h)
collect (PROJECT_LIB_SOURCES xhdcp22_rx_dp_sinit.c)
collect (PROJECT_LIB_SOURCES xhdcp22_rx_dp_test.c)
collect (PROJECT_LIB_SOURCES xhdcp22_rx_dp.c)
collect (PROJECT_LIB_HEADERS xhdcp22_rx_dp.h)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(hdcp22_rx_dp STATIC ${_sources})
set_target_properties(hdcp22_rx_dp PROPERTIES LINKER_LANGUAGE C)
