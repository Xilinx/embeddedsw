# Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_plat_elliptic_client.h)
collect (PROJECT_LIB_SOURCES xsecure_plat_elliptic_client.c)

if(NOT "Versal_2VP" IN_LIST VARIANT AND NOT "Versal_2VP_P" IN_LIST VARIANT)
	collect (PROJECT_LIB_HEADERS xsecure_ecdhalginfo.h)
	collect (PROJECT_LIB_SOURCES xsecure_ecdhalginfo.c)
endif()
