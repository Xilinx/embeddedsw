/******************************************************************************
*
* Copyright (C) 2012 - 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xaxipmon_g.c
* @addtogroup axipmon_v6_6
* @{
*
* This file contains a configuration table that specifies the configuration
* of AxiMon devices in the system.
*
* See xaxipmon.h for more information about this driver.
*
* @note None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a bss    02/22/12 First release
* 2.00a bss    06/23/12 Updated to support v2_00a version of IP.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxipmon.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each System Monitor/ADC
 * device in the system.
 */
XAxiPmon_Config XAxiPmon_ConfigTable[XPAR_XAXIPMON_NUM_INSTANCES] =
{
	{
		XPAR_AXI_PERF_MON_0_DEVICE_ID,
		XPAR_AXI_PERF_MON_0_BASEADDR,
		XPAR_AXI_PERF_MON_0_GLOBAL_COUNT_WIDTH,
		XPAR_AXI_PERF_MON_0_METRICS_SAMPLE_COUNT_WIDTH
	}
};
/** @} */
