/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xparameters.h"
#include "xcframe.h"

/*
* The configuration table for devices
*/

XCframe_Config XCframe_ConfigTable[XPAR_XCFRAME_NUM_INSTANCES] =
{
	{
		XPAR_CFI_CFRAME_0_DEVICE_ID,
		XPAR_CFI_CFRAME_0_BASEADDR
	}
};
