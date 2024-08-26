# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(common)
if((
  ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa78") OR
  ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr52")
  ) AND
  ("${XILNVM_mode}" STREQUAL "client")
  )
  add_subdirectory(client)
else()
  add_subdirectory(server)
endif()
