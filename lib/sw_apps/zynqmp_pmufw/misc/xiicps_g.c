/******************************************************************************
* Copyright (c) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


#include "xparameters.h"
#include "xiicps.h"

/*
* The configuration table for devices
*/

XIicPs_Config XIicPs_ConfigTable[XPAR_XIICPS_NUM_INSTANCES] =
{
	{
		XPAR_PSU_I2C_0_DEVICE_ID,
		XPAR_PSU_I2C_0_BASEADDR,
		XPAR_PSU_I2C_0_I2C_CLK_FREQ_HZ
	},
	{
		XPAR_PSU_I2C_1_DEVICE_ID,
		XPAR_PSU_I2C_1_BASEADDR,
		XPAR_PSU_I2C_1_I2C_CLK_FREQ_HZ
	}
};
