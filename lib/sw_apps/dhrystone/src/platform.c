/******************************************************************************
* Copyright (c) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xparameters.h"
#include "xil_cache.h"
#if !defined (SDT)
#include "platform_config.h"
#endif
#if defined (STDOUT_IS_16550) || ( defined (SDT) && defined (XPAR_STDIN_IS_UARTNS550))
#include "xuartns550_l.h"
#endif

void enable_caches()
{
#if defined (__MICROBLAZE__)
#ifdef XPAR_MICROBLAZE_USE_ICACHE
    Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
    Xil_DCacheEnable();
#endif
#endif
}

void disable_caches()
{
	Xil_DCacheDisable();
	Xil_ICacheDisable();
}

void init_platform()
{
	enable_caches();
	/* if we have a uart 16550, then that needs to be initialized */
#if defined (STDOUT_IS_16550) || defined (XPAR_STDIN_IS_UARTNS550)
#if defined (SDT)
	XUartNs550_SetBaud(STDOUT_BASEADDRESS, XPAR_XUARTNS550_0_CLOCK_FREQ, 9600);
	XUartNs550_SetLineControlReg(STDOUT_BASEADDRESS, XUN_LCR_8_DATA_BITS);
#else
	XUartNs550_SetBaud(STDOUT_BASEADDR, XPAR_XUARTNS550_CLOCK_HZ, 9600);
	XUartNs550_SetLineControlReg(STDOUT_BASEADDR, XUN_LCR_8_DATA_BITS);
#endif
#endif
}

void cleanup_platform()
{
	disable_caches();
}

