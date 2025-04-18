# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.14.7)
project(xilpm)

find_package(common)
include(${CMAKE_CURRENT_SOURCE_DIR}/xilpm.cmake NO_POLICY_SCOPE)
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)

if("${CMAKE_MACHINE}" STREQUAL "Versal" OR "${CMAKE_MACHINE}" STREQUAL "VersalNet")
	add_subdirectory(versal_common)
endif()

if("${CMAKE_MACHINE}" STREQUAL "Versal")
        add_subdirectory(versal)
elseif("${CMAKE_MACHINE}" STREQUAL "VersalNet")
        add_subdirectory(versal_net)
elseif("${CMAKE_MACHINE}" STREQUAL "ZynqMP")
        add_subdirectory(zynqmp/client)
else()
	message(FATAL_ERROR "Wrong machine name ${CMAKE_MACHINE}")
endif()

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

if (NOT ${YOCTO})
	file(COPY ${CMAKE_BINARY_DIR}/include/xpm_config.h DESTINATION ${CMAKE_INCLUDE_PATH}/)
endif()

add_library(xilpm STATIC ${_sources})
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
target_compile_options(xilpm PRIVATE -iquote ${CMAKE_BINARY_DIR}/include)
endif()
set_target_properties(xilpm PROPERTIES LINKER_LANGUAGE C)
get_headers(${_headers})
set_target_properties(xilpm PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_PATH}/libxilpm.a;${CMAKE_INCLUDE_PATH}/xpm_config.h;${clean_headers}" COMPILE_OPTIONS "")
install(TARGETS xilpm LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INCLUDE_PATH}/..)
