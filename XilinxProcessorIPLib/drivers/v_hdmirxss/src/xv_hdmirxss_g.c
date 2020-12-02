/*******************************************************************
*
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/

#include "xparameters.h"
#include "xv_hdmirxss.h"

/*
* List of Sub-cores included in the Subsystem for Device ID 0
* Sub-core device id will be set by its driver in xparameters.h
*/

#define XPAR_V_HDMI_RX_SS_AXI_TIMER_PRESENT	 1
#define XPAR_V_HDMI_RX_SS_AXI_TIMER_ABSOLUTE_BASEADDR	 (XPAR_V_HDMI_RX_SS_BASEADDR + XPAR_V_HDMI_RX_SS_AXI_TIMER_BASEADDR)

#define XPAR_V_HDMI_RX_SS_HDCP22_RX_SS_PRESENT	 1
#define XPAR_V_HDMI_RX_SS_HDCP22_RX_SS_ABSOLUTE_BASEADDR	 (XPAR_V_HDMI_RX_SS_BASEADDR + XPAR_V_HDMI_RX_SS_HDCP22_RX_SS_BASEADDR)

#define XPAR_V_HDMI_RX_SS_HDCP_1_4_PRESENT	 1
#define XPAR_V_HDMI_RX_SS_HDCP_1_4_ABSOLUTE_BASEADDR	 (XPAR_V_HDMI_RX_SS_BASEADDR + XPAR_V_HDMI_RX_SS_HDCP_1_4_BASEADDR)

#define XPAR_V_HDMI_RX_SS_V_HDMI_RX_PRESENT	 1
#define XPAR_V_HDMI_RX_SS_V_HDMI_RX_ABSOLUTE_BASEADDR	 (XPAR_V_HDMI_RX_SS_BASEADDR + XPAR_V_HDMI_RX_SS_V_HDMI_RX_BASEADDR)


/*
* List of Sub-cores excluded from the subsystem for Device ID 0
*   - Excluded sub-core device id is set to 255
*   - Excluded sub-core baseaddr is set to 0
*/



XV_HdmiRxSs_Config XV_HdmiRxSs_ConfigTable[XPAR_XV_HDMIRXSS_NUM_INSTANCES] =
{
	{
		XPAR_V_HDMI_RX_SS_DEVICE_ID,
		XPAR_V_HDMI_RX_SS_BASEADDR,
		XPAR_V_HDMI_RX_SS_HIGHADDR,
		(XVidC_PixelsPerClock) XPAR_V_HDMI_RX_SS_INPUT_PIXELS_PER_CLOCK,
		XPAR_V_HDMI_RX_SS_MAX_BITS_PER_COMPONENT,

		{
			XPAR_V_HDMI_RX_SS_AXI_TIMER_PRESENT,
			XPAR_V_HDMI_RX_SS_AXI_TIMER_DEVICE_ID,
			XPAR_V_HDMI_RX_SS_AXI_TIMER_ABSOLUTE_BASEADDR
		},
		{
			XPAR_V_HDMI_RX_SS_HDCP_1_4_PRESENT,
			XPAR_V_HDMI_RX_SS_HDCP_1_4_DEVICE_ID,
			XPAR_V_HDMI_RX_SS_HDCP_1_4_ABSOLUTE_BASEADDR
		},
		{
			XPAR_V_HDMI_RX_SS_HDCP22_RX_SS_PRESENT,
			XPAR_V_HDMI_RX_SS_HDCP22_RX_SS_DEVICE_ID,
			XPAR_V_HDMI_RX_SS_HDCP22_RX_SS_ABSOLUTE_BASEADDR
		},
		{
			XPAR_V_HDMI_RX_SS_V_HDMI_RX_PRESENT,
			XPAR_V_HDMI_RX_SS_V_HDMI_RX_DEVICE_ID,
			XPAR_V_HDMI_RX_SS_V_HDMI_RX_ABSOLUTE_BASEADDR
		},
	}
};
