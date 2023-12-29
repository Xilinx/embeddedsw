/******************************************************************************
* Copyright (C) 2010-2017 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_sdirxss.h"

/*
* List of Sub-cores included in the Subsystem for Device ID 0
* Sub-core device id will be set by its driver in xparameters.h
*/

#define XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_PRESENT	1
#define XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_ABSOLUTE_BASEADDR	(XPAR_V_SMPTE_UHDSDI_RX_SS_BASEADDR + XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_S_AXI_CTRL_BASEADDR)

#ifndef SDT
/*
* List of Sub-cores excluded from the subsystem for Device ID 0
*	- Excluded sub-core device id is set to 255
*	- Excluded sub-core baseaddr is set to 0
*/



XV_SdiRxSs_Config XV_SdiRxSs_ConfigTable[XPAR_XV_SDIRXSS_NUM_INSTANCES] =
{
	{
		XPAR_V_SMPTE_UHDSDI_RX_SS_DEVICE_ID,
		XPAR_V_SMPTE_UHDSDI_RX_SS_BASEADDR,
		(XVidC_PixelsPerClock) XPAR_V_SMPTE_UHDSDI_RX_SS_PIXELS_PER_CLOCK,
		XPAR_V_SMPTE_UHDSDI_RX_SS_LINE_RATE,
		XPAR_V_SMPTE_UHDSDI_RX_SS_BPP,

		{
			XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_PRESENT,
			XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_DEVICE_ID,
			XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_ABSOLUTE_BASEADDR
		},
	}
};
#else
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
