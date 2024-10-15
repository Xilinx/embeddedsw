# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_aes_core_hw.h)
collect (PROJECT_LIB_HEADERS xsecure_aes.h)
collect (PROJECT_LIB_SOURCES xsecure_aes.c)
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
collect (PROJECT_LIB_SOURCES xsecure_aes_ipihandler.c)
collect (PROJECT_LIB_HEADERS xsecure_aes_ipihandler.h)
endif()
