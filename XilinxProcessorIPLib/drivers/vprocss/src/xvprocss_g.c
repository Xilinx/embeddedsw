/*******************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

/**
* @file xvprocss_g.c
* @addtogroup vprocss Overview
*/

#include "xvprocss.h"

XVprocSs_Config XVprocSs_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,v-proc-ss-2.3", /* compatible */
		0xa0100000, /* reg */
		0xa01fffff, /* xlnx,highaddr */
		0x1, /* xlnx,topology */
		0x2, /* xlnx,samples-per-clk */
		0xa, /* xlnx,max-data-width */
		0x3, /* xlnx,num-video-components */
		0xf00, /* xlnx,max-cols */
		0x870, /* xlnx,max-rows */
		0x1, /* xlnx,deint-motion-adaptive */
		0x1, /* rstaximm-present */
		0x60000, /* rstaximm-connected */
		0x1, /* rstaxis-present */
		0x70000, /* rstaxis-connected */
		0x1, /* vdma-present */
		0x0, /* vdma-connected */
		0x1, /* router-present */
		0xa0000, /* router-connected */
		0x1, /* csc-present */
		0x10000, /* csc-connected */
		0x1, /* deint-present */
		0x20000, /* deint-connected */
		0x1, /* hcrsmplr-present */
		0x30000, /* hcrsmplr-connected */
		0x1, /* hscale-present */
		0x40000, /* hscale-connected */
		0x1, /* lbox-present */
		0x50000, /* lbox-connected */
		0x1, /* vcrsmplrin-present */
		0x80000, /* vcrsmplrin-connected */
		0x1, /* vcrsmplrout-present */
		0x90000, /* vcrsmplrout-connected */
		0x1, /* vscale-present */
		0xb0000 /* vscale-connected */
	},
	 {
		 NULL
	}
};
