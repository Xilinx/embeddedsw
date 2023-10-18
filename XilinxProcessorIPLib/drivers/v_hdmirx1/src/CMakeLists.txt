# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)

project(v_hdmirx1)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xv_hdmirx1.c)
collect (PROJECT_LIB_HEADERS xv_hdmirx1_frl.h)
collect (PROJECT_LIB_HEADERS xv_hdmirx1.h)
collect (PROJECT_LIB_SOURCES xv_hdmirx1_intr.c)
collect (PROJECT_LIB_SOURCES xv_hdmirx1_sinit.c)
collect (PROJECT_LIB_SOURCES xv_hdmirx1_frl.c)
collect (PROJECT_LIB_SOURCES xv_hdmirx1_g.c)
collect (PROJECT_LIB_HEADERS xv_hdmirx1_hw.h)
collect (PROJECT_LIB_SOURCES xv_hdmirx1_selftest.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(v_hdmirx1 STATIC ${_sources})
set_target_properties(v_hdmirx1 PROPERTIES LINKER_LANGUAGE C)
