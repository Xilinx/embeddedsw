# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if (NOT ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze"))
  option(XILOCP_cache_disable "Enables/Disables Cache for xilocp client library." ON)
  if(XILOCP_cache_disable)
    set(XOCP_CACHE_DISABLE " ")
  endif()
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xilocp_bsp_config.h.in ${CMAKE_BINARY_DIR}/include/xilocp_bsp_config.h)
