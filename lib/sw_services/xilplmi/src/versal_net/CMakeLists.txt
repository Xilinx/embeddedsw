# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(common)
if("${XILPLMI_mode}" STREQUAL "client")
  add_subdirectory(client)
else()
  add_subdirectory(server)
endif()
