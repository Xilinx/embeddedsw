/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhwicap_selftest.c
* @addtogroup hwicap_v11_4
* @{
*
* This file contains a diagnostic self test function for the XHwIcap driver.
* The self test functions writes to the Interrupt Enable Register and reads it
* back for comparison.
*
* See xhwicap.h for more information.
*
* @note	None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a sv     09/17/07 First release
* 4.00a hvm    12/1/09  Updated with HAL phase 1 modifications
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xhwicap.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/

/*****************************************************************************/
/**
*
* Run a self-test on the driver/device. The test
*	- Writes to the Interrupt Enable Register and reads it back
*	for comparison.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
*
* @return
*		- XST_SUCCESS if the value read from the register
*		is the same as the value written.
*		- XST_FAILURE otherwise
*
* @note		None.
*
******************************************************************************/
int XHwIcap_SelfTest(XHwIcap *InstancePtr)
{
	int Status = XST_SUCCESS;
	u32 IeRegister;
	u32 DgieRegister;


	/*
	 * Assert the argument
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	/*
	 * Save a copy of the Global Interrupt Enable Register and Interrupt
	 * Enable Register before writing them so that they can be restored.
	 */
	DgieRegister = XHwIcap_ReadReg(InstancePtr->HwIcapConfig.BaseAddress,
						XHI_GIER_OFFSET);
	IeRegister = XHwIcap_IntrGetEnabled(InstancePtr);

	/*
	 * Disable the Global Interrupt
	 */
	XHwIcap_IntrGlobalDisable(InstancePtr);


	/*
	 * Disable/Enable the interrupts and then verify that the register
	 * is read back correct.
	 */
	XHwIcap_IntrDisable(InstancePtr, XHI_IPIXR_ALL_MASK);
	if (XHwIcap_IntrGetEnabled(InstancePtr) != 0x0) {
		Status = XST_FAILURE;
	}

	XHwIcap_IntrEnable(InstancePtr, (XHI_IPIXR_WEMPTY_MASK |
					XHI_IPIXR_RDP_MASK));
	if (XHwIcap_IntrGetEnabled(InstancePtr) !=
			(XHI_IPIXR_WEMPTY_MASK | XHI_IPIXR_RDP_MASK)) {
		Status |= XST_FAILURE;
	}

	/*
	 * Restore the Interrupt Enable Register to the value before the
	 * test.
	 */
	XHwIcap_IntrDisable(InstancePtr, XHI_IPIXR_ALL_MASK);
	if (IeRegister != 0) {
		XHwIcap_IntrEnable(InstancePtr, IeRegister);
	}


	/*
	 * Restore the Global Interrupt Enable Register to the value
	 * before the test.
	 */
	XHwIcap_WriteReg(InstancePtr->HwIcapConfig.BaseAddress,
				XHI_GIER_OFFSET, DgieRegister);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
/** @} */
