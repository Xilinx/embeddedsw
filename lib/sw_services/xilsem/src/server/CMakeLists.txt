# Copyright (C) 2023-2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xilsem.h)

if("${CMAKE_MACHINE}" STREQUAL "Versal")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsem_versal_SDT.a ${CMAKE_LIBRARY_PATH}/libxilsem.a)
endif()

if("${CMAKE_MACHINE}" STREQUAL "VersalNet")
	if("${CMAKE_SUBMACHINE}" STREQUAL "VersalNet")
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsem_versal_net_SDT.a ${CMAKE_LIBRARY_PATH}/libxilsem.a)
	elseif("${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM")
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsem_versal_2ve_2vm_SDT.a ${CMAKE_LIBRARY_PATH}/libxilsem.a)
	endif()
endif()
