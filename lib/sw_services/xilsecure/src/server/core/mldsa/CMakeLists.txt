# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_mldsa.h)
collect (PROJECT_LIB_HEADERS xsecure_mldsa_hw.h)
collect (PROJECT_LIB_SOURCES xsecure_mldsa.c)

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
    collect (PROJECT_LIB_HEADERS xsecure_mldsa_ipihandler.h)
    collect (PROJECT_LIB_SOURCES xsecure_mldsa_ipihandler.c)
endif()
