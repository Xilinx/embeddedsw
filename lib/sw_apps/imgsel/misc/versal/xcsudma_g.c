/*******************************************************************
* Copyright (C) 2010-2022 Xilinx, Inc. All Rights Reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
* Description: Driver configuration
*
*******************************************************************/

#include "xparameters.h"
#include "xcsudma.h"

/*
* The configuration table for devices
*/

XCsuDma_Config XCsuDma_ConfigTable[XPAR_XCSUDMA_NUM_INSTANCES] =
{
	{
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_DMA_0_DEVICE_ID,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_DMA_0_BASEADDR,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_DMA_0_DMATYPE
	},
	{
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_DMA_1_DEVICE_ID,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_DMA_1_BASEADDR,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_DMA_1_DMATYPE
	}
};
