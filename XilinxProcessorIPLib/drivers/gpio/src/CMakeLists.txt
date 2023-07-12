# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(gpio)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xgpio.c)
collect (PROJECT_LIB_HEADERS xgpio.h)
collect (PROJECT_LIB_SOURCES xgpio_extra.c)
collect (PROJECT_LIB_SOURCES xgpio_g.c)
collect (PROJECT_LIB_HEADERS xgpio_i.h)
collect (PROJECT_LIB_SOURCES xgpio_intr.c)
collect (PROJECT_LIB_HEADERS xgpio_l.h)
collect (PROJECT_LIB_SOURCES xgpio_selftest.c)
collect (PROJECT_LIB_SOURCES xgpio_sinit.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(gpio STATIC ${_sources})
set_target_properties(gpio PROPERTIES LINKER_LANGUAGE C)
