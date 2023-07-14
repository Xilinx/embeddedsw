# Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(mcdma)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xmcdma.c)
collect (PROJECT_LIB_HEADERS xmcdma.h)
collect (PROJECT_LIB_SOURCES xmcdma_bd.c)
collect (PROJECT_LIB_HEADERS xmcdma_bd.h)
collect (PROJECT_LIB_SOURCES xmcdma_g.c)
collect (PROJECT_LIB_HEADERS xmcdma_hw.h)
collect (PROJECT_LIB_SOURCES xmcdma_intr.c)
collect (PROJECT_LIB_SOURCES xmcdma_selftest.c)
collect (PROJECT_LIB_SOURCES xmcdma_sinit.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(mcdma STATIC ${_sources})
set_target_properties(mcdma PROPERTIES LINKER_LANGUAGE C)
