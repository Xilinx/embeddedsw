/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file _sys_read.c
 * @brief Semihosting read stub for ARM compilers.
 */

#include "xil_types.h"
/* Stub for read() sys-call */
/**
 * @brief Stub for semihosting read; performs no input and returns len.
 * @param[in] fh File handle (unused).
 * @param[out] buf Buffer for read bytes (unused; left unchanged).
 * @param[in] len Number of bytes requested.
 * @param[in] mode Transfer mode (unused).
 * @return Number of bytes NOT read.
 */
__weak s32 _sys_read(u32 fh, u8 *buf, u32 len, s32 mode)
{
   /* Return the number of character NOT read */
   return len;
}
