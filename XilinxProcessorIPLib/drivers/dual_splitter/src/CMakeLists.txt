# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)

project(dual_splitter)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)

collect (PROJECT_LIB_SOURCES xdualsplitter_g.c)
collect (PROJECT_LIB_HEADERS xdualsplitter_hw.h)
collect (PROJECT_LIB_SOURCES xdualsplitter_intr.c)
collect (PROJECT_LIB_SOURCES xdualsplitter_selftest.c)
collect (PROJECT_LIB_SOURCES xdualsplitter_sinit.c)
collect (PROJECT_LIB_SOURCES xdualsplitter.c)
collect (PROJECT_LIB_HEADERS xdualsplitter.h)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(dual_splitter STATIC ${_sources})
set_target_properties(dual_splitter PROPERTIES LINKER_LANGUAGE C)
