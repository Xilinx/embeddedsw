# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5")
collect (PROJECT_LIB_SOURCES cortexr5_sleep.c)
endif()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa9")
collect (PROJECT_LIB_SOURCES globaltimer_sleep_zynq.c)
endif()

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblazeel") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze"))
collect (PROJECT_LIB_SOURCES microblaze_sleep.c)
endif()

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72")
	OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53")
	OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53-32")
	OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64"))
collect (PROJECT_LIB_SOURCES globaltimer_sleep.c)
endif()
