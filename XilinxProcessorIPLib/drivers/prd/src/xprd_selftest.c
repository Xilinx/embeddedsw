/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
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
* @file xprd_selftest.c
* @addtogroup prd_v1_0
* @{
*
* This file contains the self-test functions for the XPrd driver.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who     Date	     Changes
* ----- ----- -----------  -----------------------------------------------
* 1.0   ms    07/14/2016    First release.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xprd.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* This function runs a self-test for the PRD driver. This self test reads the
* value from the status register.
*
* @param	InstancePtr is a pointer to the XPrd instance.
*
* @return
*		- XST_SUCCESS if the test was successful.
*
* @note		None.
*
******************************************************************************/
s32 XPrd_SelfTest(XPrd *InstancePtr)
{
	u32 Data;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Read the value from the status register */
	Data = XPrd_ReadReg((InstancePtr->Config.BaseAddress) +
			XPRD_CTRL_OFFSET);

	return XST_SUCCESS;
}
/** @} */
