# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_SOURCES xpuf.c)
collect (PROJECT_LIB_SOURCES xpuf_plat.c)
collect (PROJECT_LIB_HEADERS xpuf_plat.h)
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  collect (PROJECT_LIB_SOURCES xpuf_ipihandler.c)
endif()
