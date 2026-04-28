# Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_ellipticclient.h)
collect (PROJECT_LIB_SOURCES xsecure_ellipticclient.c)

if(NOT "Versal_2VP" IN_LIST VARIANT AND NOT "Versal_2VP_P" IN_LIST VARIANT)
  collect (PROJECT_LIB_HEADERS xsecure_ellipticalginfo.h)
  collect (PROJECT_LIB_SOURCES xsecure_ellipticalginfo.c)
endif()
