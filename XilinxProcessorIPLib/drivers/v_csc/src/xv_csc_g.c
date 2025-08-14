/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/
/**
* @file xv_csc_g.c
* @addtogroup v_csc Overview
*/
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_csc.h"

#ifndef SDT
/*
* The configuration table for devices
*/

/**
 * XV_csc_ConfigTable - Array of configuration structures for the Color Space Converter (CSC) driver.
 *
 * This table contains the configuration parameters for each instance of the CSC hardware
 * present in the system. Each entry is populated with values defined in the hardware
 * platform configuration (xparameters.h), such as device ID, base address, supported
 * data widths, and feature enable flags.
 *
 * Fields:
 *   - DeviceId: Unique identifier for the CSC instance.
 *   - BaseAddress: Base address for the control interface of the CSC.
 *   - SamplesPerClock: Number of samples processed per clock cycle.
 *   - MaxWidth: Maximum supported image width.
 *   - MaxHeight: Maximum supported image height.
 *   - MaxDataWidth: Maximum data width supported by the CSC.
 *   - Enable422: Flag indicating support for 4:2:2 chroma subsampling.
 *   - Enable420: Flag indicating support for 4:2:0 chroma subsampling.
 *   - EnableWindow: Flag indicating support for windowing feature.
 *
 * Note:
 *   The table is conditionally compiled based on the number of CSC instances defined
 *   by XPAR_XV_CSC_NUM_INSTANCES.
 */
XV_csc_Config XV_csc_ConfigTable[] =
{
	{
#ifdef XPAR_XV_CSC_NUM_INSTANCES
		XPAR_V_PROC_SS_0_V_CSC_DEVICE_ID,
		XPAR_V_PROC_SS_0_V_CSC_S_AXI_CTRL_BASEADDR,
		XPAR_V_PROC_SS_0_V_CSC_SAMPLES_PER_CLOCK,
		XPAR_V_PROC_SS_0_V_CSC_V_CSC_MAX_WIDTH,
		XPAR_V_PROC_SS_0_V_CSC_V_CSC_MAX_HEIGHT,
		XPAR_V_PROC_SS_0_V_CSC_MAX_DATA_WIDTH,
		XPAR_V_PROC_SS_0_V_CSC_ENABLE_422,
		XPAR_V_PROC_SS_0_V_CSC_ENABLE_420,
		XPAR_V_PROC_SS_0_V_CSC_ENABLE_WINDOW
#endif
	}
};
#else
/**
 * @brief Configuration table for XV_csc instances.
 *
 * This table contains the configuration parameters for each instance of the XV_csc
 * (Color Space Converter) hardware core. Each entry corresponds to a hardware instance
 * and specifies its properties such as compatible string, base address, supported
 * samples per clock, maximum width and height, data width, and feature enable flags.
 *
 * The table is placed in the ".drvcfg_sec" section.
 *
 * Fields:
 * - compatible: Device tree compatible string.
 * - reg: Base address of the hardware instance.
 * - xlnx,samples-per-clock: Number of samples processed per clock cycle.
 * - xlnx,v-csc-max-width: Maximum supported width.
 * - xlnx,v-csc-max-height: Maximum supported height.
 * - xlnx,max-data-width: Maximum data width.
 * - xlnx,enable-422: Flag to enable 4:2:2 chroma subsampling.
 * - xlnx,enable-420: Flag to enable 4:2:0 chroma subsampling.
 * - xlnx,enable-window: Flag to enable windowing feature.
 *
 * The last entry with NULL is used as a table terminator.
 */

XV_csc_Config XV_csc_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {
	{
		"xlnx,v-csc-1.1", /* compatible */
		0x0, /* reg */
		0x2, /* xlnx,samples-per-clock */
		0xf00, /* xlnx,v-csc-max-width */
		0x870, /* xlnx,v-csc-max-height */
		0xa, /* xlnx,max-data-width */
		0x1, /* xlnx,enable-422 */
		0x1, /* xlnx,enable-420 */
		0x1 /* xlnx,enable-window */
	},
	 {
		 NULL
	}
};
#endif