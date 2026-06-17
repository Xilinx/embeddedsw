/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/
/**
 * @file xv_vcresampler_g.c
 * @addtogroup v_vcresampler Overview
 */
#include "xv_vcresampler.h"

/**
 * @brief Configuration table for XV_vcresampler instances.
 *
 * This table contains the configuration parameters for each instance
 * of the XV_vcresampler hardware IP core. Each entry specifies the
 * hardware properties and capabilities of a particular instance.
 *
 * Fields:
 * - compatible: String identifying the compatible hardware version.
 * - reg: Base address of the hardware instance.
 * - xlnx,samples-per-clock: Number of samples processed per clock cycle.
 * - xlnx,num-video-components: Number of video components supported.
 * - xlnx,max-cols: Maximum number of columns (horizontal resolution).
 * - xlnx,max-rows: Maximum number of rows (vertical resolution).
 * - xlnx,max-data-width: Maximum data width supported.
 * - xlnx,convert-type: Type of color conversion supported.
 * - xlnx,num-v-taps: Number of vertical filter taps.
 *
 * The table is terminated by an entry with a NULL compatible field.
 */
XV_vcresampler_Config XV_vcresampler_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,v-vcresampler-1.1", /* compatible */
		0x0, /* reg */
		0x2, /* xlnx,samples-per-clock */
		0x3, /* xlnx,num-video-components */
		0xf00, /* xlnx,max-cols */
		0x870, /* xlnx,max-rows */
		0xa, /* xlnx,max-data-width */
		0x2, /* xlnx,convert-type */
		0x4 /* xlnx,num-v-taps */
	},
	 {
		 NULL
	}
};
