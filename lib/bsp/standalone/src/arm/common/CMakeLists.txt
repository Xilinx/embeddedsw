# Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT

if("${proc_compiler}" STREQUAL "armclang")
  add_subdirectory(armclang)
elseif("${proc_compiler}" STREQUAL "iccarm.exe")
  add_subdirectory(iccarm)
elseif(NOT ("${proc_compiler}" STREQUAL "armcc"))
  add_subdirectory(gcc)
endif()
collect (PROJECT_LIB_SOURCES putnum.c)
collect (PROJECT_LIB_SOURCES vectors.c)
collect (PROJECT_LIB_SOURCES xil_exception.c)
collect (PROJECT_LIB_SOURCES xil_spinlock.c)
collect (PROJECT_LIB_SOURCES xpm_counter.c)
collect (PROJECT_LIB_HEADERS vectors.h)
collect (PROJECT_LIB_HEADERS xil_exception.h)
collect (PROJECT_LIB_HEADERS xil_spinlock.h)
