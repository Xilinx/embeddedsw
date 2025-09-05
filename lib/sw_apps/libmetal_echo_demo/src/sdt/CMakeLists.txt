# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
cmake_minimum_required (VERSION 3.0.2)
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

project (libmetal_amp_demod C)

string(REGEX MATCH "r5" r5 "${CMAKE_SYSTEM_PROCESSOR}")
if (r5)
  set (MACHINE "xlnx_r5" CACHE STRING "" FORCE)
  set (CROSS_PREFIX "armr5-none-eabi-" CACHE STRING "" FORCE)
  set (CMAKE_SYSTEM_PROCESSOR "arm" CACHE STRING "" FORCE)
  set (PROJECT_MACHINE "xlnx_r5" CACHE STRING "" FORCE)
  add_definitions( -DWITH_DOC=OFF )
  add_definitions( -DWITH_EXAMPLES=ON )
  add_definitions( -DXPAR_SCUGIC_0_DEVICE_ID=XPAR_SCUGIC_SINGLE_DEVICE_ID )
  add_definitions( -DXPAR_SCUGIC_0_DIST_BASEADDR=XPAR_SCUGIC_DIST_BASEADDR )
  add_definitions( -DSDT=1 )

  set( DEMO_LINK_FLAGS " -lxil -lxilstandalone -lxiltimer -lgcc" )
  if("${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM")
    set( DEMO_LINK_FLAGS "${DEMO_LINK_FLAGS} -lxilpm_ng" )
  else()
    set( DEMO_LINK_FLAGS "${DEMO_LINK_FLAGS} -lxilpm" )
  endif()

  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -specs=${CMAKE_SPECS_FILE} " )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CMAKE_INCLUDE_PATH} " )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${proc_extra_compiler_flags}" )
endif()

set (LIBMETAL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libmetal")

list (APPEND CMAKE_MODULE_PATH
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/platforms")

include (libmetal/cmake/syscheck.cmake)
include (CheckIncludeFiles)
include (CheckCSourceCompiles)
include (libmetal/cmake/collect.cmake)
include (libmetal/cmake/options.cmake)
set_property (GLOBAL PROPERTY "PROJECT_EC_FLAGS" -Wall -Wextra)
set_property (GLOBAL PROPERTY HAS_SYSTEM_DT ON)

foreach(_inc_path ${CMAKE_INCLUDE_PATH})
  collect (PROJECT_INC_DIRS "${_inc_path}")
endforeach()

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/")
add_subdirectory (libmetal/examples)

# vim: expandtab:ts=2:sw=2:smartindent
