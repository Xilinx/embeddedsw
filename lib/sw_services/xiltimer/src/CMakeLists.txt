# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)
project(xiltimer)

include(${CMAKE_CURRENT_SOURCE_DIR}/XiltimerExample.cmake NO_POLICY_SCOPE)
include(${CMAKE_CURRENT_SOURCE_DIR}/xiltimer.cmake NO_POLICY_SCOPE)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/core)

collect (PROJECT_LIB_SOURCES xiltimer.c)
collect (PROJECT_LIB_HEADERS xiltimer.h)
if (${NON_YOCTO})
collect (PROJECT_LIB_HEADERS sleep.h)
endif()
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
if (${NON_YOCTO})
file(COPY ${CMAKE_BINARY_DIR}/include/xtimer_config.h DESTINATION ${CMAKE_INCLUDE_PATH}/)
endif()
add_library(xiltimer STATIC ${_sources})
set_target_properties(xiltimer PROPERTIES LINKER_LANGUAGE C)
get_headers(${_headers})
set_target_properties(xiltimer PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_PATH}/libxiltimer.a;${CMAKE_INCLUDE_PATH}/xtimer_config.h;${clean_headers}")
install(TARGETS xiltimer LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INCLUDE_PATH}/..)
