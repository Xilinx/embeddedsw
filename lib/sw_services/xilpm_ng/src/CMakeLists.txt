# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.14.7)
project(xilpm_ng)

find_package(common)
include(${CMAKE_CURRENT_SOURCE_DIR}/xilpm_ng.cmake NO_POLICY_SCOPE)

collector_create(PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create(PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)

add_subdirectory(util)
add_subdirectory(xilpm_boot)

# Add xilpm_runtime only if EEMI or Subsystem is enabled
if(XILPM_NG_EEMI_ENABLE OR XILPM_NG_SUBSYS_ENABLE)
  add_subdirectory(xilpm_runtime)
endif()

collector_list(_sources PROJECT_LIB_SOURCES)
collector_list(_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

add_library(xilpm_ng STATIC ${_sources})
set_target_properties(xilpm_ng PROPERTIES LINKER_LANGUAGE C)
get_headers(${_headers})
set_target_properties(
  xilpm_ng
  PROPERTIES ADDITIONAL_CLEAN_FILES
             "${CMAKE_LIBRARY_PATH}/libxilpm_ng.a;${clean_headers}"
             COMPILE_OPTIONS "")
install(
  TARGETS xilpm_ng
  LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH}
  ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include
        DESTINATION ${CMAKE_INCLUDE_PATH}/..)
