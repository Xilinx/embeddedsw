# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)
project(scutimer)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xscutimer.c)
collect (PROJECT_LIB_HEADERS xscutimer.h)
collect (PROJECT_LIB_SOURCES xscutimer_g.c)
collect (PROJECT_LIB_HEADERS xscutimer_hw.h)
collect (PROJECT_LIB_SOURCES xscutimer_selftest.c)
collect (PROJECT_LIB_SOURCES xscutimer_sinit.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(scutimer STATIC ${_sources})
set_target_properties(scutimer PROPERTIES LINKER_LANGUAGE C)
