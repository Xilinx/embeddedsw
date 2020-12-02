/*******************************************************************
*
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/

#include "xparameters.h"
#include "xv_hdmitx.h"

/*
* The configuration table for devices
*/

XV_HdmiTx_Config XV_HdmiTx_ConfigTable[XPAR_XV_HDMITX_NUM_INSTANCES] =
{
	{
		XPAR_V_HDMI_TX_SS_V_HDMI_TX_DEVICE_ID,
		XPAR_V_HDMI_TX_SS_V_HDMI_TX_BASEADDR
	}
};
