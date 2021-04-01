/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartpsv_selftest.c
* @addtogroup uartpsv_v1_5
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
* 1.2  rna  01/20/20  Use XUartPsv_ProgramCtrlReg function to restore CR
* 1.3  rna  05/14/20  Fix Misra-C violations
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xuartpsv.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XUARTPSV_TOTAL_BYTES (u8)32	/**< No. of bytes in transfer */

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
	s32 Status = (s32)XST_SUCCESS;
	u32 CtrlRegister;
	u8 Index;
	u32 ReceiveDataResult;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Save the Control Register to restore later */
	CtrlRegister = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTCR_OFFSET);

	/* Setup for local loop back, this function modifies Control register */
	XUartPsv_SetOperMode(InstancePtr, XUARTPSV_OPER_MODE_LOCAL_LOOP);

	/* Send a number of bytes and receive them, one at a time. */
	for (Index = 0U; Index < (XUARTPSV_TOTAL_BYTES - 1U); Index++) {
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
	for (Index = 0U; Index < (XUARTPSV_TOTAL_BYTES - 1U); Index++) {
		if (TestString[Index] != ReturnString[Index]) {
			Status = (s32)XST_UART_TEST_FAIL;
		}
	}

	/* Reprogram the control Register with the saved value */
	XUartPsv_ProgramCtrlReg(InstancePtr, CtrlRegister);

	return (s32)Status;
}
/** @} */
