/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdeint_selftest.c
* @addtogroup deinterlacer_v3_4
* @{
*
* This file contains the self-test functions for the XCfa driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------
* 3.2   adk  02/13/14 First Release.
*                     Added the XDeint_Selftest function.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdeint.h"
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
* This function reads Version register of Deinterlacer core and compares
* with zero as part of self test.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
*
* @return
*		- XST_SUCCESS if the test was successful.
*		- XST_FAILURE if the test failed.
*
******************************************************************************/
int XDeint_Selftest(XDeint *InstancePtr)
{
	u32 Version;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Version = XDeint_ReadReg((InstancePtr)->Config.BaseAddress,
					(XDEINT_VER_OFFSET));

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
