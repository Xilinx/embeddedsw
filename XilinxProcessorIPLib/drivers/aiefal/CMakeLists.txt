###############################################################################
# Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################

cmake_minimum_required (VERSION 3.0)

list (APPEND CMAKE_MODULE_PATH
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/platforms")

project (xaiefal)

include (CheckIncludeFiles)
include (CheckCSourceCompiles)
include (ExternalProject)
include (collect)
include (options)
include (depends)

foreach(_inc_path ${CMAKE_INCLUDE_PATH})
  collect (PROJECT_INC_DIRS "${_inc_path}")
endforeach()

if (WITH_BUILD_XAIEDRV)
  if ("${PROJECT_SYSTEM}" STREQUAL "linux")
    set (_makefile Makefile.Linux)
  else()
    set (_makefile Makefile)
  endif ("${PROJECT_SYSTEM}" STREQUAL "linux")
  set (CC ${CMAKE_C_COMPILER})
  set (CFLAGS -Wall -Wextra ${AIEDRV_BACKEND})
  find_program(MAKE_EXE NAMES gmake nmake make)
  ExternalProject_Add(aienginev2
    URL "${CMAKE_CURRENT_SOURCE_DIR}/../aienginev2"
    SOURCE_DIR "${CMAKE_BINARY_DIR}/aienginev2-src"
    BUILD_IN_SOURCE true
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${MAKE_EXE} -C src -f ${_makefile} CC=${CC} "CFLAGS=-std=c99 -Wall -Wextra ${AIEDRV_BACKEND}"
    INSTALL_COMMAND ""
  )
collect (PROJECT_INC_DIRS ${CMAKE_BINARY_DIR}/aienginev2-src/include)
collect (PROJECT_LIB_DIRS ${CMAKE_BINARY_DIR}/aienginev2-src/src)
endif (WITH_BUILD_XAIEDRV)

enable_testing ()

add_subdirectory (src)

if (WITH_DOC)
  add_subdirectory (doc)
endif (WITH_DOC)

if (WITH_TESTS)
  add_subdirectory (tests)
endif (WITH_TESTS)

if (WITH_EXAMPLES)
  add_subdirectory (examples)
endif (WITH_EXAMPLES)

# vim: expandtab:ts=2:sw=2:smartindent
