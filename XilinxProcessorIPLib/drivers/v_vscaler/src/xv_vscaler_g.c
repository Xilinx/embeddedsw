/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_vscaler.h"

#ifndef SDT
/*
* The configuration table for devices
*/

XV_vscaler_Config XV_vscaler_ConfigTable[] =
{
	{
#ifdef XPAR_XV_VSCALER_NUM_INSTANCES
		XPAR_V_PROC_SS_0_V_VSCALER_DEVICE_ID,
		XPAR_V_PROC_SS_0_V_VSCALER_S_AXI_CTRL_BASEADDR,
		XPAR_V_PROC_SS_0_V_VSCALER_SAMPLES_PER_CLOCK,
		XPAR_V_PROC_SS_0_V_VSCALER_NUM_VIDEO_COMPONENTS,
		XPAR_V_PROC_SS_0_V_VSCALER_MAX_COLS,
		XPAR_V_PROC_SS_0_V_VSCALER_MAX_ROWS,
		XPAR_V_PROC_SS_0_V_VSCALER_MAX_DATA_WIDTH,
		XPAR_V_PROC_SS_0_V_VSCALER_PHASE_SHIFT,
		XPAR_V_PROC_SS_0_V_VSCALER_SCALE_MODE,
		XPAR_V_PROC_SS_0_V_VSCALER_TAPS,
		XPAR_V_PROC_SS_0_V_VSCALER_ENABLE_420
#endif
	}
};
#else
XV_vscaler_Config XV_vscaler_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,v-vscaler-1.1", /* compatible */
		0xb0000, /* reg */
		0x2, /* xlnx,samples-per-clock */
		0x3, /* xlnx,num-video-components */
		0xf00, /* xlnx,max-cols */
		0x870, /* xlnx,max-rows */
		0xa, /* xlnx,max-data-width */
		0x6, /* xlnx,phase-shift */
		0x2, /* xlnx,scale-mode */
		0x6, /* xlnx,taps */
		0x1 /* xlnx,enable-420 */
	},
	 {
		 NULL
	}
};
#endif