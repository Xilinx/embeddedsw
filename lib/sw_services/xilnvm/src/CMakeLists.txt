# Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(xilnvm)

find_package(common)

include(${CMAKE_CURRENT_SOURCE_DIR}/xilnvm.cmake NO_POLICY_SCOPE)

collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${CMAKE_BINARY_DIR}/include)

if(("${XILNVM_mode}" STREQUAL "client") AND
	(NOT ("xilmailbox" IN_LIST BSP_LIBSRC_SUBDIRS)))
    message(WARNING "XilNvm library requires xilmailbox library in client mode.")
endif()

if("${CMAKE_MACHINE}" STREQUAL "Versal" OR "${CMAKE_MACHINE}" STREQUAL "VersalNet")
  add_subdirectory(versal_gen)
elseif("${CMAKE_MACHINE}" STREQUAL "spartanuplus")
  add_subdirectory(spartanup)
endif()

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

if (NOT ${YOCTO})
  file(COPY ${CMAKE_BINARY_DIR}/include/xilnvm_bsp_config.h DESTINATION ${CMAKE_INCLUDE_PATH}/)
endif()

add_library(xilnvm STATIC ${_sources})
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
target_compile_options(xilnvm PRIVATE -iquote ${CMAKE_BINARY_DIR}/include)
endif()

get_headers(${_headers})

set_target_properties(xilnvm PROPERTIES LINKER_LANGUAGE C)
set_target_properties(xilnvm PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_PATH}/libxilnvm.a;${CMAKE_INCLUDE_PATH}/xilnvm_bsp_config.h;${clean_headers}")

install(TARGETS xilnvm LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INCLUDE_PATH}/..)
