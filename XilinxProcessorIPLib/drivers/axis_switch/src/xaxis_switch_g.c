/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xaxis_switch_g.c
* @addtogroup axis_switch_v1_2
* @{
*
* This file gets generated automatically by HSI.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------
* 1.00 sha 07/15/15 Added XPAR_XAXIS_SWITCH_NUM_INSTANCES macro to control
*                   config table parameters. Modified copyright header.
*                   Added doxygen tag and modification history.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xaxis_switch.h"

/************************** Constant Definitions *****************************/

/*
* The configuration table for devices
*/

XAxis_Switch_Config XAxis_Switch_ConfigTable[] =
{
	{
#ifdef XPAR_XAXIS_SWITCH_NUM_INSTANCES
		XPAR_AXIS_SWITCH_0_DEVICE_ID,
		XPAR_AXIS_SWITCH_0_BASEADDR,
		XPAR_AXIS_SWITCH_0_NUM_SI,
		XPAR_AXIS_SWITCH_0_NUM_MI
#endif
	}
};
/** @} */
