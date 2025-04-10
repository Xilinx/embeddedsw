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

option(XILASU_enable_perf_measurement "Enables/Disables performance measurement APIs." OFF)
if(XILASU_enable_perf_measurement)
    set(XASU_PERF_MEASUREMENT_ENABLE " ")
endif()

option(XILASU_aes_cm_mode_support "Enables/Disables counter measure mode for AES." ON)
if(XILASU_aes_cm_mode_support)
    set(XASU_AES_CM_ENABLE " ")
endif()

option(XILASU_ecc_cm_mode_support "Enables/Disables counter measure mode for ECC." ON)
if(XILASU_ecc_cm_mode_support)
    set(XASU_ECC_CM_ENABLE " ")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xasu_bsp_config.h.in ${CMAKE_BINARY_DIR}/include/xasu_bsp_config.h)
