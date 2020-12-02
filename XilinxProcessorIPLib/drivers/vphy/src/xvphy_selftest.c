/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xvphy_selftest.c
 *
 * This file contains a diagnostic self-test function for the XVphy driver. It
 * will check many of the Video PHY's register values against the default
 * reset values as a sanity-check that the core is ready to be used.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  10/19/15 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xvphy.h"
#include "xstatus.h"

/**************************** Variable Definitions ****************************/

/**
 * This table contains the default values for the Video PHY core's registers.
 */
static u32 ResetValues[1][2] =
{
	{XVPHY_VERSION_REG, 0}
};

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function runs a self-test on the XVphy driver/device. The sanity test
 * checks whether or not all tested registers hold their default reset values.
 *
 * @param	InstancePtr is a pointer to the XVphy instance.
 *
 * @return
 *		- XST_SUCCESS if the self-test passed - all tested registers
 *		  hold their default reset values.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XVphy_SelfTest(XVphy *InstancePtr)
{
	u8 Index;
	u32 Val;

	/* Compare general usage registers with their default values. */
	for (Index = 0; Index < 1; Index++) {
		Val = XVphy_ReadReg(InstancePtr->Config.BaseAddr,
				ResetValues[Index][0]);
		/* Fail if register does not hold default value. */
		if (Val != ResetValues[Index][1]) {
			return XST_FAILURE;
		}
	}

	/* All tested registers hold their default reset values. */
	return XST_SUCCESS;
}
