# Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
collect (PROJECT_LIB_SOURCES xil-crt0.S)
collect (PROJECT_LIB_SOURCES boot.S)
collect (PROJECT_LIB_SOURCES cpu_init.S)
if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "FreeRTOS")
collect (PROJECT_LIB_SOURCES  asm_vectors.S)
endif()
