# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if(NOT "${CMAKE_SUBMACHINE}" STREQUAL "VersalGen2")
	collect (PROJECT_LIB_HEADERS xloader_plat.h)
	collect (PROJECT_LIB_HEADERS xloader_plat_secure.h)
endif()

if(NOT "${CMAKE_SUBMACHINE}" STREQUAL "VersalGen2")
	collect (PROJECT_LIB_SOURCES xloader_plat.c)
	collect (PROJECT_LIB_SOURCES xloader_plat_secure.c)
	collect (PROJECT_LIB_SOURCES xloader_ddr_init.c)
endif()