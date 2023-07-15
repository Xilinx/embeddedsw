# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(intc)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xintc.c)
collect (PROJECT_LIB_HEADERS xintc.h)
collect (PROJECT_LIB_SOURCES xintc_g.c)
collect (PROJECT_LIB_HEADERS xintc_i.h)
collect (PROJECT_LIB_SOURCES xintc_intr.c)
collect (PROJECT_LIB_SOURCES xintc_l.c)
collect (PROJECT_LIB_HEADERS xintc_l.h)
collect (PROJECT_LIB_SOURCES xintc_options.c)
collect (PROJECT_LIB_SOURCES xintc_selftest.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(intc STATIC ${_sources})
set_target_properties(intc PROPERTIES LINKER_LANGUAGE C)
