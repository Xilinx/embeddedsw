/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/


#include "xparameters.h"
#include "xhdcp22_rx.h"

/*
* The configuration table for devices
*/

XHdcp22_Rx_Config XHdcp22_Rx_ConfigTable[] =
{
#if XPAR_XHDCP22_RX_NUM_INSTANCES
	{
		XPAR_V_HDMI_RX_SS_0_HDCP22_RX_0_DEVICE_ID,
		XPAR_V_HDMI_RX_SS_0_HDCP22_RX_0_BASEADDR,
		XPAR_V_HDMI_RX_SS_0_HDCP22_RX_0_PROTOCOL,
		XPAR_V_HDMI_RX_SS_0_HDCP22_RX_0_MODE,
		XPAR_V_HDMI_RX_SS_0_HDCP22_RX_0_HDCP22_TIMER_DEVICE_ID,
		XPAR_V_HDMI_RX_SS_0_HDCP22_RX_0_HDCP22_CIPHER_DEVICE_ID,
		XPAR_V_HDMI_RX_SS_0_HDCP22_RX_0_HDCP22_MMULT_DEVICE_ID,
		XPAR_V_HDMI_RX_SS_0_HDCP22_RX_0_HDCP22_RNG_DEVICE_ID
	}
#endif
};
