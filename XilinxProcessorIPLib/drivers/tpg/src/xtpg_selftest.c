/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtpg_selftest.c
* @addtogroup tpg_v3_3
* @{
*
* This file contains the self-test functions for the TPG driver.
* The self test function reads the Version Register.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  ------- ----------------------------------------------
* 3.0   adk   02/19/14 First Release.
*                      Implemented XTpg_SelfTest function.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xtpg.h"
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
* This function reads Version register of TPG core and compares with zero
* as part of self test.
*
* @param	InstancePtr is a pointer to the TPG instance.
*
* @return
*		- XST_SUCCESS if the Version register read test was successful.
*		- XST_FAILURE if the Version register read test failed.
*
* @note		None.
*
******************************************************************************/
int XTpg_SelfTest(XTpg *InstancePtr)
{
	u32 Version;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read TPG core version register. */
	Version = XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_VERSION_OFFSET));

	/* Compare version with zero. */
	if(Version != (u32)0x0) {
		Status = (XST_SUCCESS);
	}
	else {
		Status = (XST_FAILURE);
	}

	return Status;
}
/** @} */
