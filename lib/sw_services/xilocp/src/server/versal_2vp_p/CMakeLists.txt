# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# Versal_2VP_P: reuse Versal Net platform and support sources.

collect (PROJECT_LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/../versal_2ve_2vm/xocp_sha.h)
collect (PROJECT_LIB_HEADERS xocp_hw.h)
collect (PROJECT_LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/../versal_net/xocp_dice_dme.h)
collect (PROJECT_LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../versal_net/xocp_dice_dme.c)
collect (PROJECT_LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/../versal_net/xocp_plat.h)
collect (PROJECT_LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../versal_net/xocp_plat.c)
