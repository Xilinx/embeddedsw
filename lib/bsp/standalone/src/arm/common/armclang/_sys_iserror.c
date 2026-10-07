/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
*
* SPDX-License-Identifier: MIT
*
******************************************************************************/
/**
 * @file _sys_iserror.c
 * @brief Semihosting iserror stub for ARM compilers.
 */
#include "xil_types.h"

/* Stub for iserror() function */
/**
 * @brief Stub for semihosting iserror; returns 1 if status is negative.
 * @param[in] status Status code to test.
 * @return 1 if status is negative, 0 otherwise.
 */
__attribute__((weak)) s32 _sys_iserror(s32 status)
{
	if (status < 0) {
		return 1;
	}

	return 0;
}
