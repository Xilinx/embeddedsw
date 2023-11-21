# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)
project(v_axi4s_remap)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xv_axi4s_remap.c)
collect (PROJECT_LIB_SOURCES xv_axi4s_remap_g.c)
collect (PROJECT_LIB_HEADERS xv_axi4s_remap_hw.h)
collect (PROJECT_LIB_SOURCES xv_axi4s_remap_linux.c)
collect (PROJECT_LIB_SOURCES xv_axi4s_remap_sinit.c)
collect (PROJECT_LIB_HEADERS xv_axi4s_remap.h)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(v_axi4s_remap STATIC ${_sources})
set_target_properties(v_axi4s_remap PROPERTIES LINKER_LANGUAGE C)
