# Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
add_subdirectory(gcc)
add_subdirectory(platform)
collect (PROJECT_LIB_HEADERS xreg_cortexa53.h)
collect (PROJECT_LIB_HEADERS xil_mmu.h)
collect (PROJECT_LIB_SOURCES xil_smc.c)
collect (PROJECT_LIB_HEADERS xpseudo_asm.h)
collect (PROJECT_LIB_HEADERS xil_cache.h)
collect (PROJECT_LIB_SOURCES xil_cache.c)
collect (PROJECT_LIB_SOURCES xil_mmu.c)
collect (PROJECT_LIB_HEADERS xil_smc.h)
collect (PROJECT_LIB_HEADERS xil_errata.h)
