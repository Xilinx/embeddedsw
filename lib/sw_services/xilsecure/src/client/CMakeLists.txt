# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(core)
if(("${CMAKE_SUBMACHINE}" STREQUAL "VersalNet"))
  add_subdirectory(versal_net)
endif()
