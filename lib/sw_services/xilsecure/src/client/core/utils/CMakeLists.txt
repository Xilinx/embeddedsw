# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if(NOT "Versal_2VP" IN_LIST VARIANT AND NOT "Versal_2VP_P" IN_LIST VARIANT)
  collect (PROJECT_LIB_HEADERS xsecure_alghelper.h)
  collect (PROJECT_LIB_SOURCES xsecure_alghelper.c)
endif()
