/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xparameters.h"
#include "xcfupmc.h"

/*
* The configuration table for devices
*/
XCfupmc_Config XCfupmc_ConfigTable[XPAR_XCFUPMC_NUM_INSTANCES] =
{
	{
		(u16)XPAR_PMC_CFU_0_DEVICE_ID,
		(u32)XPAR_PMC_CFU_0_BASEADDR
	}
};
