/*******************************************************************
* Copyright (C) 2010-2016 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*

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
