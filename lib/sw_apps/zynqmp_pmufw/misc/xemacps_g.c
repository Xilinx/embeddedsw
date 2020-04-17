/******************************************************************************
* Copyright (c) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


#include "xparameters.h"
#include "xemacps.h"

/*
* The configuration table for devices
*/

XEmacPs_Config XEmacPs_ConfigTable[XPAR_XEMACPS_NUM_INSTANCES] =
{
	{
		XPAR_PSU_ETHERNET_3_DEVICE_ID,
		XPAR_PSU_ETHERNET_3_BASEADDR,
		XPAR_PSU_ETHERNET_3_IS_CACHE_COHERENT
	}
};
