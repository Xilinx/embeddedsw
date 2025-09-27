# Copyright (C) 2024 - 2025, Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xocp.h)
collect (PROJECT_LIB_HEADERS xocp_cmd.h)
collect (PROJECT_LIB_HEADERS xocp_hw.h)
collect (PROJECT_LIB_HEADERS xocp_init.h)
collect (PROJECT_LIB_HEADERS xocp_ipihandler.h)
collect (PROJECT_LIB_HEADERS xocp_keymgmt.h)

collect (PROJECT_LIB_SOURCES xocp.c)
collect (PROJECT_LIB_SOURCES xocp_cmd.c)
collect (PROJECT_LIB_SOURCES xocp_init.c)
collect (PROJECT_LIB_SOURCES xocp_ipihandler.c)
collect (PROJECT_LIB_SOURCES xocp_keymgmt.c)

if(("${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM"))
	add_subdirectory(versal_2ve_2vm)
elseif("${CMAKE_MACHINE}" STREQUAL "VersalNet")
	add_subdirectory(versal_net)
elseif("${CMAKE_MACHINE}" STREQUAL "Versal_2vp")
	add_subdirectory(versal_2vp)
endif()
