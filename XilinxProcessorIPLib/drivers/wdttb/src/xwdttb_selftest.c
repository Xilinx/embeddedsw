/* $Id: xwdttb_selftest.c,v 1.1.2.1 2009/12/04 05:52:37 svemula Exp $ */
/******************************************************************************
*
* (c) Copyright 2002-2009 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
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
