/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/


#include "xparameters.h"
#include "xv_vcresampler.h"

/*
* The configuration table for devices
*/

XV_vcresampler_Config XV_vcresampler_ConfigTable[] =
{
	{
#ifdef XPAR_XV_VCRESAMPLER_NUM_INSTANCES
		XPAR_V_PROC_SS_0_V_VCRESAMPLER_0_DEVICE_ID,
		XPAR_V_PROC_SS_0_V_VCRESAMPLER_0_S_AXI_CTRL_BASEADDR,
		XPAR_V_PROC_SS_0_V_VCRESAMPLER_0_SAMPLES_PER_CLOCK,
		XPAR_V_PROC_SS_0_V_VCRESAMPLER_0_NUM_VIDEO_COMPONENTS,
		XPAR_V_PROC_SS_0_V_VCRESAMPLER_0_MAX_COLS,
		XPAR_V_PROC_SS_0_V_VCRESAMPLER_0_MAX_ROWS,
		XPAR_V_PROC_SS_0_V_VCRESAMPLER_0_MAX_DATA_WIDTH,
		XPAR_V_PROC_SS_0_V_VCRESAMPLER_0_CONVERT_TYPE,
		XPAR_V_PROC_SS_0_V_VCRESAMPLER_0_NUM_V_TAPS
#endif
	}
};
