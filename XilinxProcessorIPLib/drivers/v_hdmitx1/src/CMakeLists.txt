# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)

project(v_hdmitx1)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xv_hdmitx1.c)
collect (PROJECT_LIB_SOURCES xv_hdmitx1_frl.c)
collect (PROJECT_LIB_HEADERS xv_hdmitx1_frl.h)
collect (PROJECT_LIB_SOURCES xv_hdmitx1_g.c)
collect (PROJECT_LIB_HEADERS xv_hdmitx1.h)
collect (PROJECT_LIB_HEADERS xv_hdmitx1_hw.h)
collect (PROJECT_LIB_SOURCES xv_hdmitx1_intr.c)
collect (PROJECT_LIB_SOURCES xv_hdmitx1_selftest.c)
collect (PROJECT_LIB_SOURCES xv_hdmitx1_sinit.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(v_hdmitx1 STATIC ${_sources})
set_target_properties(v_hdmitx1 PROPERTIES LINKER_LANGUAGE C)
