/******************************************************************************
*
* Copyright (C) 2002 - 2016 Xilinx, Inc. All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @addtogroup wdttb_v4_0
* @{
*
* Contains diagnostic self-test functions for the XWdtTb component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00b jhl  02/06/02 First release
* 1.10b mta  03/23/07 Updated to new coding style
* 2.00a ktn  10/22/09 Updated to use the HAL processor APIs/macros.
* 4.0   sha  12/17/15 Adherence to coding and Doxygen guidelines.
*                     Changed int -> u32.
*                     Update self-test with window WDT feature.
*                     Removed included xil_types, xil_io and xil_assert header
*                     files.
*                     Adherence to MISRA-C guidelines.
* 4.0   sha  01/29/16 Added following macros for Window WDT feature:
*                     XWT_FW_COUNT, XWT_SW_COUNT.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xwdttb.h"

/************************** Constant Definitions *****************************/

#define XWT_MAX_SELFTEST_LOOP_COUNT	0x00010000U
#define XWT_FW_COUNT			0x0U
#define XWT_SW_COUNT			0x10000U

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* This function runs a self-test on the timebase or window if enabled. Timebase
* test verifies that the timebase is incrementing. The watchdog timer is not
* tested due to the time required to wait for the watchdog timer to expire. The
* time consumed by this test is dependant on the system clock and the
* configuration of the dividers in for the input clock of the timebase.
*
* Window test verifies that the windowing feature does not generate bad event
* after enabling window feature with first and second window count. It disables
* window feature immediately.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return
*		- XST_SUCCESS if self-test was successful.
*		- XST_WDTTB_TIMER_FAILED if the timebase is not incrementing.
*		- XST_FAILURE if self-test was failed.
*
* @note		None.
*
******************************************************************************/
s32 XWdtTb_SelfTest(XWdtTb *InstancePtr)
{
	u32 LoopCount;
	u32 TbrValue1;
	u32 TbrValue2;
	s32 Status;

	/*
	 * Assert to ensure the inputs are valid and the instance has been
	 * initialized
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Check whether Window WDT is enabled */
	if (InstancePtr->Config.EnableWinWdt == (u32)1) {
		/* Configure first window with zero count */
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_FWR_OFFSET,
			XWT_FW_COUNT);

		/* Configure second window count */
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_SWR_OFFSET,
			XWT_SW_COUNT);

		/* Enable Window WDT */
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_ESR_OFFSET,
			XWT_ESR_WEN_MASK);

		/* Set writable mode */
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_MWR_OFFSET,
			1);

		/* Read enable status register and update WEN bit */
		TbrValue1 = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
			XWT_ESR_OFFSET) & (~XWT_ESR_WEN_MASK);

		/*
		 * Clear WSW bit. Otherwise writing 1 will generate restart
		 * kick. WSW is RW1C.
		 */
		TbrValue1 &= ~((u32)XWT_ESR_WSW_MASK);

		/* Disable Window WDT feature */
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_ESR_OFFSET,
			TbrValue1);

		/* Read enable status register and get last bad events */
		TbrValue2 = (XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
				XWT_ESR_OFFSET) & XWT_ESR_LBE_MASK) >>
				XWT_ESR_LBE_SHIFT;

		/* Compare last bad event */
		if (TbrValue2 == (u32)0) {
			Status = XST_SUCCESS;
		}
		else {
			Status = XST_FAILURE;
		}
	}
	else {
		/*
		 * Read the timebase register twice to start the test
		 */
		TbrValue1 = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
			XWT_TBR_OFFSET);
		TbrValue2 = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
			XWT_TBR_OFFSET);

		/*
		 * Read the timebase register for a number of iterations or
		 * until it increments, which ever occurs first
		 */
		for (LoopCount = (u32)0;
			((LoopCount <= XWT_MAX_SELFTEST_LOOP_COUNT) &&
				(TbrValue2 == TbrValue1)); LoopCount++) {
			TbrValue2 =
				XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
					XWT_TBR_OFFSET);
		}

		/*
		 * If the timebase register changed the test is successful,
		 * otherwise it failed
		 */
		if (TbrValue2 != TbrValue1) {
			Status = XST_SUCCESS;
		}
		else {
			Status = XST_WDTTB_TIMER_FAILED;
		}
	}

	return Status;
}
/** @} */
