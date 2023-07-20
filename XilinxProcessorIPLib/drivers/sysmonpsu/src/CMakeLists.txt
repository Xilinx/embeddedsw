# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(sysmonpsu)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_HEADERS xsysmonpsu.h)
collect (PROJECT_LIB_SOURCES xsysmonpsu_selftest.c)
collect (PROJECT_LIB_SOURCES xsysmonpsu_sinit.c)
collect (PROJECT_LIB_SOURCES xsysmonpsu_intr.c)
collect (PROJECT_LIB_SOURCES xsysmonpsu.c)
collect (PROJECT_LIB_SOURCES xsysmonpsu_g.c)
collect (PROJECT_LIB_HEADERS xsysmonpsu_hw.h)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(sysmonpsu STATIC ${_sources})
set_target_properties(sysmonpsu PROPERTIES LINKER_LANGUAGE C)
