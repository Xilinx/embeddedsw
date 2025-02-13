# Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(srio)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xsrio.c)
collect (PROJECT_LIB_HEADERS xsrio.h)
collect (PROJECT_LIB_SOURCES xsrio_g.c)
collect (PROJECT_LIB_HEADERS xsrio_hw.h)
collect (PROJECT_LIB_SOURCES xsrio_sinit.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(srio STATIC ${_sources})
set_target_properties(srio PROPERTIES LINKER_LANGUAGE C)
