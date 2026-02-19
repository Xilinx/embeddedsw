/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

/**
 * @file xv_axi4s_remap.c
 * @addtogroup v_axi4s_remap Overview
 */

/***************************** Include Files *********************************/
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_axi4s_remap.h"


/************************** Variable Definitions *****************************/
/**
 * Configuration table for AXI4-Stream Remap devices
 *
 * This table contains the configuration information for each AXI4-Stream Remap
 * instance in the system, including device ID, base address, video parameters,
 * and feature support flags.
 */
XV_axi4s_remap_Config XV_axi4s_remap_ConfigTable[] =
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
