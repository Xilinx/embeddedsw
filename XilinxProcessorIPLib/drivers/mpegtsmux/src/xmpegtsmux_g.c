/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xparameters.h"
#include "xmpegtsmux.h"

/*
* The configuration table for devices
*/

XMpegtsmux_Config XMpegtsmux_ConfigTable[XPAR_XMPEGTSMUX_NUM_INSTANCES] =
{
	{
		XPAR_MPEGTSMUX_0_DEVICE_ID,
		XPAR_MPEGTSMUX_0_S_AXI_CTRL_BASEADDR
	}
};
