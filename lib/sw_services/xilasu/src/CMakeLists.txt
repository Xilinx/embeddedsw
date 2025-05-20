# Copyright (C) 2024-2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(xilasu)

find_package(common)

include(${CMAKE_CURRENT_SOURCE_DIR}/xilasu.cmake NO_POLICY_SCOPE)

collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${CMAKE_BINARY_DIR}/include)

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze_riscv")
  if((XILASU_keywrap_support) AND NOT ((XILASU_rsa_padding_support)))
    set(XILASU_rsa_padding_support TRUE CACHE BOOL "Padding is enabled as keywrap depends on RSA padding" FORCE)
  endif()

  if((XILASU_kdf_support) AND NOT ((XILASU_hmac_support)))
    set(XILASU_hmac_support TRUE CACHE BOOL "HMAC is enabled as KDF depends on HMAC" FORCE)
  endif()

  if((XILASU_ecies_support) AND NOT ((XILASU_hmac_support)))
    set(XILASU_hmac_support TRUE CACHE BOOL "HMAC is enabled as ECIES depends on HMAC " FORCE)
    if(NOT(XILASU_ecc_p192_support) AND NOT(XILASU_ecc_p224_support) AND NOT(XILASU_ecc_p256_support)
      AND NOT(XILASU_ecc_p384_support) AND NOT(XILASU_ecc_p521_support) AND NOT(XILASU_ecc_bp_p256_support)
      AND NOT(XILASU_ecc_bp_p320_support) AND NOT(XILASU_ecc_bp_p384_support) AND NOT(XILASU_ecc_bp_p512_support))
      message(FATAL_ERROR "Enable any supported curve for ECIES to be enabled")
    endif()
  endif()
endif()

add_subdirectory(common)
if("${XILASU_Mode}" STREQUAL "server")
  add_subdirectory(server)
else()
  add_subdirectory(client)
endif()

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

if(NOT ${YOCTO})
file(COPY ${CMAKE_BINARY_DIR}/include/xasu_bsp_config.h DESTINATION ${CMAKE_INCLUDE_PATH}/)
endif()

add_library(xilasu STATIC ${_sources})
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
target_compile_options(xilasu PRIVATE -iquote ${CMAKE_BINARY_DIR}/include)
endif()

get_headers(${_headers})

set_target_properties(xilasu PROPERTIES LINKER_LANGUAGE C)
set_target_properties(xilasu PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_PATH}/libxilasu.a;${clean_headers}")

install(TARGETS xilasu LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INCLUDE_PATH}/..)
