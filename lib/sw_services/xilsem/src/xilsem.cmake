# Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.3)

option(XILSEM_ssit_en "Defines if XilSEM SSIT client is enabled(1) or disabled (0)." OFF)

if("${CMAKE_MACHINE}" STREQUAL "Versal")
	if(${XILSEM_ssit_en})
		set(XILSEM_ENABLE_SSIT " ")
	endif()
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xilsem_config.h.in ${CMAKE_BINARY_DIR}/include/xilsem_config.h)
