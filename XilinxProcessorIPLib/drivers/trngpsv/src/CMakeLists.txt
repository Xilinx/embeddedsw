# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(trngpsv)

find_package(common)

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${CMAKE_BINARY_DIR}/include)

collect (PROJECT_LIB_SOURCES CompactAES.c)
collect (PROJECT_LIB_HEADERS CompactAES.h)
collect (PROJECT_LIB_SOURCES xtrngpsv.c)
collect (PROJECT_LIB_HEADERS xtrngpsv.h)
collect (PROJECT_LIB_HEADERS xtrngpsv_alginfo.h)
collect (PROJECT_LIB_SOURCES xtrngpsv_df.c)
collect (PROJECT_LIB_SOURCES xtrngpsv_g.c)
collect (PROJECT_LIB_HEADERS xtrngpsv_hw.h)
collect (PROJECT_LIB_SOURCES xtrngpsv_sinit.c)
collect (PROJECT_LIB_SOURCES xtrngpsv_tests.c)

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

add_library(trngpsv STATIC ${_sources})

set_target_properties(trngpsv PROPERTIES LINKER_LANGUAGE C)

install(TARGETS trngpsv LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
