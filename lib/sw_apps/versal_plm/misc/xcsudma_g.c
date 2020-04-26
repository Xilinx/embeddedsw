
/*******************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "xparameters.h"
#include "xcsudma.h"

/*
* The configuration table for devices
*/

XCsuDma_Config XCsuDma_ConfigTable[XPAR_XCSUDMA_NUM_INSTANCES] =
{
	{
		XPAR_PSV_PMC_DMA_0_DEVICE_ID,
		XPAR_PSV_PMC_DMA_0_BASEADDR,
		XPAR_PSV_PMC_DMA_0_DMATYPE
	},
	{
		XPAR_PSV_PMC_DMA_1_DEVICE_ID,
		XPAR_PSV_PMC_DMA_1_BASEADDR,
		XPAR_PSV_PMC_DMA_1_DMATYPE
	}
};


