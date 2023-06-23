/******************************************************************************
* Copyright (C) 2004 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xparameters.h"
#if !defined (SDT)
#include "platform_config.h"
#endif

#if ( defined (STDOUT_IS_16550) || ( defined (SDT) && defined (XPAR_STDIN_IS_UARTNS550)))
#include "xuartns550_l.h"
#endif

void init_stdout()
{
	/* if we have a uart 16550, then that needs to be initialized */
#if ( defined (STDOUT_IS_16550) || ( defined (SDT) && defined (XPAR_STDIN_IS_UARTNS550)))
	XUartNs550_SetBaud(STDOUT_BASEADDR, XPAR_XUARTNS550_CLOCK_HZ, 9600);
	XUartNs550_SetLineControlReg(STDOUT_BASEADDR, XUN_LCR_8_DATA_BITS);
#endif
}
