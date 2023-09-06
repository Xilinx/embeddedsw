# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.14.7)

list(LENGTH SPI_NUM_DRIVER_INSTANCES _len)
if(${_len} EQUAL 0)
    message(FATAL_ERROR "This application requires a AXI Quad SPI in the hardware.")
endif()