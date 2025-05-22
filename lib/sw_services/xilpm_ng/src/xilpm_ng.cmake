# Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

# Define XILPM library options with default values
option(XILPM_NG_EEMI_ENABLE "Enable EEMI support" ON)
option(XILPM_NG_SUBSYS_ENABLE "Enable Subsystem support" ON)

if(XILPM_NG_SUBSYS_ENABLE)
    if(NOT XILPM_NG_EEMI_ENABLE)
        set(XILPM_NG_EEMI_ENABLE ON CACHE BOOL "Enable EEMI support" FORCE)
        message(STATUS "XILPM_NG_SUBSYS_ENABLE is ON; setting XILPM_NG_EEMI_ENABLE to ON")
    endif()
endif()

# Definitions for XILPM library based on options Enable Subsystem support (if
# enabled) This includes EEMI + Subsystem. Otherwise, enable EEMI support
if(XILPM_NG_SUBSYS_ENABLE)
  message(STATUS "Subsystem support is enabled.")
  add_compile_definitions(XILPM_RUNTIME)
  add_compile_definitions(XILPM_RUNTIME_BANNER=2)
  add_compile_definitions(XILPM_RUNTIME_SUBSYS)
else()
  message(STATUS "Subsystem support is disabled.")
  if(XILPM_NG_EEMI_ENABLE)
    message(STATUS "EEMI support is enabled.")
    add_compile_definitions(XILPM_RUNTIME)
    add_compile_definitions(XILPM_RUNTIME_BANNER=1)
    add_compile_definitions(XILPM_RUNTIME_EEMI)
  else()
    message(STATUS "EEMI support is disabled.")
  endif()
endif()

# If the part used in BSP is one of the following speed-grades, it can be subject to
# VID adjustment.  Depending on to which HW SKU the part belongs, voltage on different
# power rails needs to be adjusted.  The VID_SPGD_INDEX macro setting identifies to
# which HW SKU the part belongs.
if ("${SPEED_GRADE}" STREQUAL "1LHP")
  set(VID_SPGD_INDEX "1")
elseif ("${SPEED_GRADE}" STREQUAL "1LXP")
  set(VID_SPGD_INDEX "2")
elseif ("${SPEED_GRADE}" STREQUAL "1LXHP")
  set(VID_SPGD_INDEX "3")
elseif ("${SPEED_GRADE}" STREQUAL "1LHQ" OR "${SPEED_GRADE}" STREQUAL "1LHJ")
  set(VID_SPGD_INDEX "4")
elseif ("${SPEED_GRADE}" STREQUAL "1LXHQ" OR "${SPEED_GRADE}" STREQUAL "1LXHJ")
  set(VID_SPGD_INDEX "5")
elseif ("${SPEED_GRADE}" STREQUAL "2LHP")
  set(VID_SPGD_INDEX "6")
elseif ("${SPEED_GRADE}" STREQUAL "2LLHI" OR "${SPEED_GRADE}" STREQUAL "2LLHJ")
  set(VID_SPGD_INDEX "7")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xpm_config.h.in ${CMAKE_BINARY_DIR}/include/xpm_config.h)
