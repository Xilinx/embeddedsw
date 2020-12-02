/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrgb2ycrcb_selftest.c
* @addtogroup rgb2ycrcb_v7_2
* @{
*
* This file contains the self-test function for the RGB2YCRCB core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 7.0   adk    01/28/14 First Release.
*                       Implemented the following function:
*                       XRgb2YCrCb_SelfTest.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xrgb2ycrcb.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function reads version register of RGB2YCRCB core and compares with zero
* as part of self test.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance.
*
* @return
*		- XST_SUCCESS if the version register read test was successful.
*		- XST_FAILURE if the version register read test failed.
*
* @note		None.
*
******************************************************************************/
int XRgb2YCrCb_SelfTest(XRgb2YCrCb *InstancePtr)
{
	u32 Version;
	int Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read RGB2YCRCB core version register. */
	Version = XRgb2YCrCb_ReadReg(InstancePtr->Config.BaseAddress,
					(XRGB_VERSION_OFFSET));

	/* Compare Version with zero */
	if (Version != (u32)0x0) {
		Status = (XST_SUCCESS);
	}
	else {
		Status = (XST_FAILURE);
	}

	return Status;
}
/** @} */
