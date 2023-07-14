# Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
if(("${CMAKE_MACHINE}" STREQUAL "Versal")
	OR ("${CMAKE_MACHINE}" STREQUAL "VersalNet"))
add_subdirectory(versal)
else()
add_subdirectory(ZynqMP)
endif()
