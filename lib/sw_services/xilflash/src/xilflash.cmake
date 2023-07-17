# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.3)
option(XILFLASH_enable_intel "Enables support for Intel family devices" ON)
option(XILFLASH_enable_amd "Enables support for AMD family devices" OFF)
if (${XILFLASH_enable_intel})
	set(XPAR_XFL_DEVICE_FAMILY_INTEL " ")
endif()
if (${XILFLASH_enable_amd})
	set(XPAR_XFL_DEVICE_FAMILY_AMD " ")
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xilflash_config.h.in ${CMAKE_BINARY_DIR}/include/xilflash_config.h)
