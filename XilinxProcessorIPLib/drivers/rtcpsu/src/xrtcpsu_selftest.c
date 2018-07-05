/******************************************************************************
*
* Copyright (C) 2015 - 2019 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/
/****************************************************************************/
/**
*
* @file xrtcpsu_selftest.c
* @addtogroup rtcpsu_v1_8
* @{
*
* This file contains the self-test functions for the XRtcPsu driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- ------ -------- -----------------------------------------------
* 1.00  kvn    04/21/15 First release.
* 1.7   sne    03/01/19 Added Versal support.
* 1.7   sne    03/01/19 Fixed violations according to MISRAC-2012 standards
*                       modified the code such as
*                       No brackets to loop body,Declared the poiner param
*                       as Pointer to const,No brackets to then/else,
*                       Literal value requires a U suffix,Casting operation to a pointer
*                       Array has no bounds specified,Logical conjunctions need brackets.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xrtcpsu.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Variable Definitions *****************************/


/************************** Function Prototypes ******************************/


/****************************************************************************/
/**
*
* This function runs a self-test on the driver and hardware device. This self
* test writes reset value into safety check register and read backs the same.
* If mismatch offers, returns the failure.
*
* @param	InstancePtr is a pointer to the XRtcPsu instance
*
* @return
*		 - XST_SUCCESS if the test was successful
*
* @note
*
* This function can hang if the hardware is not functioning properly.
*
******************************************************************************/
s32 XRtcPsu_SelfTest(const XRtcPsu *InstancePtr)
{
	s32 Status = XST_SUCCESS;
	u32 SafetyCheck;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write the reset value in safety check register and
	 * try reading back. If mismatch happens, return failure.
	 */
	XRtcPsu_WriteReg(InstancePtr->RtcConfig.BaseAddr + XRTC_SFTY_CHK_OFFSET,
			XRTC_SFTY_CHK_RSTVAL);
	SafetyCheck = XRtcPsu_ReadReg(InstancePtr->RtcConfig.BaseAddr +
			XRTC_SFTY_CHK_OFFSET);

	if (SafetyCheck != XRTC_SFTY_CHK_RSTVAL) {
		Status = XST_FAILURE;
	}

	return Status;
}
/** @} */
