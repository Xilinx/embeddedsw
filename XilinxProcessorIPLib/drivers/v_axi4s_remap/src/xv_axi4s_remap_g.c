/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_axi4s_remap.h"

/*
* The configuration table for devices
*/

XV_axi4s_remap_Config XV_axi4s_remap_ConfigTable[]
{
#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
	{
		XPAR_V_AXI4S_REMAP_0_DEVICE_ID,
		XPAR_V_AXI4S_REMAP_0_S_AXI_CTRL_BASEADDR,
		XPAR_V_AXI4S_REMAP_0_NUM_VIDEO_COMPONENTS,
		XPAR_V_AXI4S_REMAP_0_MAX_COLS,
		XPAR_V_AXI4S_REMAP_0_MAX_ROWS,
		XPAR_V_AXI4S_REMAP_0_IN_SAMPLES_PER_CLOCK,
		XPAR_V_AXI4S_REMAP_0_OUT_SAMPLES_PER_CLOCK,
		XPAR_V_AXI4S_REMAP_0_CONVERT_SAMPLES_PER_CLOCK,
		XPAR_V_AXI4S_REMAP_0_IN_MAX_DATA_WIDTH,
		XPAR_V_AXI4S_REMAP_0_OUT_MAX_DATA_WIDTH,
		XPAR_V_AXI4S_REMAP_0_IN_HDMI_420,
		XPAR_V_AXI4S_REMAP_0_OUT_HDMI_420,
		XPAR_V_AXI4S_REMAP_0_IN_SAMPLE_DROP,
		XPAR_V_AXI4S_REMAP_0_OUT_SAMPLE_REPEAT
	}
#endif
};
