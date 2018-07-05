/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xycrcb2rgb_selftest.c
* @addtogroup ycrcb2rgb_v7_2
* @{
*
* This file contains the self-test function for the YCRCB2RGB core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ----   -------- -----------------------------------------------
* 7.0   adk    02/03/14 First Release.
*                       Implemented the following function:
*                       XYCrCb2Rgb_SelfTest
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xycrcb2rgb.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function reads Version register of YCRCB2RGB core and compares with
* zero as part of self test.
*
* @param	InstancePtr is a pointer to the XCrCb2Rgb instance.
*
* @return
*		- XST_SUCCESS if the Version register read test was successful.
*		- XST_FAILURE if the Version register read test failed.
*
* @note		None.
*
******************************************************************************/
int XYCrCb2Rgb_SelfTest(XYCrCb2Rgb *InstancePtr)
{
	u32 Version;
	int Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Version = XYCrCb2Rgb_ReadReg(InstancePtr->Config.BaseAddress,
					(XYCC_VERSION_OFFSET));

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
