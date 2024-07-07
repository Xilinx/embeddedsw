# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if(("${CMAKE_MACHINE}" STREQUAL "VersalNet") OR
	("${CMAKE_MACHINE}" STREQUAL "Versal"))
  add_subdirectory(sha_pmx)
endif()
