# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
add_subdirectory(common)
if("${XILPUF_Mode}" STREQUAL "server")
  add_subdirectory(server)
else()
  add_subdirectory(client)
endif()
