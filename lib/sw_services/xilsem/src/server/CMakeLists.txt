# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xilsem.h)

if("${CMAKE_MACHINE}" STREQUAL "Versal")
execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsem_versal.a ${CMAKE_LIBRARY_PATH}/libxilsem.a)
endif()

if("${CMAKE_MACHINE}" STREQUAL "VersalNet")
execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsem_versal_net.a ${CMAKE_LIBRARY_PATH}/libxilsem.a)
endif()
