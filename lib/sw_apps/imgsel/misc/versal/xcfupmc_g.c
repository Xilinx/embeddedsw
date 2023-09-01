/*******************************************************************
* Copyright (C) 2022 Xilinx, Inc. All Rights Reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
* Description: Driver configuration
*
*******************************************************************/

#include "xparameters.h"
#include "xcfupmc.h"

/*
* The configuration table for devices
*/

XCfupmc_Config XCfupmc_ConfigTable[XPAR_XCFUPMC_NUM_INSTANCES] =
{
	{
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_CFU_APB_0_DEVICE_ID,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_CFU_APB_0_BASEADDR
	}
};
