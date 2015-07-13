/******************************************************************************
*
* Copyright (C) 2010 - 2015 Xilinx, Inc.  All rights reserved.
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
* @file xttcps_g.c
* @addtogroup ttcps_v3_0
* @{
*
* This file contains a configuration table where each entry is the
* configuration information for one timer counter device in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00a drg/jz 01/21/10 First release
* 2.00  hk     22/01/14 Added check for picking instances other than
*                       default.
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xttcps.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each TTC device
 * in the system.
 */
XTtcPs_Config XTtcPs_ConfigTable[XPAR_XTTCPS_NUM_INSTANCES] = {
	{
		 (u16)XPAR_XTTCPS_0_DEVICE_ID,	/* Device ID for instance */
		 (u32)XPAR_XTTCPS_0_BASEADDR,	/* Device base address */
		 (u32)XPAR_XTTCPS_0_TTC_CLK_FREQ_HZ	/* Device input clock frequency */
	},
#ifdef XPAR_XTTCPS_1_DEVICE_ID
	{
		 (u16)XPAR_XTTCPS_1_DEVICE_ID,	/* Device ID for instance */
		 (u32)XPAR_XTTCPS_1_BASEADDR,	/* Device base address */
		 (u32)XPAR_XTTCPS_1_CLOCK_HZ	/* Device input clock frequency */
	},
#endif

#ifdef XPAR_XTTCPS_2_DEVICE_ID
	{
		 (u16)XPAR_XTTCPS_2_DEVICE_ID,	/* Device ID for instance */
		 (u32)XPAR_XTTCPS_2_BASEADDR,	/* Device base address */
		 (u32)XPAR_XTTCPS_2_CLOCK_HZ	/* Device input clock frequency */
	},
#endif

#ifdef XPAR_XTTCPS_3_DEVICE_ID
	{
		 (u16)XPAR_XTTCPS_3_DEVICE_ID,	/* Device ID for instance */
		 (u32)XPAR_XTTCPS_3_BASEADDR,	/* Device base address */
		 (u32)XPAR_XTTCPS_3_CLOCK_HZ	/* Device input clock frequency */
	},
#endif

#ifdef XPAR_XTTCPS_4_DEVICE_ID
	{
		 (u16)XPAR_XTTCPS_4_DEVICE_ID,	/* Device ID for instance */
		 (u32)XPAR_XTTCPS_4_BASEADDR,	/* Device base address */
		 (u32)XPAR_XTTCPS_4_CLOCK_HZ	/* Device input clock frequency */
	},
#endif

#ifdef XPAR_XTTCPS_5_DEVICE_ID
	{
		 (u16)XPAR_XTTCPS_5_DEVICE_ID,	/* Device ID for instance */
		 (u32)XPAR_XTTCPS_5_BASEADDR,	/* Device base address */
		 (u32)XPAR_XTTCPS_5_CLOCK_HZ	/* Device input clock frequency */
	},
#endif
};
/** @} */
