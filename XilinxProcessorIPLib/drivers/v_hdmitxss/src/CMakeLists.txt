# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)

project(v_hdmitxss)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xv_hdmitxss.c)
collect (PROJECT_LIB_SOURCES xv_hdmitxss_coreinit.c)
collect (PROJECT_LIB_HEADERS xv_hdmitxss_coreinit.h)
collect (PROJECT_LIB_SOURCES xv_hdmitxss_g.c)
collect (PROJECT_LIB_HEADERS xv_hdmitxss.h)
collect (PROJECT_LIB_SOURCES xv_hdmitxss_hdcp.c)
collect (PROJECT_LIB_SOURCES xv_hdmitxss_log.c)
collect (PROJECT_LIB_SOURCES xv_hdmitxss_sinit.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(v_hdmitxss STATIC ${_sources})
set_target_properties(v_hdmitxss PROPERTIES LINKER_LANGUAGE C)
