/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xprd_selftest.c
* @addtogroup prd Overview
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
* 2.3   Nava  07/30/2025    Improve self-test by validating control register
*                           read/write path.
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
	u32 Data, ReadBack;

	/* Validate input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Read the value from the control register */
	Data = XPrd_ReadReg(InstancePtr->Config.BaseAddress + XPRD_CTRL_OFFSET);

	/* Write the same value back */
	XPrd_WriteReg(InstancePtr->Config.BaseAddress + XPRD_CTRL_OFFSET, Data);

	/* Read it again to verify */
	ReadBack = XPrd_ReadReg(InstancePtr->Config.BaseAddress + XPRD_CTRL_OFFSET);

	/*compare to ensure consistency */
	if (Data != ReadBack) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
/** @} */
