/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
******************************************************************************/
/**
 * @file _sys_exit.c
 * @brief Semihosting exit stub for ARM compilers.
 */
#include "xil_types.h"

/* Stub for exit() sys-call */
/**
 * @brief Stub for semihosting exit; spins indefinitely.
 * @param[in] rc Return code (unused).
 */
__attribute__((weak)) void _sys_exit(__attribute__((unused)) s32 rc)
{
	while(1) {
		;
	}
}

/* Weak stub for exit() */
/**
 * @brief Stub for exit(); spins indefinitely as there is no OS to return to.
 * @param[in] status Exit status code (unused).
 */
__attribute__((weak)) void exit(int status)
{
	(void)status;
	while (1) {
		;
	}
}
