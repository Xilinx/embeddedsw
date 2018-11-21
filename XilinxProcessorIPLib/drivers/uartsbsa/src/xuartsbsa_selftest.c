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
* @file xuartsbsa_selftest.c
* @addtogroup uartsbsa_v1_0
* @{
*
* This file contains the self-test functions for the XUartSbsa driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who  Date      Changes
* ---  ---  --------- -----------------------------------------------
* 1.0  sg   09/18/17  First Release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xuartsbsa.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XUARTSBSA_TOTAL_BYTES (u8)32

/************************** Variable Definitions *****************************/

static u8 TestString[XUARTSBSA_TOTAL_BYTES]="abcdefghABCDEFGH012345677654321";
static u8 ReturnString[XUARTSBSA_TOTAL_BYTES];

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* This function runs a self-test on the driver and hardware device. This self
* test performs a local loopback and verifies data can be sent and received.
*
* The time for this test is proportional to the baud rate that has been set
* prior to calling this function.
*
* The mode and control registers are restored before return.
*
* @param	InstancePtr is a pointer to the XUartSbsa instance
*
* @return
*		- XST_SUCCESS if the test was successful
*		- XST_UART_TEST_FAIL if the test failed looping back the data
*
* @note
* This function can hang if the hardware is not functioning properly.
*
******************************************************************************/
s32 XUartSbsa_SelfTest(XUartSbsa *InstancePtr)
{
	s32 Status = XST_SUCCESS;
	u32 CtrlRegister;
	u8 Index;
	u32 ReceiveDataResult;
	u32 LineCtrlRegister;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Setup for local loop back */
	CtrlRegister = XUartSbsa_ReadReg(InstancePtr->Config.BaseAddress,
				XUARTSBSA_UARTCR_OFFSET);
	CtrlRegister |= XUARTSBSA_UARTCR_LBE;
	XUartSbsa_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTSBSA_UARTCR_OFFSET, CtrlRegister);

	/* Send a number of bytes and receive them, one at a time. */
	for (Index = 0U; Index < XUARTSBSA_TOTAL_BYTES; Index++) {
		/*
		 * Send out the byte and if it was not sent then the failure
		 * will be caught in the comparison at the end
		 */
		(void)XUartSbsa_Send(InstancePtr, &TestString[Index], 1U);

		/*
		 * Wait until the byte is received. This can hang if the HW
		 * is broken. Watch for the FIFO empty flag to be false.
		 */
		ReceiveDataResult = Xil_In32((InstancePtr->Config.
					BaseAddress) +
					XUARTSBSA_UARTFR_OFFSET) &
					XUARTSBSA_UARTFR_RXFE;
		while (ReceiveDataResult == (u32)XUARTSBSA_UARTFR_RXFE) {
			ReceiveDataResult = Xil_In32((InstancePtr->Config.
						BaseAddress) +
						XUARTSBSA_UARTFR_OFFSET) &
						XUARTSBSA_UARTFR_RXFE;
		}

		/* Receive the byte */
		(void)XUartSbsa_Recv(InstancePtr, &ReturnString[Index], 1U);
	}

	/*
	 * Compare the bytes received to the bytes sent to verify the exact
	 * data was received
	 */
	for (Index = 0U; Index < XUARTSBSA_TOTAL_BYTES; Index++) {
		if (TestString[Index] != ReturnString[Index]) {
			Status = XST_UART_TEST_FAIL;
		}
	}

	/* Disable UART */
	CtrlRegister = XUartSbsa_ReadReg(InstancePtr->Config.BaseAddress,
					XUARTSBSA_UARTCR_OFFSET);
	CtrlRegister &= ~XUARTSBSA_UARTCR_UARTEN;
	XUartSbsa_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTSBSA_UARTCR_OFFSET, CtrlRegister);

	/*
	 * Check is TX completed
	 */
	while (XUartSbsa_IsTransmitbusy(InstancePtr->Config.BaseAddress));

	/*
	 * Flush the transmit FIFO by setting the FEN bit to 0 in the
	 * Line Control Register
	 */
	LineCtrlRegister = XUartSbsa_ReadReg(InstancePtr->Config.BaseAddress,
					XUARTSBSA_UARTLCR_OFFSET);

	LineCtrlRegister &= ~XUARTSBSA_UARTLCR_FEN;
	XUartSbsa_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTSBSA_UARTLCR_OFFSET, LineCtrlRegister);

	/* Setup for normal loop mode*/
	CtrlRegister = XUartSbsa_ReadReg(InstancePtr->Config.BaseAddress,
					XUARTSBSA_UARTCR_OFFSET);
	CtrlRegister &= ~XUARTSBSA_UARTCR_MODE_MASK;
	CtrlRegister |= XUARTSBSA_UARTCR_MODE_NORMAL;
	XUartSbsa_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTSBSA_UARTCR_OFFSET, CtrlRegister);

	/* Enable UART */
	CtrlRegister = XUartSbsa_ReadReg(InstancePtr->Config.BaseAddress,
					XUARTSBSA_UARTCR_OFFSET);
	CtrlRegister |= XUARTSBSA_UARTCR_UARTEN;
	XUartSbsa_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTSBSA_UARTCR_OFFSET, CtrlRegister);

	return Status;
}
/** @} */
