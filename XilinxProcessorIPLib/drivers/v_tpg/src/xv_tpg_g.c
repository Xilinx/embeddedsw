/*******************************************************************
* Copyright (C) 2010-2021 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_tpg.h"

/*
* The configuration table for devices
*/

XV_tpg_Config XV_tpg_ConfigTable[] =
{
	{
#ifdef XPAR_XV_TPG_NUM_INSTANCES
		XPAR_V_TPG_0_DEVICE_ID,
		XPAR_V_TPG_0_S_AXI_CTRL_BASEADDR,
		XPAR_V_TPG_0_HAS_AXI4S_SLAVE,
		XPAR_V_TPG_0_SAMPLES_PER_CLOCK,
		XPAR_V_TPG_0_NUM_VIDEO_COMPONENTS,
		XPAR_V_TPG_0_MAX_COLS,
		XPAR_V_TPG_0_MAX_ROWS,
		XPAR_V_TPG_0_MAX_DATA_WIDTH,
		XPAR_V_TPG_0_SOLID_COLOR,
		XPAR_V_TPG_0_RAMP,
		XPAR_V_TPG_0_COLOR_BAR,
		XPAR_V_TPG_0_DISPLAY_PORT,
		XPAR_V_TPG_0_COLOR_SWEEP,
		XPAR_V_TPG_0_ZONE_PLATE,
		XPAR_V_TPG_0_FOREGROUND
#endif
	}
};
