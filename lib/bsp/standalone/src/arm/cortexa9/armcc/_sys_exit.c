/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file _sys_exit.c
 * @brief Semihosting exit stub for ARM compilers.
 */

#include "xil_types.h"
/* Stuv for exit() sys-call */
/**
 * @brief Stub for semihosting exit; spins indefinitely.
 * @param[in] rc Return code (unused).
 */
__weak void _sys_exit(s32 rc)
{
   while(1) {
		;
	}
}
