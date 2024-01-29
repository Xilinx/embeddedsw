# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.3)

set(XPAR_XILPM_ENABLED " ")

string(COMPARE EQUAL "${DEVICE_ID}" "xcvp1902" IS_DEVICE_ID_PRESENT)
if (IS_DEVICE_ID_PRESENT)
  set(XCVP1902 " ")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xpm_config.h.in ${CMAKE_BINARY_DIR}/include/xpm_config.h)
