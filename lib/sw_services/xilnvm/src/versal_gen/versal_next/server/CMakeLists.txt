# Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(efuse)

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  collect (PROJECT_LIB_HEADERS xnvm_bbram_cdohandler.h)
  collect (PROJECT_LIB_SOURCES xnvm_bbram_cdohandler.c)
  collect (PROJECT_LIB_HEADERS xnvm_cmd.h)
  collect (PROJECT_LIB_SOURCES xnvm_cmd.c)
endif()
