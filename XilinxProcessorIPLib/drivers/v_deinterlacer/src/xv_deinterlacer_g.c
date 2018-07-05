/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/
#include "xparameters.h"
#include "xv_deinterlacer.h"

/*
* The configuration table for devices
*/

XV_deinterlacer_Config XV_deinterlacer_ConfigTable[] =
{
	{
#ifdef XPAR_XV_DEINTERLACER_NUM_INSTANCES
		XPAR_V_PROC_SS_0_V_DEINTERLACER_DEVICE_ID,
		XPAR_V_PROC_SS_0_V_DEINTERLACER_S_AXI_CTRL_BASEADDR,
		XPAR_V_PROC_SS_0_V_DEINTERLACER_NUM_VIDEO_COMPONENTS,
		XPAR_V_PROC_SS_0_V_DEINTERLACER_MAX_DATA_WIDTH
#endif
	}
};
