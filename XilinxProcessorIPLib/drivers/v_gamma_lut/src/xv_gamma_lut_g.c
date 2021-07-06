/*******************************************************************
*
* Copyright (C) 2010-2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


#include "xparameters.h"
#include "xv_gamma_lut.h"

/*
* The configuration table for devices
*/

XV_gamma_lut_Config XV_gamma_lut_ConfigTable[] =
{
	{
#ifdef XPAR_XV_GAMMA_LUT_NUM_INSTANCES
		XPAR_V_GAMMA_LUT_0_DEVICE_ID,
		XPAR_V_GAMMA_LUT_0_S_AXI_CTRL_BASEADDR,
		XPAR_V_GAMMA_LUT_0_SAMPLES_PER_CLOCK,
		XPAR_V_GAMMA_LUT_0_MAX_COLS,
		XPAR_V_GAMMA_LUT_0_MAX_ROWS,
		XPAR_V_GAMMA_LUT_0_MAX_DATA_WIDTH
#endif
	}
};
