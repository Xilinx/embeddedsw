# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(hdcp22_tx)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xhdcp22_tx_crypt.c)
collect (PROJECT_LIB_SOURCES xhdcp22_tx_g.c)
collect (PROJECT_LIB_HEADERS xhdcp22_tx_i.h)
collect (PROJECT_LIB_SOURCES xhdcp22_tx_sinit.c)
collect (PROJECT_LIB_SOURCES xhdcp22_tx.c)
collect (PROJECT_LIB_HEADERS xhdcp22_tx.h)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(hdcp22_tx STATIC ${_sources})
set_target_properties(hdcp22_tx PROPERTIES LINKER_LANGUAGE C)
