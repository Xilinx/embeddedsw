# Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
add_subdirectory(gcc)
add_subdirectory(platform)
collect (PROJECT_LIB_SOURCES xil_cache.c)
collect (PROJECT_LIB_HEADERS xil_cache.h)
collect (PROJECT_LIB_HEADERS xil_cache_l.h)
collect (PROJECT_LIB_SOURCES xil_mmu.c)
collect (PROJECT_LIB_HEADERS xil_mmu.h)
collect (PROJECT_LIB_HEADERS xparameters_ps.h)
collect (PROJECT_LIB_HEADERS xpseudo_asm.h)
collect (PROJECT_LIB_HEADERS xreg_cortexa53.h)
