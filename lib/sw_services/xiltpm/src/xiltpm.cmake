# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if (NOT ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze"))
  option(XILTPM_cache_disable "Enables/Disables Cache for xiltpm client library." ON)
  if(XILTPM_cache_disable)
    set(XTPM_CACHE_DISABLE " ")
  endif()
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xiltpm_bsp_config.h.in ${CMAKE_BINARY_DIR}/include/xiltpm_bsp_config.h)
