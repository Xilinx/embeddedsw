/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/


#include "xparameters.h"
#include "xhdcp22_mmult.h"

/*
* The configuration table for devices
*/

XHdcp22_mmult_Config XHdcp22_mmult_ConfigTable[] =
{
#if XPAR_XHDCP22_MMULT_NUM_INSTANCES
	{
		XPAR_V_HDMI_RX_SS_0_HDCP22_RX_SS_HDCP22_MMULT_DEVICE_ID,
		XPAR_V_HDMI_RX_SS_0_HDCP22_RX_SS_HDCP22_MMULT_BASEADDR
	}
#endif
};
