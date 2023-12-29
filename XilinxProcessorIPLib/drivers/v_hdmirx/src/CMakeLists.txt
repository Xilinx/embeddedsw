# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)

project(v_hdmirx)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xv_hdmirx.c)
collect (PROJECT_LIB_HEADERS xv_hdmirx.h)
collect (PROJECT_LIB_SOURCES xv_hdmirx_vsif.c)
collect (PROJECT_LIB_HEADERS xv_hdmirx_vsif.h)
collect (PROJECT_LIB_SOURCES xv_hdmirx_intr.c)
collect (PROJECT_LIB_SOURCES xv_hdmirx_sinit.c)
collect (PROJECT_LIB_SOURCES xv_hdmirx_g.c)
collect (PROJECT_LIB_HEADERS xv_hdmirx_hw.h)
collect (PROJECT_LIB_SOURCES xv_hdmirx_selftest.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(v_hdmirx STATIC ${_sources})
set_target_properties(v_hdmirx PROPERTIES LINKER_LANGUAGE C)
