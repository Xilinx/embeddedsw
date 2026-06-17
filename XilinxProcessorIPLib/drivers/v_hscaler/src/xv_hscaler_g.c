/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
********************************************************************/

/**
 * @file xv_hscaler_g.c
 * @addtogroup v_hscaler Overview
 */

#include "xv_hscaler.h"

/**
 * @brief Configuration table for XV_hscaler instances.
 *
 * This table contains the configuration parameters for each instance of the XV_hscaler
 * hardware core. Each entry describes the hardware properties and capabilities of a
 * specific XV_hscaler device.
 *
 * Fields:
 * - compatible: Device compatibility string.
 * - reg: Base address of the hardware instance.
 * - xlnx,samples-per-clock: Number of samples processed per clock cycle.
 * - xlnx,num-video-components: Number of video components supported.
 * - xlnx,max-cols: Maximum number of columns (horizontal resolution).
 * - xlnx,max-rows: Maximum number of rows (vertical resolution).
 * - xlnx,max-data-width: Maximum data width per component.
 * - xlnx,phase-shift: Phase shift value for scaling.
 * - xlnx,scale-mode: Supported scaling mode.
 * - xlnx,taps: Number of filter taps.
 * - xlnx,enable-422: Enable support for YUV 4:2:2 format.
 * - xlnx,enable-420: Enable support for YUV 4:2:0 format.
 * - xlnx,enable-csc: Enable color space conversion.
 *
 * The table is placed in the ".drvcfg_sec" section for driver configuration.
 */

XV_hscaler_Config XV_hscaler_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,v-hscaler-1.1", /* compatible */
		0x40000, /* reg */
		0x2, /* xlnx,samples-per-clock */
		0x3, /* xlnx,num-video-components */
		0xf00, /* xlnx,max-cols */
		0x870, /* xlnx,max-rows */
		0xa, /* xlnx,max-data-width */
		0x6, /* xlnx,phase-shift */
		0x2, /* xlnx,scale-mode */
		0x6, /* xlnx,taps */
		0x1, /* xlnx,enable-422 */
		0x1, /* xlnx,enable-420 */
		0x0 /* xlnx,enable-csc */
	},
	 {
		 NULL
	}
};
