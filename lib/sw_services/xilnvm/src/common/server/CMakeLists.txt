# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xnvm_bbram.h)
collect (PROJECT_LIB_HEADERS xnvm_bbram_hw.h)
collect (PROJECT_LIB_HEADERS xnvm_efuse_common.h)
collect (PROJECT_LIB_HEADERS xnvm_efuse_common_hw.h)

collect (PROJECT_LIB_SOURCES xnvm_bbram.c)
collect (PROJECT_LIB_SOURCES xnvm_efuse_common.c)

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze"))
  collect (PROJECT_LIB_SOURCES xnvm_init.c)
  collect (PROJECT_LIB_HEADERS xnvm_init.h)
  collect (PROJECT_LIB_HEADERS xnvm_bbram_common_cdohandler.h)
  collect (PROJECT_LIB_SOURCES xnvm_bbram_common_cdohandler.c)
endif()