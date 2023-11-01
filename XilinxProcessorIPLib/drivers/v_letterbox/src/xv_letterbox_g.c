/*******************************************************************
* Copyright (C) 2010-2015 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
********************************************************************/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_letterbox.h"

#ifndef SDT
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
#else
XV_letterbox_Config XV_letterbox_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,v-letterbox-1.1", /* compatible */
		0x50000, /* reg */
		0x2, /* xlnx,samples-per-clock */
		0x3, /* xlnx,num-video-components */
		0xf00, /* xlnx,max-cols */
		0x870, /* xlnx,max-rows */
		0xa /* xlnx,max-data-width */
	},
	 {
		 NULL
	}
};
#endif