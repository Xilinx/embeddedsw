# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)

project(dp12rxss)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/hdcp1x)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/iic)

collect (PROJECT_LIB_SOURCES xdprxss_dbg.c)
collect (PROJECT_LIB_SOURCES xdprxss_g.c)
collect (PROJECT_LIB_HEADERS xdprxss_hw.h)
collect (PROJECT_LIB_SOURCES xdprxss_intr.c)
collect (PROJECT_LIB_SOURCES xdprxss_dp159.c)
collect (PROJECT_LIB_HEADERS xdprxss_dp159.h)
collect (PROJECT_LIB_SOURCES xdprxss_selftest.c)
collect (PROJECT_LIB_SOURCES xdprxss_sinit.c)
collect (PROJECT_LIB_SOURCES xdprxss.c)
collect (PROJECT_LIB_HEADERS xdprxss.h)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(dp12rxss STATIC ${_sources})
set_target_properties(dp12rxss PROPERTIES LINKER_LANGUAGE C)
