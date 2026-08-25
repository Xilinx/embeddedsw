# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM")
  set(XNVM_EFUSE_PLATFORM versal_2ve2vm)
else()
  set(XNVM_EFUSE_PLATFORM versal_net)
endif()

if("${XILNVM_mode}" STREQUAL "client")
  add_subdirectory(common)
  add_subdirectory(client)
else()
  add_subdirectory(common)
  add_subdirectory(server)
endif()
