# Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS xocp_client.h)
collect (PROJECT_LIB_HEADERS xocp_mailbox.h)

collect (PROJECT_LIB_SOURCES xocp_client.c)
collect (PROJECT_LIB_SOURCES xocp_mailbox.c)

if("${CMAKE_SUBMACHINE}" STREQUAL "VersalNet")
    add_subdirectory(hwpcr)
    add_subdirectory(swpcr)
    add_subdirectory(dme)
    add_subdirectory(key_mgmt)
endif()

if("${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM")
    add_subdirectory(hwpcr)
    add_subdirectory(swpcr)
endif()
