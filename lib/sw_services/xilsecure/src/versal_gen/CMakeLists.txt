# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if (NOT("${CMAKE_MACHINE}" STREQUAL "SPARTANUP"))
  add_subdirectory(common)
endif()
if(("${XILSECURE_mode}" STREQUAL "server") OR ("${CMAKE_MACHINE}" STREQUAL "SPARTANUP"))
  add_subdirectory(server)
else()
  add_subdirectory(client)
endif()
