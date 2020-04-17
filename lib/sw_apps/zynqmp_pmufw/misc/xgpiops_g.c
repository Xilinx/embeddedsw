/******************************************************************************
* Copyright (c) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


#include "xparameters.h"
#include "xgpiops.h"

/*
* The configuration table for devices
*/

XGpioPs_Config XGpioPs_ConfigTable[XPAR_XGPIOPS_NUM_INSTANCES] =
{
	{
		XPAR_PSU_GPIO_0_DEVICE_ID,
		XPAR_PSU_GPIO_0_BASEADDR
	}
};
