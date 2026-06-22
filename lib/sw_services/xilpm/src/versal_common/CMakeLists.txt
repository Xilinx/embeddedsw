# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

# For Versal_2VE_2VM the platform headers (including xpm_nodeid.h)
# are provided by versal_2ve_2vm/client, so the shared copy under
# versal_common/common must NOT be pulled in here. Versal_2VE_2VM is
# identified by CMAKE_SUBMACHINE.
if(NOT "${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM")
	add_subdirectory(common)
endif()

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblazeel") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze"))
	add_subdirectory(server)
elseif(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") OR
	("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5") OR
	("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa78") OR
	("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr52"))
	add_subdirectory(client)
else()
	message(FATAL_ERROR "Wrong system processor ${CMAKE_SYSTEM_PROCESSOR}")
endif()
