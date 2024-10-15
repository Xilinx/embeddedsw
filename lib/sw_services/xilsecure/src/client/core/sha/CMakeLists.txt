# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${CMAKE_SUBMACHINE}" STREQUAL "VersalGen2")
  add_subdirectory(sha_pmxc)
else()
  add_subdirectory(sha_pmx)
endif()
