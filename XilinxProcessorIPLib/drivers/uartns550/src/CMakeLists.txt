# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(uartns550)

find_package(common)
# If the PG flag is present, remove any references to it as BSP doesn't support the mcount API.
remove_pg()

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xuartns550.c)
collect (PROJECT_LIB_HEADERS xuartns550.h)
collect (PROJECT_LIB_SOURCES xuartns550_format.c)
collect (PROJECT_LIB_SOURCES xuartns550_g.c)
collect (PROJECT_LIB_HEADERS xuartns550_i.h)
collect (PROJECT_LIB_SOURCES xuartns550_intr.c)
collect (PROJECT_LIB_SOURCES xuartns550_l.c)
collect (PROJECT_LIB_HEADERS xuartns550_l.h)
collect (PROJECT_LIB_SOURCES xuartns550_options.c)
collect (PROJECT_LIB_SOURCES xuartns550_selftest.c)
collect (PROJECT_LIB_SOURCES xuartns550_sinit.c)
collect (PROJECT_LIB_SOURCES xuartns550_stats.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(uartns550 STATIC ${_sources})
set_target_properties(uartns550 PROPERTIES LINKER_LANGUAGE C)
