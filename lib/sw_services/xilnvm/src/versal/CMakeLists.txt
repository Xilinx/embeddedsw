# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(common)
if((
  ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") OR
  ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5") OR
  ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze")
  ) AND
  ("${XILNVM_mode}" STREQUAL "client")
  )
  add_subdirectory(client)
else()
  add_subdirectory(server)
endif()
