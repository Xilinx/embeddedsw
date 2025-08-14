/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
********************************************************************/
/**
 * @file xv_hcresampler_g.c
 * @addtogroup v_hcresampler Overview
 */

#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_hcresampler.h"

#ifndef SDT
/**
 * XV_hcresampler_ConfigTable
 *
 * This table contains configuration structures for the XV_hcresampler driver.
 * Each entry represents the configuration parameters for a specific instance
 * of the v-hcresampler hardware core.
 *
 * Fields:
 * - Device ID: Unique identifier for the hardware instance.
 * - Base Addr: Base address of the hardware core.
 * - Samples/Clk: Number of samples processed per clock cycle.
 * - Max Columns: Maximum number of columns supported.
 * - Max Rows: Maximum number of rows supported.
 * - Max Data Width: Maximum data width supported.
 * - Convert Type: Type of conversion performed by the core.
 * - Num H Taps: Number of horizontal filter taps.
 */
XV_hcresampler_Config XV_hcresampler_ConfigTable[] =
{
	{
#ifdef XPAR_XV_HCRESAMPLER_NUM_INSTANCES
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_DEVICE_ID, /* Device ID */
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_S_AXI_CTRL_BASEADDR, /* Base Addr */
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_SAMPLES_PER_CLOCK, /* Samples/Clk */
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_MAX_COLS, /* Max Columns */
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_MAX_ROWS, /* Max Rows */
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_MAX_DATA_WIDTH, /* Max Data Width */
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_CONVERT_TYPE, /* Convert Type */
		XPAR_V_PROC_SS_0_V_HCRESAMPLER_NUM_H_TAPS /* Num H Taps */
#endif
	}
};
#else
/**
 * XV_hcresampler_ConfigTable
 *
 * This table contains configuration structures for the XV_hcresampler driver.
 * Each entry represents the configuration parameters for a specific instance
 * of the v-hcresampler hardware core.
 *
 * Fields:
 * - compatible: Hardware core compatibility string.
 * - reg: Base address of the hardware core.
 * - xlnx,samples-per-clock: Number of samples processed per clock cycle.
 * - xlnx,max-cols: Maximum number of columns supported.
 * - xlnx,max-rows: Maximum number of rows supported.
 * - xlnx,max-data-width: Maximum data width supported.
 * - xlnx,convert-type: Type of conversion performed by the core.
 * - xlnx,num-h-taps: Number of horizontal filter taps.
 *
 * The final entry with NULL indicates the end of the configuration table.
 */
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