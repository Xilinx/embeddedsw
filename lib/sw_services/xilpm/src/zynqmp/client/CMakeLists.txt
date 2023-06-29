# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.14.7)

add_subdirectory(common)

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53")
	OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53-32")
	OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64"))
	add_subdirectory(apu)
elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5")
	add_subdirectory(rpu)
else()
	message(FATAL_ERROR "Wrong system processor ${CMAKE_SYSTEM_PROCESSOR}")
endif()
