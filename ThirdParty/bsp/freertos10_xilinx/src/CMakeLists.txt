# Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)
project(freertos)

find_package(common)
include(${CMAKE_CURRENT_SOURCE_DIR}/freertos10_xilinx.cmake * NO_POLICY_SCOPE)

collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)

enable_language(C ASM)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Source)

collect (PROJECT_LIB_HEADERS FreeRTOSSTMTrace.h)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
if (NOT ${YOCTO})
file(COPY ${CMAKE_BINARY_DIR}/include/FreeRTOSConfig.h DESTINATION ${CMAKE_INCLUDE_PATH}/)
endif()
add_library(freertos STATIC ${_sources})
set_target_properties(freertos PROPERTIES LINKER_LANGUAGE C)
install(TARGETS freertos LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INCLUDE_PATH}/..)
