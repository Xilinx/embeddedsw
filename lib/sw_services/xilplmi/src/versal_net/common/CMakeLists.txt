# Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
if((NOT "${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM") AND
   (NOT "Versal_2VP_P" IN_LIST VARIANT))
collect (PROJECT_LIB_HEADERS xplmi_error_node.h)
endif()