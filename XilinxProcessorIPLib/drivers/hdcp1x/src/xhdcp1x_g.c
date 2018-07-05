/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#include "xparameters.h"
#include "xhdcp1x.h"

/*
* The configuration table for devices
*/

XHdcp1x_Config XHdcp1x_ConfigTable[] =
{
#if XPAR_XHDCP_NUM_INSTANCES
	{
		XPAR_DP_RX_HIER_DP_RX_SUBSYSTEM_0_DP_RX_HDCP_DEVICE_ID,
		XPAR_DP_RX_HIER_DP_RX_SUBSYSTEM_0_DP_RX_HDCP_BASEADDR,
		XPAR_DP_RX_HIER_DP_RX_SUBSYSTEM_0_DP_RX_HDCP_S_AXI_FREQUENCY,
		XPAR_DP_RX_HIER_DP_RX_SUBSYSTEM_0_DP_RX_HDCP_IS_RX,
		XPAR_DP_RX_HIER_DP_RX_SUBSYSTEM_0_DP_RX_HDCP_IS_HDMI
	},
	{
		XPAR_DP_TX_HIER_DP_TX_SUBSYSTEM_0_DP_TX_HDCP_DEVICE_ID,
		XPAR_DP_TX_HIER_DP_TX_SUBSYSTEM_0_DP_TX_HDCP_BASEADDR,
		XPAR_DP_TX_HIER_DP_TX_SUBSYSTEM_0_DP_TX_HDCP_S_AXI_FREQUENCY,
		XPAR_DP_TX_HIER_DP_TX_SUBSYSTEM_0_DP_TX_HDCP_IS_RX,
		XPAR_DP_TX_HIER_DP_TX_SUBSYSTEM_0_DP_TX_HDCP_IS_HDMI
	}
#endif
};
