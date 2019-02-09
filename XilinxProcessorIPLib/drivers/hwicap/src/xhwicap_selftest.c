/******************************************************************************
*
* Copyright (C) 2007 - 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xhwicap_selftest.c
* @addtogroup hwicap_v11_2
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
