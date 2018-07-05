/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xvtc_selftest.c
* @addtogroup vtc_v8_2
* @{
*
* This file contains the self test function for the VTC core.
* The self test function reads the Version register.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 6.1   adk    08/23/14 First Release.
*                       Implemented following function:
*                       XVtc_SelfTest.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xvtc.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function reads version register of the VTC core and compares with zero
* as part of self test.
*
* @param	InstancePtr is a pointer to the XVtc instance.
*
* @return
*		- XST_SUCCESS if the Version register read test was successful.
*		- XST_FAILURE if the Version register read test failed.
*
* @note		None.
*
******************************************************************************/
int XVtc_SelfTest(XVtc *InstancePtr)
{
	u32 Version;
	int Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read VTC core version register. */
	Version = XVtc_ReadReg((InstancePtr)->Config.BaseAddress,
					(XVTC_VER_OFFSET));

	/* Compare version with zero */
	if(Version != (u32)0x0) {
		Status = (u32)(XST_SUCCESS);
	}
	else {
		Status = (u32)(XST_FAILURE);
	}

	return Status;
}
/** @} */
