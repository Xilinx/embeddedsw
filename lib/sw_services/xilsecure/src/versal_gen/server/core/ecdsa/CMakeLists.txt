# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_ellipticcrvs.h)
collect (PROJECT_LIB_HEADERS xsecure_ecdsa_rsa_hw.h)
collect (PROJECT_LIB_HEADERS xsecure_elliptic.h)
collect (PROJECT_LIB_SOURCES xsecure_elliptic.c)
collect (PROJECT_LIB_SOURCES xsecure_ellipticcrvs.c)
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
collect (PROJECT_LIB_SOURCES xsecure_elliptic_ipihandler.c)
collect (PROJECT_LIB_HEADERS xsecure_elliptic_ipihandler.h)
endif()
