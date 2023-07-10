/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmon_selftest.c
* @addtogroup Overview
* @{
*
* This file contains a diagnostic self test function for the XSysMon driver.
* The self test function does a simple read/write test of the Alarm Threshold
* Register.
*
* See xsysmon.h for more information.
*
* @note	None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a xd/sv  05/22/07 First release
* 4.00a ktn    10/22/09 Updated the file to use the HAL APIs/macros.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xsysmon.h"

/************************** Constant Definitions ****************************/

/*
 * The following constant defines the test value to be written
 * to the Alarm Threshold Register
 */
#define XSM_ATR_TEST_VALUE 		0x55

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/

/*****************************************************************************/
/**
*
* Run a self-test on the driver/device. The test
*	- Resets the device,
*	- Writes a value into the Alarm Threshold register and reads it back
*	for comparison.
*	- Resets the device again.
*
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return
*		- XST_SUCCESS if the value read from the Alarm Threshold
*		register is the same as the value written.
*		- XST_FAILURE Otherwise
*
* @note		This is a destructive test in that resets of the device are
*		performed. Refer to the device specification for the
*		device status after the reset operation.
*
******************************************************************************/
int XSysMon_SelfTest(XSysMon *InstancePtr)
{
	int Status;
	u32 RegValue;

	/*
	 * Assert the argument
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	/*
	 * Reset the device to get it back to its default state
	 */
	XSysMon_Reset(InstancePtr);

	/*
	 * Write a value into the Alarm Threshold registers, read it back, and
	 * do the comparison
	 */
	XSysMon_SetAlarmThreshold(InstancePtr, XSM_ATR_VCCINT_UPPER,
				  XSM_ATR_TEST_VALUE);
	RegValue = XSysMon_GetAlarmThreshold(InstancePtr, XSM_ATR_VCCINT_UPPER);

	if (RegValue == XSM_ATR_TEST_VALUE) {
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}

	/*
	 * Reset the device again to its default state.
	 */
	XSysMon_Reset(InstancePtr);


	/*
	 * Return the test result.
	 */
	return Status;
}
/** @} */
