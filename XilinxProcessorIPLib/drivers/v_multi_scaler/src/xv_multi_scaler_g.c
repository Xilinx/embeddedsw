
/*******************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "xparameters.h"
#include "xv_multi_scaler.h"

/*
* The configuration table for devices
*/

XV_multi_scaler_Config XV_multi_scaler_ConfigTable
	[XPAR_XV_MULTI_SCALER_NUM_INSTANCES] = {
	{
		XPAR_V_MULTI_SCALER_0_DEVICE_ID,
		XPAR_V_MULTI_SCALER_0_S_AXI_CTRL_BASEADDR,
		XPAR_V_MULTI_SCALER_0_SAMPLES_PER_CLOCK,
		XPAR_V_MULTI_SCALER_0_MAX_DATA_WIDTH,
		XPAR_V_MULTI_SCALER_0_MAX_COLS,
		XPAR_V_MULTI_SCALER_0_MAX_ROWS,
		XPAR_V_MULTI_SCALER_0_PHASE_SHIFT,
		XPAR_V_MULTI_SCALER_0_SCALE_MODE,
		XPAR_V_MULTI_SCALER_0_TAPS,
		XPAR_V_MULTI_SCALER_0_MAX_OUTS
	}
};
