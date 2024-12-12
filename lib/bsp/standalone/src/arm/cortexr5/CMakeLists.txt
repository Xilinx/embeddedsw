# Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
if("${proc_compiler}" STREQUAL "armclang")
  add_subdirectory(armclang)
elseif(NOT ("${proc_compiler}" STREQUAL "iccarm.exe"))
  add_subdirectory(gcc)
endif()
add_subdirectory(platform)
collect (PROJECT_LIB_HEADERS xil_mmu.h)
collect (PROJECT_LIB_HEADERS xpseudo_asm.h)
collect (PROJECT_LIB_HEADERS xil_cache.h)
collect (PROJECT_LIB_HEADERS xpm_counter.h)
collect (PROJECT_LIB_SOURCES xil_cache.c)
collect (PROJECT_LIB_HEADERS xreg_cortexr5.h)
