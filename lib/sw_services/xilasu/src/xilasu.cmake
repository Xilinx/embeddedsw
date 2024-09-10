# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze_riscv")
  set(XILASU_Mode "server")
else()
  set(XILASU_Mode "client")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xasu_bsp_config.h.in ${CMAKE_BINARY_DIR}/include/xasu_bsp_config.h)
