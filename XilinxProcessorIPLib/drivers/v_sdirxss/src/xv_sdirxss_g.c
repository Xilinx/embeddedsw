/******************************************************************************
* Copyright (C) 2010-2017 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#include "xv_sdirxss.h"

/*
* List of Sub-cores included in the Subsystem for Device ID 0
* Sub-core device id will be set by its driver in xparameters.h
*/

#define XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_PRESENT	1
#define XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_ABSOLUTE_BASEADDR	(XPAR_V_SMPTE_UHDSDI_RX_SS_BASEADDR + XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_S_AXI_CTRL_BASEADDR)

XV_SdiRxSs_Config XV_SdiRxSs_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {
	{
		"xlnx,v-smpte-uhdsdi-rx-ss-2.0", /* compatible */
		0x80000000, /* reg */
		0x2, /* xlnx,pixels-per-clock */
		0x2, /* xlnx,line-rate */
		0xa, /* xlnx,bpp */
		{
			0x1, /* sdirx-present */
			0x0, /* sdirx-connected */
		},
		0x4059, /* interrupts */
		0xf9010000 /* interrupt-parent */
	},
	{
		NULL
	}
};
