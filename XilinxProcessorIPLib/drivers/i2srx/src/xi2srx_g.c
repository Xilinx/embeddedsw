
/*******************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "xparameters.h"
#include "xi2srx.h"

/*
* The configuration table for devices
*/

XI2srx_Config XI2srx_ConfigTable[XPAR_XI2SRX_NUM_INSTANCES] =
{
	{
		XPAR_I2S_RECEIVER_0_DEVICE_ID,
		XPAR_I2S_RECEIVER_0_BASEADDR,
		XPAR_I2S_RECEIVER_0_DWIDTH,
		XPAR_I2S_RECEIVER_0_IS_MASTER,
		XPAR_I2S_RECEIVER_0_NUM_CHANNELS,
		XPAR_I2S_RECEIVER_0_32BIT_LR
	}
};


