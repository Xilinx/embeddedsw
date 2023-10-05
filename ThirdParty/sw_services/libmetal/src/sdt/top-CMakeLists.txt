# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
cmake_minimum_required (VERSION 3.0.2)
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

set (LIBMETAL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

list (APPEND CMAKE_MODULE_PATH
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/platforms")

string(REGEX MATCH "a9" a9 "${CMAKE_SYSTEM_PROCESSOR}")
if (a9)
  set(MACHINE "zynq7" CACHE STRING "" FORCE)
  set (CMAKE_SYSTEM_PROCESSOR "arm" CACHE STRING "" FORCE)
endif()

string(REGEX MATCH "a53" a53 "${CMAKE_SYSTEM_PROCESSOR}")
if (a53)
  set(MACHINE "zynqmp_a53" CACHE STRING "" FORCE)
endif()

string(REGEX MATCH "a72" a72 "${CMAKE_SYSTEM_PROCESSOR}")
if (a72)
  set(MACHINE "zynqmp_a72" CACHE STRING "" FORCE)
endif()

string(REGEX MATCH "a78" a78 "${CMAKE_SYSTEM_PROCESSOR}")
if (a78)
  set(MACHINE "zynqmp_a78" CACHE STRING "" FORCE)
endif()

string(REGEX MATCH "microblaze" mb "${CMAKE_SYSTEM_PROCESSOR}")
string(REGEX MATCH "pmu" pmu "${CMAKE_SYSTEM_PROCESSOR}")
string(REGEX MATCH "pmc" pmc "${CMAKE_SYSTEM_PROCESSOR}")
if (mb OR pmu OR pmc)
  set(MACHINE "microblaze_generic" CACHE STRING "" FORCE)
  set (CROSS_PREFIX "mb-" CACHE STRING "" FORCE)
  set (CMAKE_SYSTEM_PROCESSOR "microblaze" CACHE STRING "" FORCE)
endif()

if (a53 OR a72 OR a78)
  set (CROSS_PREFIX "aarch64-none-elf-" CACHE STRING "" FORCE)
  set (CMAKE_SYSTEM_PROCESSOR "aarch64" CACHE STRING "" FORCE)
endif()

string(REGEX MATCH "r5" r5 "${CMAKE_SYSTEM_PROCESSOR}")
if (r5)
  set(MACHINE "zynqmp_r5" CACHE STRING "" FORCE)
  set (CROSS_PREFIX "armr5-none-eabi-" CACHE STRING "" FORCE)
  set (CMAKE_SYSTEM_PROCESSOR "arm" CACHE STRING "" FORCE)
endif()

set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/build-libmetal" CACHE STRING "" FORCE)

add_definitions(-DXPAR_SCUGIC_0_DEVICE_ID=XPAR_SCUGIC_SINGLE_DEVICE_ID)
add_definitions(-DXPAR_SCUGIC_0_DIST_BASEADDR=XPAR_SCUGIC_DIST_BASEADDR)

include (syscheck)
project (metal C)

include (CheckIncludeFiles)
include (CheckCSourceCompiles)
include (collect)
include (options)
set_property (GLOBAL PROPERTY "PROJECT_EC_FLAGS" -Wall  -Wextra)
include (depends)

foreach(_inc_path ${CMAKE_INCLUDE_PATH})
  collect (PROJECT_INC_DIRS "${_inc_path}")
endforeach()

enable_testing ()
add_subdirectory (lib)
