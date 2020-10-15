/******************************************************************************
* Copyright (C) 2004 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xparameters.h"
#include "platform_config.h"

#ifdef STDOUT_IS_16550
#include "xuartns550_l.h"
#endif

void
init_stdout()
{
	/* if we have a uart 16550, then that needs to be initialized */
#ifdef STDOUT_IS_16550
	XUartNs550_SetBaud(STDOUT_BASEADDR, XPAR_XUARTNS550_CLOCK_HZ, 9600);
	XUartNs550_SetLineControlReg(STDOUT_BASEADDR, XUN_LCR_8_DATA_BITS);
#endif
}
