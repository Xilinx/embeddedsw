/******************************************************************************
*
* Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
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
* @file xdualsplitter_sinit.c
* @addtogroup dual_splitter_v1_1
* @{
*
* This file contains static initialization function for Xilinx Dual Splitter
* core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -------------------------------------------------------
* 1.00  sha 07/21/14 Initial release.
* 1.00  sha 07/08/15 Defined macro XPAR_XDUALSPLITTER_NUM_INSTANCES if not
*                    defined in xparameters.h.
* 1.1   tu  09/18/17 Removed compilation warning in function
*                    XDualSplitter_LookupConfig()
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdualsplitter.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

#ifndef XPAR_XDUALSPLITTER_NUM_INSTANCES
#define XPAR_XDUALSPLITTER_NUM_INSTANCES	0
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to the XDualSplitter_Config structure
* based on the core id, <i>DeviceId</i>. The return value will refer to an
* entry in the device configuration table defined in the xdualsplitter_g.c
* file.
*
* @param	DeviceId is the unique core ID of the Dual Splitter core for
*		the lookup operation.
*
* @return	XDualSplitter_LookupConfig returns a reference to a config
*		record in the configuration table (in xdaulsplitter_g.c)
*		corresponding to <i>DeviceId</i>, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XDualSplitter_Config *XDualSplitter_LookupConfig(u16 DeviceId)
{
	extern XDualSplitter_Config
		XDualSplitter_ConfigTable[XPAR_XDUALSPLITTER_NUM_INSTANCES];
	XDualSplitter_Config *CfgPtr = NULL;
	int Index;

	/* Checking for device id for which instance it is matching */
	for (Index = 0x0; Index < XPAR_XDUALSPLITTER_NUM_INSTANCES; Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XDualSplitter_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDualSplitter_ConfigTable[Index];
			break;
		}
	}

	return (XDualSplitter_Config *)CfgPtr;
}
/** @} */
