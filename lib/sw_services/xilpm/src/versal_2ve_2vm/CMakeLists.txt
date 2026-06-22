# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

# versal_2ve_2vm hosts the platform-specific client code for Versal_2VE_2VM:
# the APU/RPU init (pm_client). The shared EEMI client code
# (pm_api_sys, pm_callbacks) comes from versal_common/client/, and the
# platform headers (xpm_defs.h, xpm_err.h, xpm_nodeid.h, ...) are provided by
# the xilpm_ng library (xilpm_ng/src/xilpm_common) via the shared BSP include
# path - so a Versal_2VE_2VM client BSP must include both xilpm and xilpm_ng.
#
# Server code for versal_2ve_2vm lives in the xilpm_ng library, so this
# directory currently exposes a client/ subtree only.

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa78") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr52"))
	add_subdirectory(client)
else()
	message(FATAL_ERROR
		"versal_2ve_2vm currently only supports cortexa78 / cortexr52 client builds (got ${CMAKE_SYSTEM_PROCESSOR})")
endif()
