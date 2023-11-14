# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)

project(dp21txss)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dualsplitter)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/hdcp1x)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/hdcp22)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vtc)

collect (PROJECT_LIB_SOURCES xdptxss_dbg.c)
collect (PROJECT_LIB_SOURCES xdptxss_g.c)
collect (PROJECT_LIB_HEADERS xdptxss_hw.h)
collect (PROJECT_LIB_SOURCES xdptxss_intr.c)
collect (PROJECT_LIB_SOURCES xdptxss_selftest.c)
collect (PROJECT_LIB_SOURCES xdptxss_sinit.c)
collect (PROJECT_LIB_SOURCES xdptxss.c)
collect (PROJECT_LIB_HEADERS xdptxss.h)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(dp21txss STATIC ${_sources})
set_target_properties(dp21txss PROPERTIES LINKER_LANGUAGE C)
