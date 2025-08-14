/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/**
* @file xv_deinterlacer_g.c
* @addtogroup v_deinterlacer Overview
*/
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_deinterlacer.h"

#ifndef SDT
/**
 * XV_deinterlacer_ConfigTable - Configuration table for XV_deinterlacer driver instances.
 *
 * This table contains configuration structures for each instance of the
 * XV_deinterlacer hardware. Each entry provides the following information:
 *   - Device ID
 *   - Base address of the hardware instance
 *   - Number of video components supported
 *   - Maximum data width supported
 *
 * The values are populated from xparameters.h macros.
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
/**
 * @brief
 *
 * XV_deinterlacer_ConfigTable - Configuration table for XV_deinterlacer driver instances.
 *
 * This table contains configuration structures for each instance of the
 * XV_deinterlacer hardware. Each entry provides the following information:
 *   - compatible: Device compatibility string (e.g., "xlnx,v-deinterlacer-5.1").
 *   - reg: Base address of the hardware instance.
 *   - xlnx,num-video-components: Number of video components supported.
 *   - xlnx,max-data-width: Maximum data width supported.
 *
 * The table is placed in the ".drvcfg_sec" section for driver configuration.
 * The last entry is a sentinel with NULL to indicate the end of the table.
 */
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