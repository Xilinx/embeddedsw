/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xparameters.h"
#include "xvtc.h"

/*
* The configuration table for devices
*/

XVtc_Config XVtc_ConfigTable[] =
{
#if XPAR_XVTC_NUM_INSTANCES
	{
		XPAR_FMC_HDMI_INPUT_V_TC_1_DEVICE_ID,
		XPAR_FMC_HDMI_INPUT_V_TC_1_BASEADDR
	},
	{
		XPAR_FMC_HDMI_OUTPUT_V_TC_1_DEVICE_ID,
		XPAR_FMC_HDMI_OUTPUT_V_TC_1_BASEADDR
	},
	{
		XPAR_FMC_SENSOR_INPUT_V_TC_1_DEVICE_ID,
		XPAR_FMC_SENSOR_INPUT_V_TC_1_BASEADDR
	},
	{
		XPAR_HDMI_OUTPUT_V_TC_1_DEVICE_ID,
		XPAR_HDMI_OUTPUT_V_TC_1_BASEADDR
	}
#endif
};
