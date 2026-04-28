# Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_shaclient.h)
collect (PROJECT_LIB_SOURCES xsecure_shaclient.c)

if(NOT "Versal_2VP_P" IN_LIST VARIANT)
	collect (PROJECT_LIB_HEADERS xsecure_shaalginfo.h)
	collect (PROJECT_LIB_SOURCES xsecure_shaalginfo.c)
endif()
