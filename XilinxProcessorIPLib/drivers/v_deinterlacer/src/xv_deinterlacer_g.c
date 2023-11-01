/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_deinterlacer.h"

#ifndef SDT
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
#else
XV_deinterlacer_Config XV_deinterlacer_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {
	{
		"xlnx,v-deinterlacer-5.1", /* compatible */
		0x0, /* reg */
		0x3, /* xlnx,num-video-components */
		0xa /* xlnx,max-data-width */
	},
	 {
		 NULL
	}
};
#endif