# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_lms.h)
collect (PROJECT_LIB_HEADERS xsecure_lms_core.h)
collect (PROJECT_LIB_HEADERS xsecure_lms_hss.h)
collect (PROJECT_LIB_HEADERS xsecure_lms_ots.h)
collect (PROJECT_LIB_SOURCES xsecure_lms.c)
collect (PROJECT_LIB_SOURCES xsecure_lms_core.c)
collect (PROJECT_LIB_SOURCES xsecure_lms_ots.c)

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") AND
   ("${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM"))
  collect (PROJECT_LIB_HEADERS xsecure_lms_ipihandler.h)
  collect (PROJECT_LIB_SOURCES xsecure_lms_ipihandler.c)
endif()
