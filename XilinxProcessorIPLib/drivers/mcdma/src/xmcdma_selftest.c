/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* @file xmcdma_selftest.c
* @addtogroup mcdma_v1_0
* @{
*
* This file contains the self-test function for the MCDMA core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   adk     18/07/17 Initial version.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xmcdma.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This file contains a diagnostic self-test function for the MCDMA driver.
* Refer to the header file xmcdma.h for more detailed information.
*
* @param	InstancePtr is a pointer to XMcDma instance.
*
* @return
*		- XST_SUCCESS if the test is successful.
*		- XST_FAILURE if the test is failed.
*
* @note		None.
*
******************************************************************************/
s32 XMcdma_SelfTest(XMcdma *InstancePtr)
{
	u32 TimeOut;

	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Reset the device */
	XMcDma_Reset(InstancePtr);

	TimeOut = XMCDMA_LOOP_COUNT;

	while (TimeOut) {

		if(XMcdma_ResetIsDone(InstancePtr)) {
			break;
		}

		TimeOut -= 1;

	}

	if (!TimeOut) {
		xil_printf("Self Test failed\r\n");

		return XST_FAILURE;
	}

	return (XST_SUCCESS);
}
/** @} */
