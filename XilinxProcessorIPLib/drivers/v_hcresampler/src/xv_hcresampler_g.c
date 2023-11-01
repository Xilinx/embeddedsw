/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
********************************************************************/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_hcresampler.h"

#ifndef SDT
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
#else

XV_hcresampler_Config XV_hcresampler_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,v-hcresampler-1.1", /* compatible */
		0x0, /* reg */
		0x2, /* xlnx,samples-per-clock */
		0xf00, /* xlnx,max-cols */
		0x870, /* xlnx,max-rows */
		0xa, /* xlnx,max-data-width */
		0x2, /* xlnx,convert-type */
		0x4 /* xlnx,num-h-taps */
	},
	 {
		 NULL
	}
};
#endif