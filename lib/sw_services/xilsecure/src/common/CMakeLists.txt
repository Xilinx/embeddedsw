# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(all)
if(("${CMAKE_MACHINE}" STREQUAL "Versal") OR ("${CMAKE_MACHINE}" STREQUAL "VersalNet"))
  add_subdirectory(versal_common)
endif()