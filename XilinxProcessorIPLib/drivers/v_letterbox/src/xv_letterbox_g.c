/*******************************************************************
* Copyright (C) 2010-2015 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
********************************************************************/
/**
 * @file xv_letterbox_g.c
 * @addtogroup xv_letterbox Overview
 */

#include "xv_letterbox.h"

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
