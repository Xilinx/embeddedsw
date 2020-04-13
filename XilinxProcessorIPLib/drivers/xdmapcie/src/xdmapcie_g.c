/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xparameters.h"
#include "xdmapcie.h"

/*
* The configuration table for devices
*/

XDmaPcie_Config XDmaPcie_ConfigTable[XPAR_XDMAPCIE_NUM_INSTANCES] =
{
	{
		XPAR_XDMA_0_DEVICE_ID,
		XPAR_XDMA_0_BASEADDR,
		XPAR_XDMA_0_AXIBAR_NUM,
		XPAR_XDMA_0_INCLUDE_BAROFFSET_REG,
		XPAR_XDMA_0_INCLUDE_RC,
		XPAR_XDMA_0_BASEADDR,
		XPAR_XDMA_0_AXIBAR_0,
		XPAR_XDMA_0_AXIBAR_1,
		XPAR_XDMA_0_AXIBAR_HIGHADDR_0,
		XPAR_XDMA_0_AXIBAR_HIGHADDR_1
	}
};
