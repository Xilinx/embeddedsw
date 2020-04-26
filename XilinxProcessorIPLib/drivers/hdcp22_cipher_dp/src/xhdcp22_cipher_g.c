/*******************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "xparameters.h"
#include "xhdcp22_cipher.h"

/*
* The configuration table for devices
*/

XHdcp22_Cipher_Config XHdcp22_Cipher_ConfigTable[XPAR_XHDCP22_CIPHER_NUM_INSTANCES] =
{
	{
		XPAR_DP_RX_HIER_0_V_DP_RXSS1_0_DP_RX_HDCP22_HDCP22_CIPHER_DP_DEVICE_ID,
		XPAR_DP_RX_HIER_0_V_DP_RXSS1_0_DP_RX_HDCP22_HDCP22_CIPHER_DP_BASEADDR
	}
};
