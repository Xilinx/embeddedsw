# Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(xilocp)

find_package(common)

include(${CMAKE_CURRENT_SOURCE_DIR}/xilocp.cmake NO_POLICY_SCOPE)

if ((NOT ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")) AND
	(NOT ("xilmailbox" IN_LIST BSP_LIBSRC_SUBDIRS)))
      message(FATAL_ERROR "Xilocp library requires xilmailbox library in client mode.")
endif()

collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${CMAKE_BINARY_DIR}/include)

add_subdirectory(common)
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  add_subdirectory(server)
else()
  add_subdirectory(client)
endif()

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

add_library(xilocp STATIC ${_sources})
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
target_compile_options(xilocp PRIVATE -iquote ${CMAKE_BINARY_DIR}/include)
endif()

get_headers(${_headers})

set_target_properties(xilocp PROPERTIES LINKER_LANGUAGE C)
set_target_properties(xilocp PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_PATH}/libxilocp.a;${clean_headers}")

install(TARGETS xilocp LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INCLUDE_PATH}/..)
