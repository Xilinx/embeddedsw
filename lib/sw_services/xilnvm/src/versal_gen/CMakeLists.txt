# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
add_subdirectory(common)
if("${CMAKE_MACHINE}" STREQUAL "Versal")
  add_subdirectory(versal)
elseif("${CMAKE_MACHINE}" STREQUAL "VersalNet")
  add_subdirectory(versal_net)
endif()
