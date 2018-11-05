/*******************************************************************
*
* Copyright (C) 2010-2016 Xilinx, Inc. All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*******************************************************************/

#include "xparameters.h"
#include "xclk_wiz.h"

/*
* The configuration table for devices
*/

XClk_Wiz_Config XClk_Wiz_ConfigTable[] =
{
	{
		XPAR_CLK_MONITOR_DEVICE_ID,
		XPAR_CLK_MONITOR_BASEADDR,
		XPAR_CLK_MONITOR_ENABLE_CLOCK_MONITOR,
		XPAR_CLK_MONITOR_ENABLE_USER_CLOCK0,
		XPAR_CLK_MONITOR_ENABLE_USER_CLOCK1,
		XPAR_CLK_MONITOR_ENABLE_USER_CLOCK2,
		XPAR_CLK_MONITOR_ENABLE_USER_CLOCK3,
		XPAR_CLK_MONITOR_REF_CLK_FREQ,
		XPAR_CLK_MONITOR_USER_CLK_FREQ0,
		XPAR_CLK_MONITOR_USER_CLK_FREQ1,
		XPAR_CLK_MONITOR_USER_CLK_FREQ2,
		XPAR_CLK_MONITOR_USER_CLK_FREQ3,
		XPAR_CLK_MONITOR_PRECISION,
		XPAR_CLK_MONITOR_ENABLE_PLL0,
		XPAR_CLK_MONITOR_ENABLE_PLL1
	},
	{
		XPAR_CLK_WIZ_DYN_RECONFIG_DEVICE_ID,
		XPAR_CLK_WIZ_DYN_RECONFIG_BASEADDR,
		XPAR_CLK_WIZ_DYN_RECONFIG_ENABLE_CLOCK_MONITOR,
		XPAR_CLK_WIZ_DYN_RECONFIG_ENABLE_USER_CLOCK0,
		XPAR_CLK_WIZ_DYN_RECONFIG_ENABLE_USER_CLOCK1,
		XPAR_CLK_WIZ_DYN_RECONFIG_ENABLE_USER_CLOCK2,
		XPAR_CLK_WIZ_DYN_RECONFIG_ENABLE_USER_CLOCK3,
		XPAR_CLK_WIZ_DYN_RECONFIG_REF_CLK_FREQ,
		XPAR_CLK_WIZ_DYN_RECONFIG_USER_CLK_FREQ0,
		XPAR_CLK_WIZ_DYN_RECONFIG_USER_CLK_FREQ1,
		XPAR_CLK_WIZ_DYN_RECONFIG_USER_CLK_FREQ2,
		XPAR_CLK_WIZ_DYN_RECONFIG_USER_CLK_FREQ3,
		XPAR_CLK_WIZ_DYN_RECONFIG_PRECISION,
		XPAR_CLK_WIZ_DYN_RECONFIG_ENABLE_PLL0,
		XPAR_CLK_WIZ_DYN_RECONFIG_ENABLE_PLL1
	}
};
