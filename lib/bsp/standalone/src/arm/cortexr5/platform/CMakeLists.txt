# Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
if("${CMAKE_MACHINE}" STREQUAL "Versal")
add_subdirectory(versal)
add_subdirectory(CortexR5)
elseif ("${CMAKE_MACHINE}" STREQUAL "VersalNet")
add_subdirectory(versal-net)
add_subdirectory(CortexR52)
else()
add_subdirectory(ZynqMP)
add_subdirectory(CortexR5)
endif()
