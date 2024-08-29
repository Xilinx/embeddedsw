# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if(NOT "${CMAKE_SUBMACHINE}" STREQUAL "VersalGen2")
collect (PROJECT_LIB_HEADERS xilpdi_plat.h)
endif()
