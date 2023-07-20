# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xnvm_common_defs.h)
collect (PROJECT_LIB_HEADERS xnvm_efuse_error.h)
collect (PROJECT_LIB_HEADERS xnvm_temp.h)
collect (PROJECT_LIB_HEADERS xnvm_utils.h)
collect (PROJECT_LIB_HEADERS xnvm_ver.h)

collect (PROJECT_LIB_SOURCES xnvm_utils.c)

if((
  ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") OR
  ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5") OR
  ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze")
  ) AND
  ("${XILNVM_mode}" STREQUAL "client")
  )
  add_subdirectory(client)
else()
  add_subdirectory(server)
endif()