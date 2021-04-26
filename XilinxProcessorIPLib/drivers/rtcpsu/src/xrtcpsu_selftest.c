/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xrtcpsu_selftest.c
* @addtogroup rtcpsu_v1_11
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
s32 XRtcPsu_SelfTest(XRtcPsu *InstancePtr)
{
	s32 Status = (s32)XST_SUCCESS;
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
		Status = (s32)XST_FAILURE;
	}

	return Status;
}
/** @} */
