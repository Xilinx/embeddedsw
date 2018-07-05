/*******************************************************************
*
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/
#include "xparameters.h"
#include "xv_sdirx.h"

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
