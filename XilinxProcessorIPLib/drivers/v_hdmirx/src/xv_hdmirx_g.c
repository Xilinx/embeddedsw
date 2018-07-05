/*******************************************************************
*
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/

#include "xparameters.h"
#include "xv_hdmirx.h"

/*
* The configuration table for devices
*/

XV_HdmiRx_Config XV_HdmiRx_ConfigTable[XPAR_XV_HDMIRX_NUM_INSTANCES] =
{
	{
		XPAR_V_HDMI_RX_SS_V_HDMI_RX_DEVICE_ID,
		XPAR_V_HDMI_RX_SS_V_HDMI_RX_BASEADDR
	}
};
