# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(trngpsx)

find_package(common)

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${CMAKE_BINARY_DIR}/include)

collect (PROJECT_LIB_HEADERS xtrngpsx.h)
collect (PROJECT_LIB_HEADERS xtrngpsx_alginfo.h)
collect (PROJECT_LIB_HEADERS xtrngpsx_hw.h)

collect (PROJECT_LIB_SOURCES xtrngpsx.c)
collect (PROJECT_LIB_SOURCES xtrngpsx_g.c)
collect (PROJECT_LIB_SOURCES xtrngpsx_sinit.c)
collect (PROJECT_LIB_SOURCES xtrngpsx_tests.c)

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

add_library(trngpsx STATIC ${_sources})

set_target_properties(trngpsx PROPERTIES LINKER_LANGUAGE C)

install(TARGETS trngpsx LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})