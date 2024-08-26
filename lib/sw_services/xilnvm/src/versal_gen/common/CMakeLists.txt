# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xnvm_common_defs.h)
collect (PROJECT_LIB_HEADERS xnvm_efuse_common_hw.h)
collect (PROJECT_LIB_HEADERS xnvm_efuse_error.h)
collect (PROJECT_LIB_HEADERS xnvm_utils.h)
collect (PROJECT_LIB_HEADERS xnvm_ver.h)

collect (PROJECT_LIB_SOURCES xnvm_utils.c)

if("${XILNVM_mode}" STREQUAL "client")
  add_subdirectory(client)
else()
  add_subdirectory(server)
endif()
