# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

project(xilrsa)

find_package(common)

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")

collect (PROJECT_LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/xilrsa.h)

collector_list (_headers PROJECT_LIB_HEADERS)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/librsa.a ${CMAKE_BINARY_DIR}/libxilrsa.a)

if (${NON_YOCTO})
  file(COPY ${CMAKE_BINARY_DIR}/libxilrsa.a DESTINATION ${CMAKE_LIBRARY_PATH} FOLLOW_SYMLINK_CHAIN)
endif()
