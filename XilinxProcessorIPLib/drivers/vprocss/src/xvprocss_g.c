/*******************************************************************
*
* Copyright (C) 2010-2016 Xilinx, Inc. All rights reserved.
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
* Use of the Software is limited solely to applications:
*(a) running on a Xilinx device, or
*(b) that interact with a Xilinx device through a bus or interconnect.
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
#include "xvprocss.h"

/*
* Subsystem Instance: <v_proc_ss_0>
*   - List of sub-cores included in the subsystem
*/

#define XPAR_V_PROC_SS_0_AXI_VDMA_PRESENT	 1
#define XPAR_V_PROC_SS_0_CSC_PRESENT	 1
#define XPAR_V_PROC_SS_0_DINT_PRESENT	 1
#define XPAR_V_PROC_SS_0_HCR_PRESENT	 1
#define XPAR_V_PROC_SS_0_HSC_PRESENT	 1
#define XPAR_V_PROC_SS_0_LTR_PRESENT	 1
#define XPAR_V_PROC_SS_0_RESET_SEL_AXI_MM_PRESENT	 1
#define XPAR_V_PROC_SS_0_RESET_SEL_AXIS_PRESENT	 1
#define XPAR_V_PROC_SS_0_VCR_I_PRESENT	 1
#define XPAR_V_PROC_SS_0_VCR_O_PRESENT	 1
#define XPAR_V_PROC_SS_0_VIDEO_ROUTER_XBAR_PRESENT	 1
#define XPAR_V_PROC_SS_0_VSC_PRESENT	 1


/*
* List of sub-cores excluded from the subsystem <v_proc_ss_0>
*   - Excluded sub-core device id is set to 255
*   - Excluded sub-core base address is set to 0
*/




XVprocSs_Config XVprocSs_ConfigTable[] =
{
	{
		XPAR_V_PROC_SS_0_DEVICE_ID,
		XPAR_V_PROC_SS_0_BASEADDR,
		XPAR_V_PROC_SS_0_HIGHADDR,
		XPAR_V_PROC_SS_0_TOPOLOGY,
		XPAR_V_PROC_SS_0_SAMPLES_PER_CLK,
		XPAR_V_PROC_SS_0_MAX_DATA_WIDTH,
		XPAR_V_PROC_SS_0_NUM_VIDEO_COMPONENTS,
		XPAR_V_PROC_SS_0_MAX_COLS,
		XPAR_V_PROC_SS_0_MAX_ROWS,

		{
			XPAR_V_PROC_SS_0_RESET_SEL_AXI_MM_PRESENT,
			XPAR_V_PROC_SS_0_RESET_SEL_AXI_MM_DEVICE_ID,
			XPAR_V_PROC_SS_0_RESET_SEL_AXI_MM_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_RESET_SEL_AXIS_PRESENT,
			XPAR_V_PROC_SS_0_RESET_SEL_AXIS_DEVICE_ID,
			XPAR_V_PROC_SS_0_RESET_SEL_AXIS_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_AXI_VDMA_PRESENT,
			XPAR_V_PROC_SS_0_AXI_VDMA_DEVICE_ID,
			XPAR_V_PROC_SS_0_AXI_VDMA_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_VIDEO_ROUTER_XBAR_PRESENT,
			XPAR_V_PROC_SS_0_VIDEO_ROUTER_XBAR_DEVICE_ID,
			XPAR_V_PROC_SS_0_VIDEO_ROUTER_XBAR_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_CSC_PRESENT,
			XPAR_V_PROC_SS_0_CSC_DEVICE_ID,
			XPAR_V_PROC_SS_0_CSC_S_AXI_CTRL_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_DINT_PRESENT,
			XPAR_V_PROC_SS_0_DINT_DEVICE_ID,
			XPAR_V_PROC_SS_0_DINT_S_AXI_CTRL_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_HCR_PRESENT,
			XPAR_V_PROC_SS_0_HCR_DEVICE_ID,
			XPAR_V_PROC_SS_0_HCR_S_AXI_CTRL_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_HSC_PRESENT,
			XPAR_V_PROC_SS_0_HSC_DEVICE_ID,
			XPAR_V_PROC_SS_0_HSC_S_AXI_CTRL_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_LTR_PRESENT,
			XPAR_V_PROC_SS_0_LTR_DEVICE_ID,
			XPAR_V_PROC_SS_0_LTR_S_AXI_CTRL_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_VCR_I_PRESENT,
			XPAR_V_PROC_SS_0_VCR_I_DEVICE_ID,
			XPAR_V_PROC_SS_0_VCR_I_S_AXI_CTRL_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_VCR_O_PRESENT,
			XPAR_V_PROC_SS_0_VCR_O_DEVICE_ID,
			XPAR_V_PROC_SS_0_VCR_O_S_AXI_CTRL_BASEADDR
		},
		{
			XPAR_V_PROC_SS_0_VSC_PRESENT,
			XPAR_V_PROC_SS_0_VSC_DEVICE_ID,
			XPAR_V_PROC_SS_0_VSC_S_AXI_CTRL_BASEADDR
		},
	}
};
