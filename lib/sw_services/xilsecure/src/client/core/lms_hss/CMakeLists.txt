# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if(NOT "Versal_2VP_P" IN_LIST VARIANT)
  collect (PROJECT_LIB_HEADERS xsecure_lmsalginfo.h)
  collect (PROJECT_LIB_SOURCES xsecure_lmsalginfo.c)
  collect (PROJECT_LIB_HEADERS xsecure_lmsclient.h)
  collect (PROJECT_LIB_SOURCES xsecure_lmsclient.c)
endif()
