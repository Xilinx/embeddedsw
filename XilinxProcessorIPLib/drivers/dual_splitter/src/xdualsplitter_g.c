/*******************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT

*******************************************************************************/


#include "xparameters.h"
#include "xdualsplitter.h"

/*
* The configuration table for devices
*/

XDualSplitter_Config XDualSplitter_ConfigTable[] =
{
	{
#ifdef XPAR_XDUALSPLITTER_NUM_INSTANCES
#if XPAR_XDUALSPLITTER_NUM_INSTANCES > 0
		XPAR_DUALSPLITTER_0_DEVICE_ID,
		XPAR_DUALSPLITTER_0_BASEADDR,
		XPAR_DUALSPLITTER_0_ACTIVE_COLS,
		XPAR_DUALSPLITTER_0_ACTIVE_ROWS,
		XPAR_DUALSPLITTER_0_MAX_SEGMENTS,
		XPAR_DUALSPLITTER_0_AXIS_VIDEO_MAX_TDATA_WIDTH,
		XPAR_DUALSPLITTER_0_AXIS_VIDEO_MAX_ITDATASMPLS_PER_CLK,
		XPAR_DUALSPLITTER_0_AXIS_VIDEO_MAX_OTDATASMPLS_PER_CLK,
		XPAR_DUALSPLITTER_0_MAX_OVRLAP,
		XPAR_DUALSPLITTER_0_MAX_SMPL_WIDTH,
		XPAR_DUALSPLITTER_0_HAS_AXI4_LITE,
		XPAR_DUALSPLITTER_0_HAS_IRQ
#endif
#endif
	}
};
