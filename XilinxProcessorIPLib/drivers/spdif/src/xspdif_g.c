
/*******************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "xparameters.h"
#include "xspdif.h"

/*
* The configuration table for devices
*/

XSpdif_Config XSpdif_ConfigTable[XPAR_XSPDIF_NUM_INSTANCES] =
{
	{
		XPAR_SPDIF_0_DEVICE_ID,
		XPAR_SPDIF_0_BASEADDR
	},
	{
		XPAR_SPDIF_1_DEVICE_ID,
		XPAR_SPDIF_1_BASEADDR
	}
};


