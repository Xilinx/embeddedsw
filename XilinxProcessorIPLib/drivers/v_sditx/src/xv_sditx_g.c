/*******************************************************************
* Copyright (C) 2010-2017 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_sditx.h"

#ifndef SDT
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
#else
XV_SdiTx_Config XV_SdiTx_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {
	{
		"xlnx,v-smpte-uhdsdi-tx-1.0", /* compatible */
		0x0, /* reg */
		0x1, /* xlnx,include-edh */
		0x2, /* xlnx,line-rate */
		0x0 /* xlnx,c_str_st352 */
	},
	{
		NULL
	}
};
#endif
