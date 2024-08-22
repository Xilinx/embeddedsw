# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(core)
if(("${CMAKE_MACHINE}" STREQUAL "Versal"))
  add_subdirectory(versal)
endif()
if(("${CMAKE_MACHINE}" STREQUAL "VersalNet"))
  add_subdirectory(versal_net)
endif()
if(("${CMAKE_MACHINE}" STREQUAL "VersalGen2"))
  add_subdirectory(versal_aiepg2)
endif()
