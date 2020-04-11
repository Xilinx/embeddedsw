
/*******************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xparameters.h"
#include "xv_hdmirx1.h"

/*
* The configuration table for devices
*/

XV_HdmiRx1_Config XV_HdmiRx1_ConfigTable[XPAR_XV_HDMIRX1_NUM_INSTANCES] =
{
	{
		XPAR_V_HDMI_RXSS1_V_HDMI_RX_DEVICE_ID,
		XPAR_V_HDMI_RXSS1_V_HDMI_RX_BASEADDR,
		XPAR_V_HDMI_RXSS1_V_HDMI_RX_AXI_LITE_FREQ_HZ,
		XPAR_V_HDMI_RXSS1_V_HDMI_RX_LNK_REF_CLK,
		XPAR_V_HDMI_RXSS1_V_HDMI_RX_VID_REF_CLK,
		XPAR_V_HDMI_RXSS1_V_HDMI_RX_DDC_EDID_SIZE
	}
};


