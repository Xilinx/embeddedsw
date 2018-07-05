/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xtmr_manager_selftest.c
* @addtogroup tmr_manager_v1_0
* @{
*
* This file contains the self-test functions for the TMR Manager component
* (XTMR_Manager).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xtmr_manager.h"
#include "xtmr_manager_i.h"
#include "xil_io.h"

/************************** Constant Definitions ****************************/


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/


/****************************************************************************/
/**
*
* Runs a self-test on the device hardware. Since there is no way to perform a
* loopback in the hardware, this test can only check the state of the status
* register to verify it is correct. This test assumes that the hardware
* device is still in its reset state, but has been initialized with the
* Initialize function.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
*
* @return
* 		- XST_SUCCESS if the self-test was successful.
* 		- XST_FAILURE if the self-test failed, the status register
*			value was not correct
*
* @note		None.
*
******************************************************************************/
int XTMR_Manager_SelfTest(XTMR_Manager *InstancePtr)
{
	u32 FirstFailingRegister;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the first failing register value to check if it is the correct
	 * value after a reset
	 */
	FirstFailingRegister = XTMR_Manager_ReadReg(InstancePtr->RegBaseAddress,
							XTM_FFR_OFFSET);

	/*
	 * If the first failing register is any other value other than zero,
	 * then the test is a failure since this is not the value after reset
	 */
	if (FirstFailingRegister != 0) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/** @} */
