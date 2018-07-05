/*******************************************************************
*
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/

#include "xparameters.h"
#include "xv_sditxss.h"

/*
* List of Sub-cores included in the Subsystem for Device ID 0
* Sub-core device id will be set by its driver in xparameters.h
*/

#define XPAR_V_SMPTE_UHDSDI_TX_SS_V_SMPTE_UHDSDI_TX_PRESENT	 1
#define XPAR_V_SMPTE_UHDSDI_TX_SS_V_SMPTE_UHDSDI_TX_ABSOLUTE_BASEADDR	 (XPAR_V_SMPTE_UHDSDI_TX_SS_BASEADDR + XPAR_V_SMPTE_UHDSDI_TX_SS_V_SMPTE_UHDSDI_TX_AXI_CTRL_BASEADDR)

#define XPAR_V_SMPTE_UHDSDI_TX_SS_V_TC_PRESENT	 1
#define XPAR_V_SMPTE_UHDSDI_TX_SS_V_TC_ABSOLUTE_BASEADDR	 (XPAR_V_SMPTE_UHDSDI_TX_SS_BASEADDR + XPAR_V_SMPTE_UHDSDI_TX_SS_V_TC_BASEADDR)


/*
* List of Sub-cores excluded from the subsystem for Device ID 0
*   - Excluded sub-core device id is set to 255
*   - Excluded sub-core baseaddr is set to 0
*/



XV_SdiTxSs_Config XV_SdiTxSs_ConfigTable[XPAR_XV_SDITXSS_NUM_INSTANCES] =
{
	{
		XPAR_V_SMPTE_UHDSDI_TX_SS_DEVICE_ID,
		XPAR_V_SMPTE_UHDSDI_TX_SS_BASEADDR,
		XPAR_V_SMPTE_UHDSDI_TX_SS_INCLUDE_ADV_FEATURES,
		XPAR_V_SMPTE_UHDSDI_TX_SS_LINE_RATE,
		XPAR_V_SMPTE_UHDSDI_TX_SS_TX_INSERT_C_STR_ST352,
		XPAR_V_SMPTE_UHDSDI_TX_SS_BPP,

		{
			XPAR_V_SMPTE_UHDSDI_TX_SS_V_SMPTE_UHDSDI_TX_PRESENT,
			XPAR_V_SMPTE_UHDSDI_TX_SS_V_SMPTE_UHDSDI_TX_DEVICE_ID,
			XPAR_V_SMPTE_UHDSDI_TX_SS_V_SMPTE_UHDSDI_TX_ABSOLUTE_BASEADDR
		},
		{
			XPAR_V_SMPTE_UHDSDI_TX_SS_V_TC_PRESENT,
			XPAR_V_SMPTE_UHDSDI_TX_SS_V_TC_DEVICE_ID,
			XPAR_V_SMPTE_UHDSDI_TX_SS_V_TC_ABSOLUTE_BASEADDR
		},
	}
};
