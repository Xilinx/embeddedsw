# Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
if(NOT ("${proc_compiler}" STREQUAL "armclang"))
  add_subdirectory(gcc)
endif()
if("${CMAKE_MACHINE}" STREQUAL "Versal")
collect (PROJECT_LIB_HEADERS xparameters_ps.h)
endif()
