/*******************************************************************
* Copyright (C) 2022 Xilinx, Inc. All Rights Reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
* Description: Driver configuration
*
*******************************************************************/

#include "xparameters.h"
#include "xiicps.h"

/*
* The configuration table for devices
*/

XIicPs_Config XIicPs_ConfigTable[XPAR_XIICPS_NUM_INSTANCES] =
{
	{
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_I2C_0_DEVICE_ID,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_I2C_0_BASEADDR,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_I2C_0_I2C_CLK_FREQ_HZ
	},
	{
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_I2C_1_DEVICE_ID,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_I2C_1_BASEADDR,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_I2C_1_I2C_CLK_FREQ_HZ
	}
};
