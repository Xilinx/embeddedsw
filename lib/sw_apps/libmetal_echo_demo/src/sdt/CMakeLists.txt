# Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
cmake_minimum_required (VERSION 3.15)
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

project (libmetal_amp_demod C)

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -specs=${CMAKE_SPECS_FILE} " )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CMAKE_INCLUDE_PATH} " )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${proc_extra_compiler_flags}" )

set (APP_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openamp-system-reference")
set (CMAKE_UTIL_DIR "${APP_BASE_DIR}/examples/legacy_apps/cmake")
set (WITH_LIBMETAL_FIND OFF)
set (WITH_OPENAMP_FIND OFF)

include (${CMAKE_UTIL_DIR}/syscheck.cmake)
include (CheckIncludeFiles)
include (CheckCSourceCompiles)
include (${CMAKE_UTIL_DIR}/collect.cmake)
include (${CMAKE_UTIL_DIR}/depends.cmake)

set_property (GLOBAL PROPERTY "PROJECT_EC_FLAGS" -Wall -Wextra)
set_property (GLOBAL PROPERTY DEMO_CFG_FILE "${APP_BASE_DIR}/examples/libmetal/machine/remote/amd_rpu/rpu.cmake")

include(CheckSymbolExists)
check_symbol_exists(VERSAL_AIEPG2 "bspconfig.h" IS_VERSAL2)
check_symbol_exists(VERSAL_NET "bspconfig.h" IS_VERSAL_NET)
check_symbol_exists(VERSAL "bspconfig.h" IS_VERSAL)

if (IS_VERSAL2)
  set_property(GLOBAL PROPERTY SOC "VERSAL2")
elseif (IS_VERSAL_NET)
  set_property(GLOBAL PROPERTY SOC "VERSAL_NET")
elseif (IS_VERSAL)
  set_property(GLOBAL PROPERTY SOC "VERSAL")
else()
  set_property(GLOBAL PROPERTY SOC "ZYNQMP")
endif()

# Lopper plugin can generate linker meta data in below file
# This can define RSC_TABLE so include before
file (GLOB LINKER_METADATA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/*Example.cmake")
if (EXISTS ${LINKER_METADATA_FILE})
  set_property(GLOBAL PROPERTY LINKER_METADATA_FILE ${LINKER_METADATA_FILE})
  message("Libmetal: Linker will be configured using file: ${LINKER_METADATA_FILE}")
else()
  message(FATAL_ERROR "Libmetal: Linker gen failed")
endif()

foreach(_inc_path ${CMAKE_INCLUDE_PATH})
  collect (PROJECT_INC_DIRS "${_inc_path}")
endforeach()

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/")

set(ROLE "remote" CACHE STRING "")
set(DEMO "irq_shmem_demo" CACHE STRING "")
set(PROJECT_MACHINE "amd_rpu" CACHE STRING "")
string (TOLOWER ${CMAKE_SYSTEM_NAME} CMAKE_SYSTEM_NAME_LOWER)
set (PROJECT_SYSTEM "${CMAKE_SYSTEM_NAME_LOWER}" CACHE STRING "")
add_subdirectory (openamp-system-reference/examples/libmetal)
# vim: expandtab:ts=2:sw=2:smartindent
