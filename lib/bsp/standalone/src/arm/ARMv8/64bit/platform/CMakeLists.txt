# Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
if("${CMAKE_MACHINE}" STREQUAL "Versal")
add_subdirectory(versal)
elseif(("${CMAKE_MACHINE}" STREQUAL "VersalNet") AND ("${CMAKE_SUBMACHINE}" STREQUAL "VersalNet"))
add_subdirectory(versal)
elseif(("${CMAKE_MACHINE}" STREQUAL "VersalNet") AND ("${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM"))
add_subdirectory(versal_2ve_2vm)
else()
add_subdirectory(ZynqMP)
endif()
