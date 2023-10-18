# Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(v_hdmi_common)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xv_hdmic.c)
collect (PROJECT_LIB_HEADERS xv_hdmic.h)
collect (PROJECT_LIB_SOURCES xv_hdmic_vsif.c)
collect (PROJECT_LIB_HEADERS xv_hdmic_vsif.h)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(v_hdmi_common STATIC ${_sources})
set_target_properties(v_hdmi_common PROPERTIES LINKER_LANGUAGE C)
