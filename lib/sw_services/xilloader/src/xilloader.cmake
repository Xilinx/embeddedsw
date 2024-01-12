# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  set(XILLOADER_mode "server")
else()
  # For soft microblaze and APU/RPU cores, mode is client.
  set(XILLOADER_mode "client")
endif()

option(XILLOADER_cache_disable "Enables/Disables Cache for XilLoader client library." ON)
if(XILLOADER_mode STREQUAL "client")
  if(XILLOADER_cache_disable)
    set(XLOADER_CACHE_DISABLE " ")
  endif()
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xilloader_bsp_config.h.in ${CMAKE_BINARY_DIR}/include/xilloader_bsp_config.h)