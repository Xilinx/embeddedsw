/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/**
* @file xv_deinterlacer_g.c
* @addtogroup v_deinterlacer Overview
*/
#include "xv_deinterlacer.h"

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
