/******************************************************************************
*
* Copyright (C) 2017-2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xuartpsv_selftest.c
* @addtogroup uartpsv_v1_1
* @{
*
* This file contains the self-test functions for the XUartPsv driver.
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
#include "xuartpsv.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XUARTPSV_TOTAL_BYTES (u8)32

/************************** Variable Definitions *****************************/

static u8 TestString[XUARTPSV_TOTAL_BYTES]="abcdefghABCDEFGH012345677654321";
static u8 ReturnString[XUARTPSV_TOTAL_BYTES];

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
* @param	InstancePtr is a pointer to the XUartPsv instance
*
* @return
*		- XST_SUCCESS if the test was successful
*		- XST_UART_TEST_FAIL if the test failed looping back the data
*
* @note
* This function can hang if the hardware is not functioning properly.
*
******************************************************************************/
s32 XUartPsv_SelfTest(XUartPsv *InstancePtr)
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
	CtrlRegister = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XUARTPSV_UARTCR_OFFSET);
	CtrlRegister |= XUARTPSV_UARTCR_LBE;
	XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTCR_OFFSET, CtrlRegister);

	/* Send a number of bytes and receive them, one at a time. */
	for (Index = 0U; Index < XUARTPSV_TOTAL_BYTES; Index++) {
		/*
		 * Send out the byte and if it was not sent then the failure
		 * will be caught in the comparison at the end
		 */
		(void)XUartPsv_Send(InstancePtr, &TestString[Index], 1U);

		/*
		 * Wait until the byte is received. This can hang if the HW
		 * is broken. Watch for the FIFO empty flag to be false.
		 */
		ReceiveDataResult = Xil_In32((InstancePtr->Config.
					BaseAddress) +
					XUARTPSV_UARTFR_OFFSET) &
					XUARTPSV_UARTFR_RXFE;
		while (ReceiveDataResult == (u32)XUARTPSV_UARTFR_RXFE) {
			ReceiveDataResult = Xil_In32((InstancePtr->Config.
						BaseAddress) +
						XUARTPSV_UARTFR_OFFSET) &
						XUARTPSV_UARTFR_RXFE;
		}

		/* Receive the byte */
		(void)XUartPsv_Recv(InstancePtr, &ReturnString[Index], 1U);
	}

	/*
	 * Compare the bytes received to the bytes sent to verify the exact
	 * data was received
	 */
	for (Index = 0U; Index < XUARTPSV_TOTAL_BYTES; Index++) {
		if (TestString[Index] != ReturnString[Index]) {
			Status = XST_UART_TEST_FAIL;
		}
	}

	/* Disable UART */
	CtrlRegister = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XUARTPSV_UARTCR_OFFSET);
	CtrlRegister &= ~XUARTPSV_UARTCR_UARTEN;
	XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTCR_OFFSET, CtrlRegister);

	/*
	 * Check is TX completed
	 */
	while (XUartPsv_IsTransmitbusy(InstancePtr->Config.BaseAddress));

	/*
	 * Flush the transmit FIFO by setting the FEN bit to 0 in the
	 * Line Control Register
	 */
	LineCtrlRegister = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XUARTPSV_UARTLCR_OFFSET);

	LineCtrlRegister &= ~XUARTPSV_UARTLCR_FEN;
	XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTLCR_OFFSET, LineCtrlRegister);

	/* Setup for normal loop mode*/
	CtrlRegister = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XUARTPSV_UARTCR_OFFSET);
	CtrlRegister &= ~XUARTPSV_UARTCR_MODE_MASK;
	CtrlRegister |= XUARTPSV_UARTCR_MODE_NORMAL;
	XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTCR_OFFSET, CtrlRegister);

	/* Enable UART */
	CtrlRegister = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XUARTPSV_UARTCR_OFFSET);
	CtrlRegister |= XUARTPSV_UARTCR_UARTEN;
	XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTCR_OFFSET, CtrlRegister);

	return Status;
}
/** @} */
