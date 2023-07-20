# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(xilnvm)

find_package(common)

include(${CMAKE_CURRENT_SOURCE_DIR}/xilnvm.cmake NO_POLICY_SCOPE)

include_directories(${CMAKE_BINARY_DIR}/include)

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") AND
   ("${XILNVM_mode}" STREQUAL "server")
  )
  message(FATAL_ERROR "XilNvm server library is not supported for microblaze.")
endif()

collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(common)
if("${CMAKE_MACHINE}" STREQUAL "Versal")
  add_subdirectory(versal)
elseif("${CMAKE_MACHINE}" STREQUAL "VersalNet")
  add_subdirectory(versal_net)
endif()

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

if (${NON_YOCTO})
  file(COPY ${CMAKE_BINARY_DIR}/include/xilnvm_bsp_config.h DESTINATION ${CMAKE_INCLUDE_PATH}/)
endif()

add_library(xilnvm STATIC ${_sources})

get_headers(${_headers})

set_target_properties(xilnvm PROPERTIES LINKER_LANGUAGE C)
set_target_properties(xilnvm PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_PATH}/libxilnvm.a;${CMAKE_INCLUDE_PATH}/xilnvm_bsp_config.h;${clean_headers}")

install(TARGETS xilnvm LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INCLUDE_PATH}/..)
