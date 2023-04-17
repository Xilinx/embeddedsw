# Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
collect (PROJECT_LIB_SOURCES xil-crt0.S)
collect (PROJECT_LIB_SOURCES boot.S)
collect (PROJECT_LIB_SOURCES initialise_monitor_handles.c)
if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "FreeRTOS")
collect (PROJECT_LIB_SOURCES  asm_vectors.S)
endif()
