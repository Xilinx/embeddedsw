# Copyright (C) 2024-2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
if("${CMAKE_MACHINE}" STREQUAL "VersalNet")
  add_subdirectory(trngpsx)
endif()
endif()
