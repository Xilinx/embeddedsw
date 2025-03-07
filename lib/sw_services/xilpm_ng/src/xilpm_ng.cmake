# Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

# Define XILPM library options with default values
option(XILPM_NG_EEMI_ENABLE "Enable EEMI support" ON)
option(XILPM_NG_SUBSYS_ENABLE "Enable Subsystem support" OFF)

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
  add_compile_definitions(XILPM_RUNTIME_BANNER="XilPm_Runtime: SUBSYS")
  add_compile_definitions(XILPM_RUNTIME_SUBSYS)
else()
  message(STATUS "Subsystem support is disabled.")
  if(XILPM_NG_EEMI_ENABLE)
    message(STATUS "EEMI support is enabled.")
    add_compile_definitions(XILPM_RUNTIME)
    add_compile_definitions(XILPM_RUNTIME_BANNER="XilPm_Runtime: EEMI")
    add_compile_definitions(XILPM_RUNTIME_EEMI)
  else()
    message(STATUS "EEMI support is disabled.")
  endif()
endif()
