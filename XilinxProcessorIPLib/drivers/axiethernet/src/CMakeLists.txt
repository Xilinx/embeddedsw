# Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)
project(axiethernet)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xaxiethernet.c)
collect (PROJECT_LIB_HEADERS xaxiethernet.h)
collect (PROJECT_LIB_SOURCES xaxiethernet_control.c)
collect (PROJECT_LIB_SOURCES xaxiethernet_g.c)
collect (PROJECT_LIB_HEADERS xaxiethernet_hw.h)
collect (PROJECT_LIB_HEADERS xaxiethernet_porting_guide.h)
collect (PROJECT_LIB_SOURCES xaxiethernet_sinit.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(axiethernet STATIC ${_sources})
set_target_properties(axiethernet PROPERTIES LINKER_LANGUAGE C)
