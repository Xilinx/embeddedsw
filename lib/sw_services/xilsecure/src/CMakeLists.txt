# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(xilsecure)

find_package(common)

include(${CMAKE_CURRENT_SOURCE_DIR}/xilsecure.cmake NO_POLICY_SCOPE)

collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${CMAKE_BINARY_DIR}/include)

if(("${XILSECURE_mode}" STREQUAL "server") AND
   (("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR
   (("${CMAKE_MACHINE}" STREQUAL "VersalNet") AND (NOT ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze"))))
  )
  message(FATAL_ERROR "XilSecure library is not supported for selected ${CMAKE_SYSTEM_PROCESSOR} processor in ${XILSECURE_mode} mode")
endif()

add_subdirectory(common)
if("${CMAKE_MACHINE}" STREQUAL "Versal")
  add_subdirectory(versal)
elseif("${CMAKE_MACHINE}" STREQUAL "VersalNet")
  add_subdirectory(versal_net)
elseif("${CMAKE_MACHINE}" STREQUAL "ZynqMP")
  add_subdirectory(zynqmp)
endif()

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

if(NOT ${YOCTO})
  file(COPY ${CMAKE_BINARY_DIR}/include/xsecure_config.h DESTINATION ${CMAKE_INCLUDE_PATH}/)
endif()

add_library(xilsecure STATIC ${_sources})

get_headers(${_headers})

set_target_properties(xilsecure PROPERTIES LINKER_LANGUAGE C)
set_target_properties(xilsecure PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_PATH}/libxilsecure.a;${CMAKE_INCLUDE_PATH}/xsecure_config.h;${CMAKE_LIBRARY_PATH}/libxilsecure_plat.a;${clean_headers}")

install(TARGETS xilsecure LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INCLUDE_PATH}/..)
