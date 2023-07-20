# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(common)

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") OR ("${XILSECURE_mode}" STREQUAL "server"))
  add_subdirectory(server)
endif()
