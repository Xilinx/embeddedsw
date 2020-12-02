/*******************************************************************
*
* Copyright (C) 2010-2015 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/

#include "xparameters.h"
#include "xv_letterbox.h"

/*
* The configuration table for devices
*/

XV_letterbox_Config XV_letterbox_ConfigTable[] =
{
	{
#ifdef XPAR_XV_LETTERBOX_NUM_INSTANCES
		XPAR_V_PROC_SS_0_V_LETTERBOX_DEVICE_ID,
		XPAR_V_PROC_SS_0_V_LETTERBOX_S_AXI_CTRL_BASEADDR,
		XPAR_V_PROC_SS_0_V_LETTERBOX_SAMPLES_PER_CLOCK,
		XPAR_V_PROC_SS_0_V_LETTERBOX_NUM_VIDEO_COMPONENTS,
		XPAR_V_PROC_SS_0_V_LETTERBOX_MAX_COLS,
		XPAR_V_PROC_SS_0_V_LETTERBOX_MAX_ROWS,
		XPAR_V_PROC_SS_0_V_LETTERBOX_MAX_DATA_WIDTH
#endif
	}
};
