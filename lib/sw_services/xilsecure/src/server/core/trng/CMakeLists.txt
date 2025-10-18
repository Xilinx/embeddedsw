# Copyright (C) 2024-2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
if("${CMAKE_MACHINE}" STREQUAL "VersalNet")
  add_subdirectory(trngpsx)
endif()
if("${CMAKE_MACHINE}" STREQUAL "Versal")
  if("Versal_2VP" IN_LIST VARIANT)
    add_subdirectory(trngpsv)
  elseif("Versal_2VP_P" IN_LIST VARIANT)
    add_subdirectory(trngpsx)
  endif()
endif()
endif()
