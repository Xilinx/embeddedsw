# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(cframe)

find_package(common)

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${CMAKE_BINARY_DIR}/include)

collect (PROJECT_LIB_SOURCES xcframe.c)
collect (PROJECT_LIB_HEADERS xcframe.h)
collect (PROJECT_LIB_SOURCES xcframe_g.c)
collect (PROJECT_LIB_HEADERS xcframe_hw.h)
collect (PROJECT_LIB_SOURCES xcframe_selftest.c)
collect (PROJECT_LIB_SOURCES xcframe_sinit.c)

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

add_library(cframe STATIC ${_sources})

set_target_properties(cframe PROPERTIES LINKER_LANGUAGE C)

install(TARGETS cframe LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
