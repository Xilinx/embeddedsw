/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/
#include "xparameters.h"
#include "xv_csc.h"

/*
* The configuration table for devices
*/

XV_csc_Config XV_csc_ConfigTable[] =
{
	{
#ifdef XPAR_XV_CSC_NUM_INSTANCES
		XPAR_V_PROC_SS_0_V_CSC_DEVICE_ID,
		XPAR_V_PROC_SS_0_V_CSC_S_AXI_CTRL_BASEADDR,
		XPAR_V_PROC_SS_0_V_CSC_SAMPLES_PER_CLOCK,
		XPAR_V_PROC_SS_0_V_CSC_V_CSC_MAX_WIDTH,
		XPAR_V_PROC_SS_0_V_CSC_V_CSC_MAX_HEIGHT,
		XPAR_V_PROC_SS_0_V_CSC_MAX_DATA_WIDTH,
		XPAR_V_PROC_SS_0_V_CSC_ENABLE_422,
		XPAR_V_PROC_SS_0_V_CSC_ENABLE_420,
		XPAR_V_PROC_SS_0_V_CSC_ENABLE_WINDOW
#endif
	}
};
