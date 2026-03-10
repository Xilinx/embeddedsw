# Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM" OR ("Versal_2VP_P" IN_LIST VARIANT))
  add_subdirectory(sha_pmxc)
else()
  add_subdirectory(sha_pmx)
endif()
