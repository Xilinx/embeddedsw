/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file _sys_open.c
 * @brief Semihosting open stub for ARM compilers.
 */

#include "xil_types.h"
/* Stub for open sys-call */
/**
 * @brief Stub for semihosting open; always returns 0.
 * @param[in] name Filename (unused).
 * @param[in] openmode Open mode flags (unused).
 * @return Always 0.
 */
__weak s32 _sys_open(const char8* name, s32 openmode)
{
   return 0;
}
