# Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.14.7)

list(LENGTH SPI_NUM_DRIVER_INSTANCES _len)
list(LENGTH OSPIPSV_NUM_DRIVER_INSTANCES _ospi_len)

if((${_len} EQUAL 0) AND ((${_ospi_len} EQUAL 0) AND ("${CMAKE_MACHINE}" STREQUAL "spartanuplus")))
	message(FATAL_ERROR "This application requires either AXI Quad SPI or OSPI in the hardware.")
Endif()
