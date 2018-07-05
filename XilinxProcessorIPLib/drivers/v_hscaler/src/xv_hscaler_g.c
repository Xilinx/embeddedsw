/*******************************************************************
*
* Copyright (C) 2010-2015 Xilinx, Inc. All rights reserved.
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
#include "xv_hscaler.h"

/*
* The configuration table for devices
*/

XV_hscaler_Config XV_hscaler_ConfigTable[] =
{
	{
#ifdef XPAR_XV_HSCALER_NUM_INSTANCES
		XPAR_V_PROC_SS_0_V_HSCALER_DEVICE_ID,
		XPAR_V_PROC_SS_0_V_HSCALER_S_AXI_CTRL_BASEADDR,
		XPAR_V_PROC_SS_0_V_HSCALER_SAMPLES_PER_CLOCK,
		XPAR_V_PROC_SS_0_V_HSCALER_NUM_VIDEO_COMPONENTS,
		XPAR_V_PROC_SS_0_V_HSCALER_MAX_COLS,
		XPAR_V_PROC_SS_0_V_HSCALER_MAX_ROWS,
		XPAR_V_PROC_SS_0_V_HSCALER_MAX_DATA_WIDTH,
		XPAR_V_PROC_SS_0_V_HSCALER_PHASE_SHIFT,
		XPAR_V_PROC_SS_0_V_HSCALER_SCALE_MODE,
		XPAR_V_PROC_SS_0_V_HSCALER_TAPS,
		XPAR_V_PROC_SS_0_V_HSCALER_ENABLE_422,
		XPAR_V_PROC_SS_0_V_HSCALER_ENABLE_420,
		XPAR_V_PROC_SS_0_V_HSCALER_ENABLE_CSC
#endif
	}
};
