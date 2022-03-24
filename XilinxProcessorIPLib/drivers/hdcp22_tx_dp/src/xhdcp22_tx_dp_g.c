
/*******************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "xparameters.h"
#include "xhdcp22_tx_dp.h"

/*
* The configuration table for devices
*/

XHdcp22_Tx_Dp_Config XHdcp22_Tx_Dp_ConfigTable[] =
{
#if XPAR_DP_TX_HIER_0_V_DP_TXSS1_0_HDCP22_ENABLE
	{
		XPAR_DP_TX_HIER_0_V_DP_TXSS1_0_DP_TX_HDCP22_DEVICE_ID,
		XPAR_DP_TX_HIER_0_V_DP_TXSS1_0_DP_TX_HDCP22_BASEADDR,
		XPAR_DP_TX_HIER_0_V_DP_TXSS1_0_DP_TX_HDCP22_PROTOCOL,
		XPAR_DP_TX_HIER_0_V_DP_TXSS1_0_DP_TX_HDCP22_MODE,
		XPAR_DP_TX_HIER_0_V_DP_TXSS1_0_DP_TX_HDCP22_HDCP22_CIPHER_DP_DEVICE_ID,
		XPAR_DP_TX_HIER_0_V_DP_TXSS1_0_DP_TX_HDCP22_HDCP22_RNG_DEVICE_ID

	}
#endif
};
