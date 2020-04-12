/*******************************************************************
*
* Copyright (C) 2010-2017 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/

#include "xparameters.h"
#include "xv_sditx.h"

/*
* The configuration table for devices
*/

XV_SdiTx_Config XV_SdiTx_ConfigTable[XPAR_XV_SDITX_NUM_INSTANCES] =
{
	{
		XPAR_V_SMPTE_UHDSDI_TX_SS_V_SMPTE_UHDSDI_TX_DEVICE_ID,
		XPAR_V_SMPTE_UHDSDI_TX_SS_V_SMPTE_UHDSDI_TX_AXI_CTRL_BASEADDR,
		XPAR_V_SMPTE_UHDSDI_TX_SS_V_SMPTE_UHDSDI_TX_INCLUDE_EDH,
		XPAR_V_SMPTE_UHDSDI_TX_SS_V_SMPTE_UHDSDI_TX_LINE_RATE,
		XPAR_V_SMPTE_UHDSDI_TX_SS_V_SMPTE_UHDSDI_TX_TX_INSERT_C_STR_ST352
	}
};
