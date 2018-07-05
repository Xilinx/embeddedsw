/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcresample_selftest.c
* @addtogroup cresample_v4_1
* @{
*
* This file contains the self-test functions for the Chroma Resampler core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -------------------------------------------------
* 4.0   adk     03/12/14 First release
*                        Implemented XCresample_SelfTest function.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcresample.h"
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
* This function reads version register of Chroma Resampler core and compares
* with zero as part of self test.
*
* @param	InstancePtr is a pointer to the XCresample instance.
*
* @return
*		- XST_SUCCESS if the test is successful.
*		- XST_FAILURE if the test is failed.
*
* @note		None.
*
******************************************************************************/
int XCresample_SelfTest(XCresample *InstancePtr)
{
	u32 Version;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Version = XCresample_ReadReg(InstancePtr->Config.BaseAddress,
					(XCRE_VERSION_OFFSET));

	/* Comparing Version with zero */
	if (Version != (u32)0x0) {
		Status = (XST_SUCCESS);
	}
	else {
		Status = (XST_FAILURE);
	}

	return Status;
}
/** @} */
