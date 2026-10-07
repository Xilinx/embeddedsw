/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file _sys_write.c
 * @brief Semihosting write stub for ARM compilers.
 */

#include "xil_types.h"
#include "xparameters.h"

/**
 * @brief Stub for semihosting write; outputs to STDOUT if configured.
 * @param[in] fh File handle (unused).
 * @param[in] buf Buffer of bytes to write.
 * @param[in] len Number of bytes to write.
 * @param[in] mode Transfer mode (unused).
 * @return Number of bytes NOT written (always 0).
 */
__weak s32 _sys_write(u32 fh, const u8 *buf, u32 len, s32 mode)
{
#ifdef STDOUT_BASEADDRESS
  u32 volatile *uart_base = (u32 *)STDOUT_BASEADDRESS;
  s32 i;

  for (i =0; i < len;i++) {
    /* wait if TNFUL */
    while (*(uart_base + 11U) & (1U << 14U)) {
		;
	}
    *(uart_base + 12U) = buf[i];
  }
#endif
  return 0;
}
