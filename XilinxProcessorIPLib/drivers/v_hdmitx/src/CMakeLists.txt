# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)

project(v_hdmitx)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xv_hdmitx.c)
collect (PROJECT_LIB_HEADERS xv_hdmitx.h)
collect (PROJECT_LIB_SOURCES xv_hdmitx_g.c)
collect (PROJECT_LIB_SOURCES xv_hdmitx_vsif.c)
collect (PROJECT_LIB_HEADERS xv_hdmitx_vsif.h)
collect (PROJECT_LIB_HEADERS xv_hdmitx_hw.h)
collect (PROJECT_LIB_SOURCES xv_hdmitx_intr.c)
collect (PROJECT_LIB_SOURCES xv_hdmitx_selftest.c)
collect (PROJECT_LIB_SOURCES xv_hdmitx_sinit.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(v_hdmitx STATIC ${_sources})
set_target_properties(v_hdmitx PROPERTIES LINKER_LANGUAGE C)
