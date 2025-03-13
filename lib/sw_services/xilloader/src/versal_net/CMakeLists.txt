# Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${XILLOADER_mode}" STREQUAL "server")
  add_subdirectory(server)
elseif(("${XILLOADER_mode}" STREQUAL "client") AND ("${CMAKE_SUBMACHINE}" STREQUAL "VersalNet"))
  add_subdirectory(client)
endif()
