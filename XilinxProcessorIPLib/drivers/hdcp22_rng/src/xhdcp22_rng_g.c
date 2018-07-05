/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/


#include "xparameters.h"
#include "xhdcp22_rng.h"

/*
* The configuration table for devices
*/

XHdcp22_Rng_Config XHdcp22_Rng_ConfigTable[] =
{
#if XPAR_XHDCP22_RNG_NUM_INSTANCES
	{
		XPAR_HDCP22_RNG_SS_HDCP22_RNG_0_DEVICE_ID,
		XPAR_HDCP22_RNG_SS_HDCP22_RNG_0_S_AXI_BASEADDR
	}
#endif
};
