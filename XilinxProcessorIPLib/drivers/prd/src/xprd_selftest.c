/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xprd_selftest.c
* @addtogroup prd_v2_1
* @{
*
* This file contains the self-test functions for the XPrd driver.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who     Date	     Changes
* ----- ----- -----------  -----------------------------------------------
* 1.0   ms    07/14/2016    First release.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xprd.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* This function runs a self-test for the PRD driver. This self test reads the
* value from the status register.
*
* @param	InstancePtr is a pointer to the XPrd instance.
*
* @return
*		- XST_SUCCESS if the test was successful.
*
* @note		None.
*
******************************************************************************/
s32 XPrd_SelfTest(XPrd *InstancePtr)
{
	u32 Data;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Read the value from the status register */
	Data = XPrd_ReadReg((InstancePtr->Config.BaseAddress) +
			XPRD_CTRL_OFFSET);

	return XST_SUCCESS;
}
/** @} */
