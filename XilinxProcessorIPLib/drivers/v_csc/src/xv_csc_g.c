/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_csc.h"

#ifndef SDT
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
#else
XV_csc_Config XV_csc_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {
	{
		"xlnx,v-csc-1.1", /* compatible */
		0x0, /* reg */
		0x2, /* xlnx,samples-per-clock */
		0xf00, /* xlnx,v-csc-max-width */
		0x870, /* xlnx,v-csc-max-height */
		0xa, /* xlnx,max-data-width */
		0x1, /* xlnx,enable-422 */
		0x1, /* xlnx,enable-420 */
		0x1 /* xlnx,enable-window */
	},
	 {
		 NULL
	}
};
#endif