# Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(common)
if("${XILPLMI_mode}" STREQUAL "server")
  add_subdirectory(server)
endif()
