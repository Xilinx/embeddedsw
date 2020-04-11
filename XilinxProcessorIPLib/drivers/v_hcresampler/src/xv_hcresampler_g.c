/*******************************************************************
*
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/
#include "xparameters.h"
#include "xv_hcresampler.h"

/*
* The configuration table for devices
*/

XV_hcresampler_Config XV_hcresampler_ConfigTable[] =
{
	{
#ifdef XPAR_XV_HCRESAMPLER_NUM_INSTANCES
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_DEVICE_ID,
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_S_AXI_CTRL_BASEADDR,
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_SAMPLES_PER_CLOCK,
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_MAX_COLS,
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_MAX_ROWS,
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_MAX_DATA_WIDTH,
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_CONVERT_TYPE,
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_NUM_H_TAPS
#endif
	}
};
