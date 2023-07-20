# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xpuf.h)
collect (PROJECT_LIB_HEADERS xpuf_hw.h)
collect (PROJECT_LIB_SOURCES xpuf.c)
collect (PROJECT_LIB_SOURCES xpuf_plat.c)
collect (PROJECT_LIB_HEADERS xpuf_plat.h)
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  collect (PROJECT_LIB_SOURCES xpuf_ipihandler.c)
  collect (PROJECT_LIB_HEADERS xpuf_ipihandler.h)
  collect (PROJECT_LIB_SOURCES xpuf_cmd.c)
  collect (PROJECT_LIB_HEADERS xpuf_cmd.h)
  collect (PROJECT_LIB_SOURCES xpuf_init.c)
  collect (PROJECT_LIB_HEADERS xpuf_init.h)
endif()
