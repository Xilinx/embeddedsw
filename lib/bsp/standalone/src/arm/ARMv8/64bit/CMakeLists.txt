# Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
if("${proc_compiler}" STREQUAL "armclang")
  add_subdirectory(armclang)
else()
  add_subdirectory(gcc)
endif()
add_subdirectory(platform)

string(FIND "${CMAKE_C_FLAGS}" "-DXEN_USE_PV_CONSOLE" XEN_PV_CONSOLE_ENABLE)
if(XEN_PV_CONSOLE_ENABLE GREATER -1)
        add_subdirectory(xpvxenconsole)
endif()

collect (PROJECT_LIB_HEADERS xreg_cortexa53.h)
collect (PROJECT_LIB_HEADERS xil_mmu.h)
collect (PROJECT_LIB_SOURCES xil_smc.c)
collect (PROJECT_LIB_HEADERS xpseudo_asm.h)
collect (PROJECT_LIB_HEADERS xil_cache.h)
collect (PROJECT_LIB_SOURCES xil_cache.c)
collect (PROJECT_LIB_SOURCES xil_mmu.c)
collect (PROJECT_LIB_HEADERS xil_smc.h)
collect (PROJECT_LIB_HEADERS xil_errata.h)
collect (PROJECT_LIB_HEADERS xpm_counter.h)
