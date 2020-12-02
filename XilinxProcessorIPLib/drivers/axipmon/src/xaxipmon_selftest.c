/******************************************************************************
* Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxipmon_selftest.c
* @addtogroup axipmon_v6_9
* @{
*
* This file contains a diagnostic self test function for the XAxiPmon driver.
* The self test function does a simple read/write test of the Alarm Threshold
* Register.
*
* See XAxiPmon.h for more information.
*
* @note	None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a bss  02/24/12 First release
* 2.00a bss  06/23/12 Updated to support v2_00a version of IP.
* 6.3   kvn  07/02/15 Modified code according to MISRA-C:2012 guidelines.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xaxipmon.h"

/************************** Constant Definitions ****************************/

/*
 * The following constant defines the test value to be written
 * to the Range Registers of Incrementers
 */

#define XAPM_TEST_RANGEUPPER_VALUE	16U /**< Test Value for Upper Range */
#define XAPM_TEST_RANGELOWER_VALUE	 8U /**< Test Value for Lower Range */

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/

/*****************************************************************************/
/**
*
* Run a self-test on the driver/device. The test
*	- Resets the device,
*	- Writes a value into the Range Registers of Incrementer 0 and reads
*	  it back for comparison.
*	- Resets the device again.
*
*
* @param	InstancePtr is a pointer to the XAxiPmon instance.
*
* @return
*		- XST_SUCCESS if the value read from the Range Register of
*		  Incrementer 0 is the same as the value written.
*		- XST_FAILURE Otherwise
*
* @note		This is a destructive test in that resets of the device are
*		performed. Refer to the device specification for the
*		device status after the reset operation.
*
******************************************************************************/
s32 XAxiPmon_SelfTest(XAxiPmon *InstancePtr)
{
	s32 Status;
	u16 RangeUpper = 0U;
	u16 RangeLower = 0U;

	/*
	 * Assert the argument
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	/*
	 * Reset the device to get it back to its default state
	 */
	(void)XAxiPmon_ResetMetricCounter(InstancePtr);
	XAxiPmon_ResetGlobalClkCounter(InstancePtr);

	/*
	 * Write a value into the Incrementer register and
	 * read it back, and do the comparison
	 */
	XAxiPmon_SetIncrementerRange(InstancePtr, XAPM_INCREMENTER_0,
					XAPM_TEST_RANGEUPPER_VALUE,
					XAPM_TEST_RANGELOWER_VALUE);

	XAxiPmon_GetIncrementerRange(InstancePtr, XAPM_INCREMENTER_0,
					&RangeUpper, &RangeLower);

	if ((RangeUpper == XAPM_TEST_RANGEUPPER_VALUE) &&
			(RangeLower == XAPM_TEST_RANGELOWER_VALUE)) {
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}

	/*
	 * Reset the device again to its default state.
	 */
	(void)XAxiPmon_ResetMetricCounter(InstancePtr);
	XAxiPmon_ResetGlobalClkCounter(InstancePtr);

	/*
	 * Return the test result.
	 */
	return Status;
}
/** @} */
