# Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_cmd.h)
collect (PROJECT_LIB_HEADERS xsecure_cryptochk.h)
collect (PROJECT_LIB_HEADERS xsecure_init.h)
collect (PROJECT_LIB_SOURCES xsecure_cryptochk.c)
collect (PROJECT_LIB_SOURCES xsecure_init.c)
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
collect (PROJECT_LIB_SOURCES xsecure_resourcehandling.c)
collect (PROJECT_LIB_HEADERS xsecure_resourcehandling.h)
endif()
