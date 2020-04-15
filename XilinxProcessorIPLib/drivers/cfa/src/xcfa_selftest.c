/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcfa_selftest.c
* @addtogroup cfa_v7_1
* @{
*
* This file contains the self-test functions for the CFA core.
* The self test function reads the Version register.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- ----------------------------------------------
* 7.0   adk     01/07/14 First Release
*                        Implemented XCfa_SelfTest function.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcfa.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function reads complete Version register of CFA core and compares
* with zero values as part of self test.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked
*		on.
*
* @return
*		- XST_SUCCESS if the Version register read test was successful.
*		- XST_FAILURE if the Version register read test failed.
*
* @note		None.
*
******************************************************************************/
int XCfa_SelfTest(XCfa *InstancePtr)
{
	u32 Version;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read CFA core version register. */
	Version = XCfa_ReadReg(InstancePtr->Config.BaseAddress,
					(XCFA_VERSION_OFFSET));

	/* Compare version with zero. */
	if (Version != (u32)0x0) {
		Status = (XST_SUCCESS);
	}
	else {
		Status = (XST_FAILURE);
	}

	return Status;
}
/** @} */
