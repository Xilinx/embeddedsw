/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
********************************************************************/
/**
 * @file xv_hcresampler_g.c
 * @addtogroup v_hcresampler Overview
 */

#include "xv_hcresampler.h"

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
