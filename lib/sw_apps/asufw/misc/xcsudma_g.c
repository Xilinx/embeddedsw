/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

#include "xparameters.h"
#include "xcsudma.h"

/*
* The configuration table for devices
*/
XCsuDma_Config XCsuDma_ConfigTable[] =
{
	{
		XPAR_ASU_DMA_0_DEVICE_ID,
		XPAR_ASU_DMA_0_BASEADDR,
		XPAR_ASU_DMA0_DMA_TYPE
	},

	{
		XPAR_ASU_DMA_1_DEVICE_ID,
		XPAR_ASU_DMA_1_BASEADDR,
		XPAR_ASU_DMA1_DMA_TYPE
	}
};
