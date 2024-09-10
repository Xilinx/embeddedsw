# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS Ecdsa.h)
collect (PROJECT_LIB_HEADERS Rsa.h)

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze_riscv")
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilasu.a ${CMAKE_LIBRARY_PATH}/libxilasu_mbv.a)
endif()