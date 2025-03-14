# Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(core)
if("${CMAKE_SUBMACHINE}" STREQUAL "VersalNet")
        add_subdirectory(versal_net)
elseif("${CMAKE_SUBMACHINE}" STREQUAL "VersalGen2")
        add_subdirectory(versal_aiepg2)
endif()
