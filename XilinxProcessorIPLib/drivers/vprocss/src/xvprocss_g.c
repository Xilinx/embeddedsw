/*******************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xvprocss.h"

#ifndef SDT
/*
* Subsystem Instance: <v_proc_ss_0>
*   - List of sub-cores included in the subsystem
*/

#define XPAR_V_PROC_SS_0_AXI_VDMA_PRESENT	 1
#define XPAR_V_PROC_SS_0_CSC_PRESENT	 1
#define XPAR_V_PROC_SS_0_DINT_PRESENT	 1
#define XPAR_V_PROC_SS_0_HCR_PRESENT	 1
#define XPAR_V_PROC_SS_0_HSC_PRESENT	 1
#define XPAR_V_PROC_SS_0_LTR_PRESENT	 1
#define XPAR_V_PROC_SS_0_RESET_SEL_AXI_MM_PRESENT	 1
#define XPAR_V_PROC_SS_0_RESET_SEL_AXIS_PRESENT	 1
#define XPAR_V_PROC_SS_0_VCR_I_PRESENT	 1
#define XPAR_V_PROC_SS_0_VCR_O_PRESENT	 1
#define XPAR_V_PROC_SS_0_VIDEO_ROUTER_XBAR_PRESENT	 1
#define XPAR_V_PROC_SS_0_VSC_PRESENT	 1


/*
* List of sub-cores excluded from the subsystem <v_proc_ss_0>
*   - Excluded sub-core device id is set to 255
*   - Excluded sub-core base address is set to 0
*/




XVprocSs_Config XVprocSs_ConfigTable[] =
{
	{
		XPAR_V_PROC_SS_0_DEVICE_ID,
		XPAR_V_PROC_SS_0_BASEADDR,
		XPAR_V_PROC_SS_0_HIGHADDR,
		XPAR_V_PROC_SS_0_TOPOLOGY,
		XPAR_V_PROC_SS_0_SAMPLES_PER_CLK,
		XPAR_V_PROC_SS_0_MAX_DATA_WIDTH,
		XPAR_V_PROC_SS_0_NUM_VIDEO_COMPONENTS,
		XPAR_V_PROC_SS_0_MAX_COLS,
		XPAR_V_PROC_SS_0_MAX_ROWS,

		{
			XPAR_V_PROC_SS_0_RESET_SEL_AXI_MM_PRESENT,
			XPAR_V_PROC_SS_0_RESET_SEL_AXI_MM_DEVICE_ID,
			XPAR_V_PROC_SS_0_RESET_SEL_AXI_MM_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_RESET_SEL_AXIS_PRESENT,
			XPAR_V_PROC_SS_0_RESET_SEL_AXIS_DEVICE_ID,
			XPAR_V_PROC_SS_0_RESET_SEL_AXIS_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_AXI_VDMA_PRESENT,
			XPAR_V_PROC_SS_0_AXI_VDMA_DEVICE_ID,
			XPAR_V_PROC_SS_0_AXI_VDMA_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_VIDEO_ROUTER_XBAR_PRESENT,
			XPAR_V_PROC_SS_0_VIDEO_ROUTER_XBAR_DEVICE_ID,
			XPAR_V_PROC_SS_0_VIDEO_ROUTER_XBAR_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_CSC_PRESENT,
			XPAR_V_PROC_SS_0_CSC_DEVICE_ID,
			XPAR_V_PROC_SS_0_CSC_S_AXI_CTRL_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_DINT_PRESENT,
			XPAR_V_PROC_SS_0_DINT_DEVICE_ID,
			XPAR_V_PROC_SS_0_DINT_S_AXI_CTRL_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_HCR_PRESENT,
			XPAR_V_PROC_SS_0_HCR_DEVICE_ID,
			XPAR_V_PROC_SS_0_HCR_S_AXI_CTRL_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_HSC_PRESENT,
			XPAR_V_PROC_SS_0_HSC_DEVICE_ID,
			XPAR_V_PROC_SS_0_HSC_S_AXI_CTRL_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_LTR_PRESENT,
			XPAR_V_PROC_SS_0_LTR_DEVICE_ID,
			XPAR_V_PROC_SS_0_LTR_S_AXI_CTRL_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_VCR_I_PRESENT,
			XPAR_V_PROC_SS_0_VCR_I_DEVICE_ID,
			XPAR_V_PROC_SS_0_VCR_I_S_AXI_CTRL_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_VCR_O_PRESENT,
			XPAR_V_PROC_SS_0_VCR_O_DEVICE_ID,
			XPAR_V_PROC_SS_0_VCR_O_S_AXI_CTRL_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_VSC_PRESENT,
			XPAR_V_PROC_SS_0_VSC_DEVICE_ID,
			XPAR_V_PROC_SS_0_VSC_S_AXI_CTRL_BASEADDR
		},
	}
};
#else
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
#endif
