# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

set(XILPUF_Mode "client" CACHE STRING "Enables A72/R5 server and client mode support for XilPuf library")
set_property(CACHE XILPUF_Mode PROPERTY STRINGS "client" "server")

option(XILPUF_cache_disable "Enables/Disables Cache for XilPuf client library." OFF)
if(XILPUF_cache_disable)
  if("${CMAKE_MACHINE}" STREQUAL "Versal")
    if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") OR
       ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5"))
      set(XPUF_CACHE_DISABLE " ")
    endif()
  endif()
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xpuf_bsp_config.h.in ${CMAKE_BINARY_DIR}/include/xpuf_bsp_config.h)
