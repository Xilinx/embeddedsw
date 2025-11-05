# Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") OR
    ("${CMAKE_MACHINE}" STREQUAL "spartanuplus"))
  set(XILPUF_Mode "server")
elseif(("${CMAKE_MACHINE}" STREQUAL "VersalNet"))
  # For APU/RPU/PL microblaze cores in Versal_Net and Versal_2VE_2VM, mode is client.
  set(XILPUF_Mode "client")
else()
  set(XILPUF_Mode "client" CACHE STRING "Enables A72/R5/PL microblaze server and client mode support for XilPuf library")
  set_property(CACHE XILPUF_Mode PROPERTY STRINGS "client" "server")
endif()

option(XILPUF_cache_disable "Enables/Disables Cache for XilPuf client library." ON)
if(XILPUF_Mode STREQUAL "client")
  if(XILPUF_cache_disable)
    set(XPUF_CACHE_DISABLE " ")
  endif()
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xpuf_bsp_config.h.in ${CMAKE_BINARY_DIR}/include/xpuf_bsp_config.h)
