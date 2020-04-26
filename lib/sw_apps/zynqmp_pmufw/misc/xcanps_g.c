
/*******************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "xparameters.h"
#include "xcanps.h"

/*
* The configuration table for devices
*/

XCanPs_Config XCanPs_ConfigTable[XPAR_XCANPS_NUM_INSTANCES] =
{
	{
		XPAR_PSU_CAN_1_DEVICE_ID,
		XPAR_PSU_CAN_1_BASEADDR
	}
};
