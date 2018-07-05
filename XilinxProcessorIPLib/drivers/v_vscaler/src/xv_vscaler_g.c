/*******************************************************************
*
* Copyright (C) 2010-2015 Xilinx, Inc. All rights reserved.
* 
*Permission is hereby granted, free of charge, to any person obtaining a copy
*of this software and associated documentation files (the "Software"), to deal
*in the Software without restriction, including without limitation the rights
*to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*copies of the Software, and to permit persons to whom the Software is
*furnished to do so, subject to the following conditions:
*
*The above copyright notice and this permission notice shall be included in
*all copies or substantial portions of the Software.
*
*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
*XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
*OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*Except as contained in this notice, the name of the Xilinx shall not be used
*in advertising or otherwise to promote the sale, use or other dealings in
*this Software without prior written authorization from Xilinx.
*
*******************************************************************/

#include "xparameters.h"
#include "xv_vscaler.h"

/*
* The configuration table for devices
*/

XV_vscaler_Config XV_vscaler_ConfigTable[] =
{
	{
#ifdef XPAR_XV_VSCALER_NUM_INSTANCES
		XPAR_V_PROC_SS_0_V_VSCALER_DEVICE_ID,
		XPAR_V_PROC_SS_0_V_VSCALER_S_AXI_CTRL_BASEADDR,
		XPAR_V_PROC_SS_0_V_VSCALER_SAMPLES_PER_CLOCK,
		XPAR_V_PROC_SS_0_V_VSCALER_NUM_VIDEO_COMPONENTS,
		XPAR_V_PROC_SS_0_V_VSCALER_MAX_COLS,
		XPAR_V_PROC_SS_0_V_VSCALER_MAX_ROWS,
		XPAR_V_PROC_SS_0_V_VSCALER_MAX_DATA_WIDTH,
		XPAR_V_PROC_SS_0_V_VSCALER_PHASE_SHIFT,
		XPAR_V_PROC_SS_0_V_VSCALER_SCALE_MODE,
		XPAR_V_PROC_SS_0_V_VSCALER_TAPS,
		XPAR_V_PROC_SS_0_V_VSCALER_ENABLE_420
#endif
	}
};
