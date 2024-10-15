# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_rsa.h)
collect (PROJECT_LIB_HEADERS xsecure_rsa_core.h)
collect (PROJECT_LIB_SOURCES xsecure_rsa_core.c)
collect (PROJECT_LIB_SOURCES xsecure_rsa.c)
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
collect (PROJECT_LIB_SOURCES xsecure_rsa_ipihandler.c)
collect (PROJECT_LIB_HEADERS xsecure_rsa_ipihandler.h)
endif()
if(("${CMAKE_MACHINE}" STREQUAL "VersalNet"))
  add_subdirectory(rsa_qmode)
endif()
