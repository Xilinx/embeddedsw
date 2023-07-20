# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(common)

if("${XILSECURE_mode}" STREQUAL "server")
  add_subdirectory(server)
else()
  add_subdirectory(client)
endif()
