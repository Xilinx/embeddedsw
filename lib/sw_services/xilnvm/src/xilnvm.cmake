# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

option(XILNVM_use_puf_hd_as_user_efuse "Enables API's to use PUF Helper data efuses as user efuses." OFF)
if(XILNVM_use_puf_hd_as_user_efuse)
  set(XNVM_ACCESS_PUF_USER_DATA " ")
endif()

option(XILNVM_cache_disable "Enables/Disables Cache for XilNvm client library." ON)
if(XILNVM_cache_disable)
  if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") OR
     ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5") OR
     ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa78") OR
     ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr52") OR
     ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze"))
     set(XNVM_CACHE_DISABLE " ")
  endif()
endif()

option(XILNVM_en_add_ppks "Enables or Disables additional PPKs" OFF)
if (XILNVM_en_add_ppks OR ("${DEVICE_ID}" STREQUAL "xcvp1052"))
  set(PLM_EN_ADD_PPKS " ")
endif()

set(XILNVM_mode "client" CACHE STRING "Enables A72/R5 server and client mode support for XilNvm library for Versal")
set_property(CACHE XILNVM_mode PROPERTY STRINGS "client" "server")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xilnvm_bsp_config.h.in ${CMAKE_BINARY_DIR}/include/xilnvm_bsp_config.h)
