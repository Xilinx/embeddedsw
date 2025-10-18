# Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_sha_common.h)

if(("${CMAKE_MACHINE}" STREQUAL "VersalNet") OR
	("${CMAKE_MACHINE}" STREQUAL "Versal"))
  if("Versal_2VP_P" IN_LIST VARIANT)
    add_subdirectory(sha_pmxc)
  else()
    add_subdirectory(sha_pmx)
  endif()
endif()

if(("${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM") OR
  ("${CMAKE_MACHINE}" STREQUAL "spartanuplus"))
  add_subdirectory(sha_pmxc)
endif()
