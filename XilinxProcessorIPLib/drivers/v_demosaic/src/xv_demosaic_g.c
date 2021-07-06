/*******************************************************************

* Copyright (C) 2010-2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/
#include "xparameters.h"
#include "xv_demosaic.h"

/*
* The configuration table for devices
*/

XV_demosaic_Config XV_demosaic_ConfigTable[] =
{
	{
#ifdef XPAR_XV_DEMOSAIC_NUM_INSTANCES
		XPAR_V_DEMOSAIC_0_DEVICE_ID,
		XPAR_V_DEMOSAIC_0_S_AXI_CTRL_BASEADDR,
		XPAR_V_DEMOSAIC_0_SAMPLES_PER_CLOCK,
		XPAR_V_DEMOSAIC_0_MAX_COLS,
		XPAR_V_DEMOSAIC_0_MAX_ROWS,
		XPAR_V_DEMOSAIC_0_MAX_DATA_WIDTH,
		XPAR_V_DEMOSAIC_0_ALGORITHM
#endif
	}
};
