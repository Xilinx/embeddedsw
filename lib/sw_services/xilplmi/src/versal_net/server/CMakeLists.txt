# Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if((NOT "${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM") AND
   (NOT "Versal_2VP_P" IN_LIST VARIANT))
collect (PROJECT_LIB_HEADERS xplmi_err.h)
collect (PROJECT_LIB_HEADERS xplmi_hw.h)
collect (PROJECT_LIB_HEADERS xplmi_plat.h)
collect (PROJECT_LIB_HEADERS xplmi_status.h)
collect (PROJECT_LIB_HEADERS xplmi_config.h)
endif()
collect (PROJECT_LIB_HEADERS xplmi_update.h)
collect (PROJECT_LIB_HEADERS xplmi_wdt.h)

if((NOT "${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM") AND
   (NOT "Versal_2VP_P" IN_LIST VARIANT))
collect (PROJECT_LIB_SOURCES xplmi_err.c)
collect (PROJECT_LIB_SOURCES xplmi_plat.c)
endif()
if (NOT "Versal_2VP_P" IN_LIST VARIANT)
collect (PROJECT_LIB_SOURCES xplmi_plat_cmd.c)
endif()
collect (PROJECT_LIB_SOURCES xplmi_update.c)
collect (PROJECT_LIB_SOURCES xplmi_wdt.c)
