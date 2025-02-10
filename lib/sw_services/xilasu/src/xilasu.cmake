# Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze_riscv")
  set(XILASU_Mode "server")
else()
  set(XILASU_Mode "client")
endif()

option(XILASU_trng_enable_drbg_mode "Enables/Disables DRBG mode for TRNG." OFF)
if(XILASU_trng_enable_drbg_mode)
    set(XASU_TRNG_ENABLE_DRBG_MODE " ")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xasu_bsp_config.h.in ${CMAKE_BINARY_DIR}/include/xasu_bsp_config.h)
