# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(usb)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xusb.c)
collect (PROJECT_LIB_HEADERS xusb.h)
collect (PROJECT_LIB_SOURCES xusb_endpoint.c)
collect (PROJECT_LIB_SOURCES xusb_g.c)
collect (PROJECT_LIB_SOURCES xusb_intr.c)
collect (PROJECT_LIB_HEADERS xusb_l.h)
collect (PROJECT_LIB_SOURCES xusb_sinit.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(usb STATIC ${_sources})
set_target_properties(usb PROPERTIES LINKER_LANGUAGE C)
