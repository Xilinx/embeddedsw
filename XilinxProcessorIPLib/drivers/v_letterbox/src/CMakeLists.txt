# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)

project(v_letterbox)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xv_letterbox.c)
collect (PROJECT_LIB_SOURCES xv_letterbox_g.c)
collect (PROJECT_LIB_HEADERS xv_letterbox_hw.h)
collect (PROJECT_LIB_SOURCES xv_letterbox_l2.c)
collect (PROJECT_LIB_HEADERS xv_letterbox_l2.h)
collect (PROJECT_LIB_SOURCES xv_letterbox_linux.c)
collect (PROJECT_LIB_HEADERS xv_letterbox.h)
collect (PROJECT_LIB_SOURCES xv_letterbox_sinit.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(v_letterbox STATIC ${_sources})
set_target_properties(v_letterbox PROPERTIES LINKER_LANGUAGE C)
