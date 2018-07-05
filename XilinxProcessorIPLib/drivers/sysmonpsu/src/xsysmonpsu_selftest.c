/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsu_selftest.c
* @addtogroup sysmonpsu_v2_6
*
* This file contains a diagnostic self test function for the XSysMon driver.
* The self test function does a simple read/write test of the Alarm Threshold
* Register.
*
* See xsysmonpsu.h for more information.
*
* @note	None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   kvn   12/15/15  First release
* 2.5   mn     07/06/18 Fixed Doxygen warnings
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xsysmonpsu.h"

/************************** Constant Definitions ****************************/

/*
 * The following constant defines the test value to be written
 * to the Alarm Threshold Register
 */
#define XSM_ATR_TEST_VALUE 		0x55U

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
* @param	InstancePtr is a pointer to the XSysMonPsu instance.
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
s32 XSysMonPsu_SelfTest(XSysMonPsu *InstancePtr)
{
	s32 Status;
	u32 RegValue;

	/* Assert the argument */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Reset the device to get it back to its default state */
	XSysMonPsu_Reset(InstancePtr);

	/*
	 * Write a value into the Alarm Threshold registers, read it back, and
	 * do the comparison
	 */
	XSysMonPsu_SetAlarmThreshold(InstancePtr, XSM_ATR_SUP1_UPPER,
				  XSM_ATR_TEST_VALUE, XSYSMON_PS);
	RegValue = (u32)XSysMonPsu_GetAlarmThreshold(InstancePtr,
					XSM_ATR_SUP1_UPPER, XSYSMON_PS);

	if (RegValue == XSM_ATR_TEST_VALUE) {
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}

	/* Reset the device again to its default state. */
	XSysMonPsu_Reset(InstancePtr);

	/* Return the test result. */
	return Status;
}
/** @} */
