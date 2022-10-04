/*******************************************************************
* Copyright (C) 2022 Xilinx, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
* Description: Driver configuration
*
*******************************************************************/

#include "xparameters.h"
#include "xcframe.h"

/*
* The configuration table for devices
*/

XCframe_Config XCframe_ConfigTable[XPAR_XCFRAME_NUM_INSTANCES] =
{
	{
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_CFI_CFRAME_0_DEVICE_ID,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_CFI_CFRAME_0_BASEADDR
	}
};
