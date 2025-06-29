
/*******************************************************************
*
* CAUTION: This file is automatically generated by HSI.
* Version: 2021.1
* DO NOT EDIT.
*
* Copyright (c) 2010 - 2022, Xilinx, Inc. All Rights Reserved.
* Copyright (c) 2022 - 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT

*
* Description: Driver configuration
*
*******************************************************************/

#include "xparameters.h"
#include "xiomodule.h"



/*
* The configuration table for devices
*/

XIOModule_Config XIOModule_ConfigTable[] =
{
	{
		XPAR_PSV_PMC_IOMODULE_0_DEVICE_ID,
		XPAR_PSV_PMC_IOMODULE_0_BASEADDR,
		XPAR_PSV_PMC_IOMODULE_0_IO_BASEADDR,
		XPAR_PSV_PMC_IOMODULE_0_INTC_HAS_FAST,
		XPAR_PSV_PMC_IOMODULE_0_INTC_BASE_VECTORS,
		XPAR_PSV_PMC_IOMODULE_0_INTC_ADDR_WIDTH ,
		((XPAR_PSV_PMC_IOMODULE_0_INTC_LEVEL_EDGE << 16) | 0x7FF),
		XIN_SVC_SGL_ISR_OPTION,
		XPAR_PSV_PMC_IOMODULE_0_FREQ,
		XPAR_PSV_PMC_IOMODULE_0_UART_BAUDRATE,
		{
			XPAR_PSV_PMC_IOMODULE_0_USE_PIT1,
			XPAR_PSV_PMC_IOMODULE_0_USE_PIT2,
			XPAR_PSV_PMC_IOMODULE_0_USE_PIT3,
			XPAR_PSV_PMC_IOMODULE_0_USE_PIT4,
		},
		{
			XPAR_PSV_PMC_IOMODULE_0_PIT1_SIZE,
			XPAR_PSV_PMC_IOMODULE_0_PIT2_SIZE,
			XPAR_PSV_PMC_IOMODULE_0_PIT3_SIZE,
			XPAR_PSV_PMC_IOMODULE_0_PIT4_SIZE,
		},
		{
			XPAR_PSV_PMC_IOMODULE_0_PIT1_EXPIRED_MASK,
			XPAR_PSV_PMC_IOMODULE_0_PIT2_EXPIRED_MASK,
			XPAR_PSV_PMC_IOMODULE_0_PIT3_EXPIRED_MASK,
			XPAR_PSV_PMC_IOMODULE_0_PIT4_EXPIRED_MASK,
		},
		{
			XPAR_PSV_PMC_IOMODULE_0_PIT1_PRESCALER,
			XPAR_PSV_PMC_IOMODULE_0_PIT2_PRESCALER,
			XPAR_PSV_PMC_IOMODULE_0_PIT3_PRESCALER,
			XPAR_PSV_PMC_IOMODULE_0_PIT4_PRESCALER,
		},
		{
			XPAR_PSV_PMC_IOMODULE_0_PIT1_READABLE,
			XPAR_PSV_PMC_IOMODULE_0_PIT2_READABLE,
			XPAR_PSV_PMC_IOMODULE_0_PIT3_READABLE,
			XPAR_PSV_PMC_IOMODULE_0_PIT4_READABLE,
		},
		{
			XPAR_PSV_PMC_IOMODULE_0_GPO1_INIT,
			XPAR_PSV_PMC_IOMODULE_0_GPO2_INIT,
			XPAR_PSV_PMC_IOMODULE_0_GPO3_INIT,
			XPAR_PSV_PMC_IOMODULE_0_GPO4_INIT,
		},
		{
		}

	}
};
