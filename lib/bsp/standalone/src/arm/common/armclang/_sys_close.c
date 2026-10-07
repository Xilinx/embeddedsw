/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
*
* SPDX-License-Identifier: MIT
*
*****************************************************************************/
/**
 * @file _sys_close.c
 * @brief Semihosting close stub for ARM compilers.
 */
#include "xil_types.h"

/* Stub for close() sys-call */
/**
 * @brief Stub for semihosting close; always returns -1 (not supported).
 * @param[in] fh File handle (unused).
 * @return Always -1.
 */
__attribute__((weak)) s32 _sys_close(__attribute__((unused)) s32 fh)
{
	return -1;
}
