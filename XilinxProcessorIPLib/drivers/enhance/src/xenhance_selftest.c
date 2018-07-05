/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xenhance_selftest.c
* @addtogroup enhance_v7_1
* @{
*
* This file contains the self-test functions for the Enhance driver.
* The self test function reads the Version register.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 7.0   adk  19/02/14 First Release.
*                     Implemented the following function
*                     XEnhance_SelfTest
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xenhance.h"
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
* This function reads complete Version register of Enhance core and compares
* with zero values as part of self test.
*
* @param	InstancePtr is a pointer to the XEnhance instance to be worked
*		on.
*
* @return
*		- XST_SUCCESS if the test was successful.
*		- XST_FAILURE if the test failed.
*
* @note		None.
*
******************************************************************************/
int XEnhance_SelfTest(XEnhance *InstancePtr)
{
	u32 Version;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read Enhance core version register. */
	Version = XEnhance_ReadReg((InstancePtr)->Config.BaseAddress,
			(XENH_VERSION_OFFSET));

	/* Compare Version with non-zero */
	if(Version != (u32)0x00) {
		Status = (XST_SUCCESS);
	}
	else {
		Status = (XST_FAILURE);
	}

	return Status;
}
/** @} */
