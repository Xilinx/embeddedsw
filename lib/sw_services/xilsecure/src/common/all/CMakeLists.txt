# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xsecure_ver.h)
if(("${XILSECURE_mode}" STREQUAL "server") OR ("${CMAKE_MACHINE}" STREQUAL "ZynqMP"))
  collect (PROJECT_LIB_HEADERS xsecure_rsa.h)
  collect (PROJECT_LIB_SOURCES xsecure_rsa.c)
endif()
