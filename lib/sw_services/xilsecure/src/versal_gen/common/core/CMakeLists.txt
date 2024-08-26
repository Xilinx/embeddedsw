# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_defs.h)
if("${XILSECURE_mode}" STREQUAL "server" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
    collect (PROJECT_LIB_SOURCES xsecure_core.c)
    collect (PROJECT_LIB_HEADERS xsecure_core.h)
endif()