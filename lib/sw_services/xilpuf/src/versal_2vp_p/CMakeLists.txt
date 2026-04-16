# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${XILPUF_Mode}" STREQUAL "server")
  add_subdirectory(server)
endif()
