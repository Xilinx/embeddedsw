# Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(zdma)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xzdma_intr.c)
collect (PROJECT_LIB_SOURCES xzdma_sinit.c)
collect (PROJECT_LIB_HEADERS xzdma_hw.h)
collect (PROJECT_LIB_SOURCES xzdma_selftest.c)
collect (PROJECT_LIB_SOURCES xzdma.c)
collect (PROJECT_LIB_HEADERS xzdma.h)
collect (PROJECT_LIB_SOURCES xzdma_g.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(zdma STATIC ${_sources})
set_target_properties(zdma PROPERTIES LINKER_LANGUAGE C)
