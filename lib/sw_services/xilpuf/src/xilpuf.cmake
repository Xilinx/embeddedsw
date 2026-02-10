# Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

# Initialize platform and processor flags
set(_IS_PLM_MICROBLAZE FALSE)
set(_IS_VERSALNET FALSE)
set(_IS_SPARTANUPLUS FALSE)

# Evaluate processor types
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  set(_IS_PLM_MICROBLAZE TRUE)
endif()

# Evaluate platform types
if("${CMAKE_MACHINE}" STREQUAL "VersalNet")
  set(_IS_VERSALNET TRUE)
endif()

if("${CMAKE_MACHINE}" STREQUAL "spartanuplus")
  set(_IS_SPARTANUPLUS TRUE)
endif()

# =============================================================================
# Mode Configuration
# =============================================================================
if(_IS_PLM_MICROBLAZE OR _IS_SPARTANUPLUS)
  set(XILPUF_Mode "server")
elseif(_IS_VERSALNET)
  # For APU/RPU/PL microblaze cores in Versal_Net and Versal_2VE_2VM, mode is client.
  set(XILPUF_Mode "client")
else()
  set(XILPUF_Mode "client" CACHE STRING "Enables A72/R5/PL microblaze server and client mode support for XilPuf library")
  set_property(CACHE XILPUF_Mode PROPERTY STRINGS "client" "server")
endif()

# =============================================================================
# Client Configuration
# =============================================================================
if((NOT _IS_PLM_MICROBLAZE) AND (NOT _IS_SPARTANUPLUS))
  option(XILPUF_cache_disable "Enables/Disables Cache for XilPuf client library." ON)
  if(XILPUF_cache_disable AND XILPUF_Mode STREQUAL "client")
    set(XPUF_CACHE_DISABLE " ")
  endif()
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xpuf_bsp_config.h.in ${CMAKE_BINARY_DIR}/include/xpuf_bsp_config.h)
