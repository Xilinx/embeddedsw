
/*******************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "xparameters.h"
#include "xhdcp22_rx_dp.h"

/*
* The configuration table for devices
*/

XHdcp22_Rx_Dp_Config XHdcp22_Rx_Dp_ConfigTable[] =
{
#if XPAR_DP_RX_HIER_0_V_DP_RXSS1_0_HDCP22_ENABLE
	{
		XPAR_DP_RX_HIER_0_V_DP_RXSS1_0_DP_RX_HDCP22_DEVICE_ID,
		XPAR_DP_RX_HIER_0_V_DP_RXSS1_0_DP_RX_HDCP22_BASEADDR,
		XPAR_DP_RX_HIER_0_V_DP_RXSS1_0_DP_RX_HDCP22_PROTOCOL,
		XPAR_DP_RX_HIER_0_V_DP_RXSS1_0_DP_RX_HDCP22_MODE,
		XPAR_DP_RX_HIER_0_V_DP_RXSS1_0_DP_RX_HDCP22_HDCP22_CIPHER_DP_DEVICE_ID,
		XPAR_DP_RX_HIER_0_V_DP_RXSS1_0_DP_RX_HDCP22_HDCP22_MMULT_DEVICE_ID,
		XPAR_DP_RX_HIER_0_V_DP_RXSS1_0_DP_RX_HDCP22_HDCP22_RNG_DEVICE_ID

	}
#endif
};
