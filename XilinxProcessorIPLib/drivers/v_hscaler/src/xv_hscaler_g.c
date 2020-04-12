/*******************************************************************
*
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/

#include "xparameters.h"
#include "xv_hscaler.h"

/*
* The configuration table for devices
*/

XV_hscaler_Config XV_hscaler_ConfigTable[] =
{
	{
#ifdef XPAR_XV_HSCALER_NUM_INSTANCES
		XPAR_V_PROC_SS_0_V_HSCALER_DEVICE_ID,
		XPAR_V_PROC_SS_0_V_HSCALER_S_AXI_CTRL_BASEADDR,
		XPAR_V_PROC_SS_0_V_HSCALER_SAMPLES_PER_CLOCK,
		XPAR_V_PROC_SS_0_V_HSCALER_NUM_VIDEO_COMPONENTS,
		XPAR_V_PROC_SS_0_V_HSCALER_MAX_COLS,
		XPAR_V_PROC_SS_0_V_HSCALER_MAX_ROWS,
		XPAR_V_PROC_SS_0_V_HSCALER_MAX_DATA_WIDTH,
		XPAR_V_PROC_SS_0_V_HSCALER_PHASE_SHIFT,
		XPAR_V_PROC_SS_0_V_HSCALER_SCALE_MODE,
		XPAR_V_PROC_SS_0_V_HSCALER_TAPS,
		XPAR_V_PROC_SS_0_V_HSCALER_ENABLE_422,
		XPAR_V_PROC_SS_0_V_HSCALER_ENABLE_420,
		XPAR_V_PROC_SS_0_V_HSCALER_ENABLE_CSC
#endif
	}
};
