# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.14.7)

add_subdirectory(common)

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblazeel") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze"))
	add_subdirectory(server)
elseif(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") OR
	("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5"))
	add_subdirectory(client)
else()
	message(FATAL_ERROR "Wrong system processor ${CMAKE_SYSTEM_PROCESSOR}")
endif()
