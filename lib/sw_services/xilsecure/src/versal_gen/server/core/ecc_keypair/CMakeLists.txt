# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_ellipticplat.h)
collect (PROJECT_LIB_SOURCES xsecure_ellipticplat.c)
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
collect (PROJECT_LIB_HEADERS xsecure_plat_elliptic_ipihandler.h)
collect (PROJECT_LIB_SOURCES xsecure_plat_elliptic_ipihandler.c)
endif()
