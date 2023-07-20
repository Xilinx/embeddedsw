# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xnvm_efuse.h)
collect (PROJECT_LIB_HEADERS xnvm_efuse_error_plat.h)
collect (PROJECT_LIB_HEADERS xnvm_efuse_hw.h)

collect (PROJECT_LIB_SOURCES xnvm_efuse.c)

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze"))
  collect (PROJECT_LIB_HEADERS xnvm_cmd.h)
  collect (PROJECT_LIB_SOURCES xnvm_cmd.c)
  collect (PROJECT_LIB_HEADERS xnvm_efuse_ipihandler.h)
  collect (PROJECT_LIB_SOURCES xnvm_efuse_ipihandler.c)
endif()