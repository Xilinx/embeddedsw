# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(common)

if(("${XILSECURE_mode}" STREQUAL "server") AND ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze"))
  add_subdirectory(server)
else()
  add_subdirectory(client)
endif()
