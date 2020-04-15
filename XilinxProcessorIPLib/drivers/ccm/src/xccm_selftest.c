/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xccm_selftest.c
* @addtogroup ccm_v6_1
* @{
*
* This file contains the self-test functions for the CCM core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- ----------------------------------------------
* 6.0   adk     03/06/14 First Release.
*                        Implemented XCcm_SelfTest function.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xccm.h"
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
* This function reads Version register of CCM core and compares with zero
* as part of self test.
*
* @param	InstancePtr is a pointer to XCcm instance.
*
* @return
*		- XST_SUCCESS if the test is successful.
*		- XST_FAILURE if the test is failed.
*
* @note		None.
*
******************************************************************************/
int XCcm_SelfTest(XCcm *InstancePtr)
{
	u32 Version;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Version = XCcm_ReadReg(InstancePtr->Config.BaseAddress,
				(XCCM_VERSION_OFFSET));

	/* Compare version with zero */
	if (Version != (u32)0x0) {
		Status = (XST_SUCCESS);
	}
	else {
		Status = (XST_FAILURE);
	}

	return Status;
}
/** @} */
