# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_alghelper.h)
collect (PROJECT_LIB_SOURCES xsecure_alghelper.c)

# xsecure_libver implements the client-server library-info query, which is only
# meaningful on the platforms selected by the guard below. The guard matches the
# Versal-only build plus the machine that is neither ZynqMP, Versal nor Spartan
# UltraScalePlus. Gate collection to those platforms to avoid breaking builds
# where the feature does not apply.
if(_IS_VERSAL_ONLY OR (NOT _IS_ZYNQMP AND NOT _IS_VERSAL AND NOT _IS_SPARTANUPLUS))
  collect (PROJECT_LIB_HEADERS xsecure_libver.h)
  collect (PROJECT_LIB_SOURCES xsecure_libver.c)
endif()
