/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/


#include "xparameters.h"
#include "xhdcp22_cipher.h"

/*
* The configuration table for devices
*/

XHdcp22_Cipher_Config XHdcp22_Cipher_ConfigTable[] =
{
#if XPAR_XHDCP22_CIPHER_NUM_INSTANCES
	{
		XPAR_HDMI_RX_SS_HDCP22_RX_SS_V_HDCP22_CIPHER_RX_0_DEVICE_ID,
		XPAR_HDMI_RX_SS_HDCP22_RX_SS_V_HDCP22_CIPHER_RX_0_CPU_BASEADDR
	},
	{
		XPAR_HDMI_TX_SS_HDCP22_TX_SS_V_HDCP22_CIPHER_TX_0_DEVICE_ID,
		XPAR_HDMI_TX_SS_HDCP22_TX_SS_V_HDCP22_CIPHER_TX_0_CPU_BASEADDR
	}
#endif
};
