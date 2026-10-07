/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file _sys_close.c
 * @brief Semihosting close stub for ARM compilers.
 */

#include "xil_types.h"
/* Stuv for close() sys-call */
/**
 * @brief Stub for semihosting close; always returns -1.
 * @param[in] fh File handle (unused).
 * @return Always -1.
 */
__weak s32 _sys_close(s32 fh)
{
   return -1;
}
