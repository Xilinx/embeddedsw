/*******************************************************************
* Copyright (C) 2010-2015 Xilinx, Inc. All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
********************************************************************/
/**
 * @file xv_letterbox_g.c
 * @addtogroup xv_letterbox Overview
 */

#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_letterbox.h"

#ifndef SDT
/**
 * XV_letterbox_ConfigTable - Configuration table for XV_letterbox driver instances.
 *
 * This table contains configuration structures for each XV_letterbox hardware instance.
 * Each entry specifies the hardware parameters for the instance.
 *
 * Fields:
 *   DEVICE_ID         - Unique device ID for the instance.
 *   BASEADDR          - Base address of the hardware register.
 *   SAMPLES_PER_CLOCK - Number of samples processed per clock cycle.
 *   NUM_VIDEO_COMPONENTS - Number of video components supported.
 *   MAX_COLS          - Maximum number of columns supported.
 *   MAX_ROWS          - Maximum number of rows supported.
 *   MAX_DATA_WIDTH    - Maximum data width supported.
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
/**
 * XV_letterbox_ConfigTable - Configuration table for XV_letterbox driver instances.
 *
 * This table contains configuration structures for each XV_letterbox hardware instance.
 * Each entry specifies the hardware parameters and compatible string for the instance.
 * The table is placed in the ".drvcfg_sec" section for driver configuration.
 *
 * Fields:
 *   compatible           - Device tree compatible string (e.g., "xlnx,v-letterbox-1.1").
 *   reg                  - Base address of the hardware register.
 *   samples-per-clock    - Number of samples processed per clock cycle.
 *   num-video-components - Number of video components supported.
 *   max-cols             - Maximum number of columns supported.
 *   max-rows             - Maximum number of rows supported.
 *   max-data-width       - Maximum data width supported.
 *
 * The last entry is a sentinel with NULL to indicate the end of the table.
 */
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