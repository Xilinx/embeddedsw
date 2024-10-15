# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_sha_common.h)

if(("${CMAKE_MACHINE}" STREQUAL "VersalNet") OR
	("${CMAKE_MACHINE}" STREQUAL "Versal"))
  add_subdirectory(sha_pmx)
endif()

if(("${CMAKE_SUBMACHINE}" STREQUAL "VersalGen2") OR
  ("${CMAKE_MACHINE}" STREQUAL "SPARTANUP"))
  add_subdirectory(sha_pmxc)
endif()
