/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_vcresampler.h"

#ifndef SDT
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
#else
XV_vcresampler_Config XV_vcresampler_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,v-vcresampler-1.1", /* compatible */
		0x0, /* reg */
		0x2, /* xlnx,samples-per-clock */
		0x3, /* xlnx,num-video-components */
		0xf00, /* xlnx,max-cols */
		0x870, /* xlnx,max-rows */
		0xa, /* xlnx,max-data-width */
		0x2, /* xlnx,convert-type */
		0x4 /* xlnx,num-v-taps */
	},
	 {
		 NULL
	}
};
#endif