# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(iomodule)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xiomodule.c)
collect (PROJECT_LIB_HEADERS xiomodule.h)
collect (PROJECT_LIB_SOURCES xiomodule_extra.c)
collect (PROJECT_LIB_SOURCES xiomodule_g.c)
collect (PROJECT_LIB_HEADERS xiomodule_i.h)
collect (PROJECT_LIB_SOURCES xiomodule_intr.c)
collect (PROJECT_LIB_HEADERS xiomodule_io.h)
collect (PROJECT_LIB_SOURCES xiomodule_l.c)
collect (PROJECT_LIB_HEADERS xiomodule_l.h)
collect (PROJECT_LIB_SOURCES xiomodule_options.c)
collect (PROJECT_LIB_SOURCES xiomodule_selftest.c)
collect (PROJECT_LIB_SOURCES xiomodule_stats.c)
collect (PROJECT_LIB_SOURCES xiomodule_uart.c)
collect (PROJECT_LIB_SOURCES xiomodule_uart_intr.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(iomodule STATIC ${_sources})
set_target_properties(iomodule PROPERTIES LINKER_LANGUAGE C)
