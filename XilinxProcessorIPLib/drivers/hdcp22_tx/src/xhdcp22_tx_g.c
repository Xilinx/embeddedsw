/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/


#include "xparameters.h"
#include "xhdcp22_tx.h"

XHdcp22_Tx_Config XHdcp22_Tx_ConfigTable[] =
{
#if XPAR_XHDCP22_TX_NUM_INSTANCES
	{
		XPAR_V_HDMI_TX_SS_0_HDCP22_TX_0_DEVICE_ID,
		XPAR_V_HDMI_TX_SS_0_HDCP22_TX_0_BASEADDR,
		XPAR_V_HDMI_TX_SS_0_HDCP22_TX_0_PROTOCOL,
		XPAR_V_HDMI_TX_SS_0_HDCP22_TX_0_MODE,
		XPAR_V_HDMI_TX_SS_0_HDCP22_TX_0_HDCP22_TIMER_DEVICE_ID,
		XPAR_V_HDMI_TX_SS_0_HDCP22_TX_0_HDCP22_CIPHER_DEVICE_ID,
		XPAR_V_HDMI_TX_SS_0_HDCP22_TX_0_HDCP22_RNG_DEVICE_ID
	}
#endif
};
