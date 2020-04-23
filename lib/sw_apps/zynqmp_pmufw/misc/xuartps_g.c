/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xparameters.h"
#include "xuartps.h"

/*
* The configuration table for devices
*/

XUartPs_Config XUartPs_ConfigTable[XPAR_XUARTPS_NUM_INSTANCES] =
{
	{
		XPAR_PSU_UART_0_DEVICE_ID,
		XPAR_PSU_UART_0_BASEADDR,
		XPAR_PSU_UART_0_UART_CLK_FREQ_HZ,
		XPAR_PSU_UART_0_HAS_MODEM
	},
	{
		XPAR_PSU_UART_1_DEVICE_ID,
		XPAR_PSU_UART_1_BASEADDR,
		XPAR_PSU_UART_1_UART_CLK_FREQ_HZ,
		XPAR_PSU_UART_1_HAS_MODEM
	}
};


