# Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_hmac.h)
collect (PROJECT_LIB_SOURCES xsecure_hmac.c)

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze" AND
   "${CMAKE_MACHINE}" STREQUAL "VersalNet")
    collect (PROJECT_LIB_HEADERS xsecure_hmac_ipihandler.h)
    collect (PROJECT_LIB_SOURCES xsecure_hmac_ipihandler.c)
endif()
