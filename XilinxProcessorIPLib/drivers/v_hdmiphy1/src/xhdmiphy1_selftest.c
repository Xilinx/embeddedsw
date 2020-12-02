/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xhdmiphy1_selftest.c
 *
 * This file contains a diagnostic self-test function for the XHdmiphy1 driver.
 * It will check many of the Video PHY's register values against the default
 * reset values as a sanity-check that the core is ready to be used.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 *            dd/mm/yy
 * ----- ---- -------- -----------------------------------------------
 * 1.0   gm   10/12/18 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xhdmiphy1.h"
#include "xstatus.h"

/**************************** Variable Definitions ****************************/

/**
 * This table contains the default values for the Video PHY core's registers.
 */
static u32 ResetValues[1][2] =
{
	{XHDMIPHY1_VERSION_REG, 0}
};

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function runs a self-test on the XHdmiphy1 driver/device. The sanity
 * test checks whether or not all tested registers hold their default reset
 * values.
 *
 * @param	InstancePtr is a pointer to the XHdmiphy1 instance.
 *
 * @return
 *		- XST_SUCCESS if the self-test passed - all tested registers
 *		  hold their default reset values.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XHdmiphy1_SelfTest(XHdmiphy1 *InstancePtr)
{
	u8 Index;
	u32 Val;

	/* Compare general usage registers with their default values. */
	for (Index = 0; Index < 1; Index++) {
		Val = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
				ResetValues[Index][0]);
		/* Fail if register does not hold default value. */
		if (Val != ResetValues[Index][1]) {
			return XST_FAILURE;
		}
	}

	/* All tested registers hold their default reset values. */
	return XST_SUCCESS;
}
