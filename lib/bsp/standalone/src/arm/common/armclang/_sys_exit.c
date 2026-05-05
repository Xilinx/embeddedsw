/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
******************************************************************************/
#include "xil_types.h"

/* Stub for exit() sys-call */
__attribute__((weak)) void _sys_exit(__attribute__((unused)) s32 rc)
{
	while(1) {
		;
	}
}

/* Weak stub for exit() */
__attribute__((weak)) void exit(int status)
{
	(void)status;
	while (1) {
		;
	}
}
