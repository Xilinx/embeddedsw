/*******************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*

*******************************************************************************/*


#include "xparameters.h"
#include "xmcdma.h"

/*
* The configuration table for devices
*/

XMcdma_Config XMcdma_ConfigTable[] =
{
	{
		XPAR_AXI_MCDMA_0_DEVICE_ID,
		XPAR_AXI_MCDMA_0_BASEADDR,
		XPAR_AXI_MCDMA_0_ADDR_WIDTH,
		XPAR_AXI_MCDMA_0_ENABLE_SINGLE_INTR,
		XPAR_AXI_MCDMA_0_INCLUDE_MM2S,
		XPAR_AXI_MCDMA_0_INCLUDE_MM2S_DRE,
		XPAR_AXI_MCDMA_0_NUM_MM2S_CHANNELS,
		XPAR_AXI_MCDMA_0_INCLUDE_S2MM,
		XPAR_AXI_MCDMA_0_INCLUDE_S2MM_DRE,
		XPAR_AXI_MCDMA_0_NUM_S2MM_CHANNELS,
		XPAR_AXI_MCDMA_0_M_AXI_MM2S_DATA_WIDTH,
		XPAR_AXI_MCDMA_0_M_AXI_S2MM_DATA_WIDTH,
		XPAR_AXI_MCDMA_0_SG_LENGTH_WIDTH,
		XPAR_AXI_MCDMA_0_SG_INCLUDE_STSCNTRL_STRM
	}
};
