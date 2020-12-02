/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xparameters.h"
#include "xdfxasm.h"

/* The configuration table for devices */
XDfxasm_Config XDfxasm_ConfigTable[XPAR_XDFXASM_NUM_INSTANCES] =
{
	{
		XPAR_DFX_AXI_SHUTDOWN_MAN_0_DEVICE_ID,
		XPAR_DFX_AXI_SHUTDOWN_MAN_0_BASEADDR
	}
};
