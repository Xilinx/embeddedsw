# Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${XILLOADER_mode}" STREQUAL "server")
  add_subdirectory(server)
endif()
