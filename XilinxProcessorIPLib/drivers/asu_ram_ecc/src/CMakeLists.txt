# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(asu_ram_ecc)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_HEADERS xasu_ram_ecc.h)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(asu_ram_ecc INTERFACE)
