# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)

project(spdif)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xspdif.c)
collect (PROJECT_LIB_HEADERS xspdif.h)
collect (PROJECT_LIB_SOURCES xspdif_g.c)
collect (PROJECT_LIB_HEADERS xspdif_hw.h)
collect (PROJECT_LIB_SOURCES xspdif_intr.c)
collect (PROJECT_LIB_SOURCES xspdif_selftest.c)
collect (PROJECT_LIB_SOURCES xspdif_sinit.c)
collect (PROJECT_LIB_SOURCES xspdif_chsts.c)
collect (PROJECT_LIB_HEADERS xspdif_chsts.h)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(spdif STATIC ${_sources})
set_target_properties(spdif PROPERTIES LINKER_LANGUAGE C)
