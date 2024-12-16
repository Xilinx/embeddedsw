# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xilsem.h)

if("${CMAKE_MACHINE}" STREQUAL "Versal")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsem_versal_SDT.a ${CMAKE_LIBRARY_PATH}/libxilsem.a)
endif()

if("${CMAKE_MACHINE}" STREQUAL "VersalNet")
	if("${CMAKE_SUBMACHINE}" STREQUAL "VersalNet")
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsem_versal_net_SDT.a ${CMAKE_LIBRARY_PATH}/libxilsem.a)
	elseif("${CMAKE_SUBMACHINE}" STREQUAL "VersalGen2")
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsem_telluride_SDT.a ${CMAKE_LIBRARY_PATH}/libxilsem.a)
	endif()
endif()
