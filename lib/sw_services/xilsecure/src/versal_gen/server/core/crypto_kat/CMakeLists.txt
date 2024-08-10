# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_kat.h)
collect (PROJECT_LIB_SOURCES xsecure_kat.c)
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
collect (PROJECT_LIB_HEADERS xsecure_kat_ipihandler.h)
collect (PROJECT_LIB_SOURCES xsecure_kat_ipihandler.c)
endif()
