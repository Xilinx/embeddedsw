# Copyright (C) 2024-2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(core)
if(("${CMAKE_MACHINE}" STREQUAL "Versal"))
  add_subdirectory(versal)
elseif(("${CMAKE_SUBMACHINE}" STREQUAL "VersalNet"))
  add_subdirectory(versal_net)
elseif(("${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM"))
  add_subdirectory(versal_2ve_2vm)
endif()
