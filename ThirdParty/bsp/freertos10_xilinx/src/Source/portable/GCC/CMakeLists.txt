# Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
if( ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53") OR
    ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") OR
    ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa78")
  )
        add_subdirectory(ARM_CA53)
elseif(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5") OR
       ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr52")
      )
        add_subdirectory(ARM_CR5)
elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa9")
        add_subdirectory(ARM_CA9)
elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze")
        add_subdirectory(MicroBlazeV9)
endif()
