/******************************************************************************
*
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_sdirx.h"

#ifndef SDT
/*
* The configuration table for devices
*/

XV_SdiRx_Config XV_SdiRx_ConfigTable[XPAR_XV_SDIRX_NUM_INSTANCES] =
{
	{
		XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_DEVICE_ID,
		XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_S_AXI_CTRL_BASEADDR,
		XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_INCLUDE_EDH,
		XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_LINE_RATE
	}
};
#else
XV_SdiRx_Config XV_SdiRx_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,v-smpte-uhdsdi-rx-1.0", /* compatible */
		0x0, /* reg */
		0x0, /* xlnx,include-edh */
		0x2 /* xlnx,line-rate */
	},
	{
		NULL
	}
};
#endif
