/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xosd_selftest.c
* @addtogroup osd_v4_0
* @{
*
* This file contains the self-test function for the OSD core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 4.0   adk    02/18/14 First Release.
*                       Implemented the following functions:
*                       XOsd_SelfTest.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xosd.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function reads Version register of OSD core and compares with zero
* as part of self test.
*
* @param	InstancePtr is a pointer to the XOsd instance.
*
* @return	- XST_SUCCESS if the Version register read test was successful.
*		- XST_FAILURE if the Version register read test failed.
*
* @note		None.
*
******************************************************************************/
int XOsd_SelfTest(XOsd *InstancePtr)
{
	u32 Version;
	int Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read OSD core Version register. */
	Version = XOsd_ReadReg(InstancePtr->Config.BaseAddress,
					(XOSD_VER_OFFSET));

	/* Compare Version with zero. */
	if (Version != (u32)0x0) {
		Status = (XST_SUCCESS);
	}
	else {
		Status = (XST_FAILURE);
	}

	return Status;
}
/** @} */
