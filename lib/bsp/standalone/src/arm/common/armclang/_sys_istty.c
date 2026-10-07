/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
*
* SPDX-License-Identifier: MIT
*
******************************************************************************/
/**
 * @file _sys_istty.c
 * @brief Semihosting istty stub for ARM compilers.
 */
#include "xil_types.h"

/* Stub for istty sys-call */
/**
 * @brief Stub for semihosting istty; always returns 1 (is a TTY).
 * @param[in] f File handle pointer (unused).
 * @return Always 1.
 */
__attribute__((weak)) s32 _sys_istty(__attribute__((unused)) u32* f)
{
	/* cannot read/write files */
	return 1;
}
