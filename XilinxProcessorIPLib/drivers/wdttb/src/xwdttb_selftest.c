/******************************************************************************
*
* Copyright (C) 2002 - 2014 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
* @file xwdttb_selftest.c
*
* Contains diagnostic self-test functions for the XWdtTb component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  02/06/02 First release
* 1.10b mta  03/23/07 Updated to new coding style
* 2.00a ktn  10/22/09 Updated to use the HAL processor APIs/macros.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xwdttb.h"

/************************** Constant Definitions *****************************/

#define XWT_MAX_SELFTEST_LOOP_COUNT 0x00010000

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/****************************************************************************/
/**
*
* Run a self-test on the timebase. This test verifies that the timebase is
* incrementing. The watchdog timer is not tested due to the time required
* to wait for the watchdog timer to expire. The time consumed by this test
* is dependant on the system clock and the configuration of the dividers in
* for the input clock of the timebase.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be worked on.
*
* @return
* 		- XST_SUCCESS if self-test was successful
* 		- XST_WDTTB_TIMER_FAILED if the timebase is not incrementing
*
* @note		None.
*
******************************************************************************/
int XWdtTb_SelfTest(XWdtTb *InstancePtr)
{
	int LoopCount;
	u32 TbrValue1;
	u32 TbrValue2;

	/*
	 * Assert to ensure the inputs are valid and the instance has been
	 * initialized
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the timebase register twice to start the test
	 */
	TbrValue1 = XWdtTb_ReadReg(InstancePtr->RegBaseAddress, XWT_TBR_OFFSET);
	TbrValue2 = XWdtTb_ReadReg(InstancePtr->RegBaseAddress, XWT_TBR_OFFSET);

	/*
	 * Read the timebase register for a number of iterations or until it
	 * increments, which ever occurs first
	 */
	for (LoopCount = 0;
	    ((LoopCount <= XWT_MAX_SELFTEST_LOOP_COUNT) &&
	     (TbrValue2 == TbrValue1)); LoopCount++) {
		TbrValue2 = XWdtTb_ReadReg(InstancePtr->RegBaseAddress,
				     XWT_TBR_OFFSET);
	}

	/*
	 * If the timebase register changed the test is successful, otherwise it
	 * failed
	 */
	if (TbrValue2 != TbrValue1) {
		return XST_SUCCESS;
	}
	else {
		return XST_WDTTB_TIMER_FAILED;
	}
}
