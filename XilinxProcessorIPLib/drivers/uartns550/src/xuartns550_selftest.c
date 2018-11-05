/******************************************************************************
*
* Copyright (C) 2002 - 2015 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xuartns550_selftest.c
* @addtogroup uartns550_v3_5
* @{
*
* This file contains the self-test functions for the 16450/16550 UART driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  08/16/01 First release
* 1.00b jhl  03/11/02 Repartitioned driver for smaller files.
* 1.11a sv   03/20/07 Updated to use the new coding guidelines.
* 2.00a ktn  10/20/09 Converted all register accesses to 32 bit access.
*		      Updated to use HAL Processor APIs. _m is removed from the
*		      name of all the macro definitions.
* 2.01a bss  01/13/12 Updated the XUartNs550_SelfTest to use Xil_AssertNonvoid
*		      in place of XASSERT_NONVOID for CR 641344.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xuartns550.h"
#include "xuartns550_i.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

#define XUN_TOTAL_BYTES 32

/************************** Variable Definitions *****************************/

static u8 TestString[XUN_TOTAL_BYTES + 1] =
					"abcdefghABCDEFGH0123456776543210";
static u8 ReturnString[XUN_TOTAL_BYTES + 1];

/************************** Function Prototypes ******************************/


/****************************************************************************/
/**
*
* This functions runs a self-test on the driver and hardware device. This self
* test performs a local loopback and verifies data can be sent and received.
*
* The statistics are cleared at the end of the test. The time for this test
* to execute is proportional to the baud rate that has been set prior to
* calling this function.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return
*
* 		- XST_SUCCESS if the test was successful
* 		- XST_UART_TEST_FAIL if the test failed looping back the data
*
* @note 	This function can hang if the hardware is not functioning
*		properly.
*
******************************************************************************/
int XUartNs550_SelfTest(XUartNs550 *InstancePtr)
{
	int Status = XST_SUCCESS;
	u32 McrRegister;
	u32 LsrRegister;
	u32 IerRegister;
	u32 Index;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Setup for polling by disabling all interrupts in the interrupt enable
	 * register
	 */
	IerRegister = XUartNs550_ReadReg(InstancePtr->BaseAddress,
						XUN_IER_OFFSET);
	XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_IER_OFFSET, 0);

	/*
	 * Setup for loopback by enabling the loopback in the modem control
	 * register
	 */
	McrRegister = XUartNs550_ReadReg(InstancePtr->BaseAddress,
						XUN_MCR_OFFSET);
	XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_MCR_OFFSET,
			 McrRegister | XUN_MCR_LOOP);

	/*
	 * Send a number of bytes and receive them, one at a time so this
	 * test will work for 450 and 550
	 */
	for (Index = 0; Index < XUN_TOTAL_BYTES; Index++) {
		/*
		 * Send out the byte and if it was not sent then the failure
		 * will be caught in the compare at the end
		 */
		XUartNs550_Send(InstancePtr, &TestString[Index], 1);

		/*
		 * Wait til the byte is received such that it should be waiting
		 * in the receiver. This can hang if the HW is broken
		 */
		do {
			LsrRegister =
			XUartNs550_GetLineStatusReg(InstancePtr->BaseAddress);
		}
		while ((LsrRegister & XUN_LSR_DATA_READY) == 0);

		/*
		 * Receive the byte that should have been received because of
		 * the loopback, if it wasn't received then it will be caught
		 * in the compare at the end
		 */
		XUartNs550_Recv(InstancePtr, &ReturnString[Index], 1);
	}

	/*
	 * Clear the stats since they are corrupted by the test
	 */
	XUartNs550_ClearStats(InstancePtr);

	/*
	 * Compare the bytes received to the bytes sent to verify the exact data
	 * was received
	 */
	for (Index = 0; Index < XUN_TOTAL_BYTES; Index++) {
		if (TestString[Index] != ReturnString[Index]) {
			Status = XST_UART_TEST_FAIL;
		}
	}

	/*
	 * Restore the registers which were altered to put into polling and
	 * loopback modes so that this test is not destructive
	 */
	XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_IER_OFFSET,
				IerRegister);
	XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_MCR_OFFSET,
				McrRegister);

	return Status;
}
/** @} */
