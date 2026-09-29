/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xipipsu_self_test_example.c
* @brief Demonstrates the XIpiPsu driver by sending an IPI to self.
*
* The example sends an Inter-Processor Interrupt (IPI) message to self and gets
* a response. Each IPI channel can trigger an interrupt to itself and can
* exchange messages through the message buffer. The example uses this feature to
* exercise the driver APIs.
*
* Example control flow:
* - Initializes the IPI and Generic Interrupt Controller (GIC) drivers.
* - Sets up the interrupt system with an IPI handler that inverts the received
*   message and sends it back as the response.
* - Writes a message and triggers an IPI to self.
* - Polls for the response until the timeout expires.
* - The interrupt handler receives the IPI and sends back the response.
* - Reads the received response and verifies it.
* - Prints a success or failure message based on the response check.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 2.2  ms  01/23/17 Modified xil_printf statement in main function to
*                   ensure that "Successfully ran" and "Failed" strings
*                   are available in all examples. This is a fix for
*                   CR-965028.
*
* 2.14  ht  05/30/23 Added support for system device-tree flow.
* 2.17  ht  11/08/24 Update description of TEST_MSG_LEN.
*       ht  11/25/24 Update Message length to accommodate for CRC bytes
*                    when IPI CRC is enabled
* 2.18  an  08/01/25 Added support for Microblaze in system device-tree flow
* 2.20  vn  07/30/26 Fix Doxygen warnings
* 2.20  vn  09/10/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/
/*****************************************************************************/
#include "stdlib.h"
#include "xil_types.h"

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xipipsu.h"
#include "xipipsu_hw.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#else
#include "xscugic.h"
#endif
#include "sleep.h"

/************************* Test Configuration ********************************/
#ifndef SDT
/** IPI device ID for this test. */
#define TEST_CHANNEL_ID	XPAR_XIPIPSU_0_DEVICE_ID
#endif

#ifdef ENABLE_IPI_CRC
/*
 * Subtracting 1 from the message length to accommodate for the CRC bytes when
 * IPI CRC is enabled.
 * */
#define TEST_MSG_LEN	7 /**< Number of words to send when IPI CRC is
			   * enabled. */
#else
#define TEST_MSG_LEN	8 /**< Number of words to send (each word is 4
			   * bytes; maximum 8). */
#endif

#ifndef SDT
/** Interrupt controller device ID. */
#define INTC_DEVICE_ID	XPAR_SCUGIC_0_DEVICE_ID
#endif
#define TIMEOUT_COUNT 10000 /**< Timeout count while polling for a response. */

/*****************************************************************************/

/* Global Instances of GIC and IPI devices */
#ifndef SDT
XScuGic GicInst; /**< Instance of the GIC interrupt controller. */
#endif
XIpiPsu IpiInst; /**< Instance of the IPI device. */

u32 MsgBuffer[TEST_MSG_LEN]; /**< Buffer that stores the test data. */

/*****************************************************************************/
/**
 * @brief	Inverts each word of a buffer in place.
 *
 * @param[in,out]	BufPtr Pointer to the buffer to invert.
 * @param[in]	MsgLen Number of words in the buffer.
 *
 ******************************************************************************/
static void InvertBuffer(u32 *BufPtr, u32 MsgLen)
{
	u32 l_Index;

	for (l_Index = 0; l_Index < MsgLen; l_Index++) {
		BufPtr[l_Index] = ~BufPtr[l_Index];
	}
}


/*****************************************************************************/
/**
 * @brief	Handles an incoming IPI and sends back the inverted message.
 *
 * Polls each valid source, checks for a message, reads it, inverts the bits,
 * and sends the inverted message back as a response.
 *
 * @param[in]	XIpiPsuPtr Pointer to the IPI instance.
 *
 ******************************************************************************/
void IpiIntrHandler(void *XIpiPsuPtr)
{

	u32 IpiSrcMask;
	u32 Index;

	u32 TmpBufPtr[TEST_MSG_LEN] = { 0 };

	u32 SrcIndex;
	XIpiPsu *InstancePtr = (XIpiPsu *) XIpiPsuPtr;

	xil_printf("---->Enter Interrupt Handler\r\n");

	Xil_AssertVoid(InstancePtr != NULL);

	IpiSrcMask = XIpiPsu_GetInterruptStatus(InstancePtr);

	/* Poll for each source and send Response (Response = ~Msg) */

	for (SrcIndex = 0U; SrcIndex < InstancePtr->Config.TargetCount;
	     SrcIndex++) {

		if (IpiSrcMask & InstancePtr->Config.TargetList[SrcIndex].Mask) {

			/*  Read Incoming Message Buffer Corresponding to Source CPU */
			XIpiPsu_ReadMessage(InstancePtr,
					    InstancePtr->Config.TargetList[SrcIndex].Mask, TmpBufPtr,
					    TEST_MSG_LEN, XIPIPSU_BUF_TYPE_MSG);

			xil_printf("Message Received:\r\n");

			for (Index = 0; Index < TEST_MSG_LEN; Index++) {
				xil_printf("W%d: 0x%08x\r\n", Index, TmpBufPtr[Index]);
			}

			/*Process the Received Message */
			InvertBuffer(TmpBufPtr, TEST_MSG_LEN);

			/* Send Response */
			XIpiPsu_WriteMessage(InstancePtr,
					     InstancePtr->Config.TargetList[SrcIndex].Mask, TmpBufPtr,
					     TEST_MSG_LEN, XIPIPSU_BUF_TYPE_RESP);
			xil_printf("Sent back Inverted Message.\r\n");

			/* Clear the Interrupt Status - This clears the OBS bit on the SRC CPU registers */
			XIpiPsu_ClearInterruptStatus(InstancePtr,
						     InstancePtr->Config.TargetList[SrcIndex].Mask);

		}
	}

	xil_printf("<----Exit Interrupt Handler\r\n");

}

#ifndef SDT
/*****************************************************************************/
/**
 * @brief	Sets up the interrupt system for the IPI.
 *
 * Initializes the interrupt controller, connects the IPI handler, and enables
 * the interrupt.
 *
 * @param[in,out]	IntcInstancePtr Pointer to the interrupt controller
 *			instance.
 * @param[in]	IpiInstancePtr Pointer to the IPI instance.
 * @param[in]	IpiIntrId Interrupt ID of the IPI.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if an error occurs.
 *
 ******************************************************************************/
static XStatus SetupInterruptSystem(XScuGic *IntcInstancePtr,
				    XIpiPsu *IpiInstancePtr, u32 IpiIntrId)
{
	u32 Status = 0;
	XScuGic_Config *IntcConfig; /* Config for interrupt controller */

	/* Initialize the interrupt controller driver */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&GicInst, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the interrupt controller interrupt handler to the
	 * hardware interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler) XScuGic_InterruptHandler, IntcInstancePtr);

	/*
	 * Connect a device driver handler that runs when a device interrupt
	 * occurs. The handler performs the specific interrupt processing for
	 * the device.
	 */
	xil_printf("Interrupt ID: %d\r\n", IpiIntrId);
	Status = XScuGic_Connect(IntcInstancePtr, IpiIntrId,
				 (Xil_InterruptHandler) IpiIntrHandler, (void *) IpiInstancePtr);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Enable the interrupt for the device */
	XScuGic_Enable(IntcInstancePtr, IpiIntrId);

	/* Enable interrupts */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 * @brief	Tests the IPI by sending a message and checking the response.
 *
 * @param[in]	InstancePtr Pointer to the IPI instance.
 *
 * @return
 *		- XST_SUCCESS if the response matches the expected value.
 *		- XST_FAILURE if the check fails or the poll times out.
 *
 ******************************************************************************/
static XStatus DoIpiTest(XIpiPsu *InstancePtr)
{

	u32 Index;
	u32 Status;

	u32 TmpBuffer[TEST_MSG_LEN] = { 0 };

	XIpiPsu_Config *DestCfgPtr;
#ifndef SDT
	DestCfgPtr = XIpiPsu_LookupConfig(TEST_CHANNEL_ID);
#else
	DestCfgPtr = XIpiPsu_LookupConfig(XPAR_XIPIPSU_0_BASEADDR);
#endif

	xil_printf("Message Content:\r\n");
	for (Index = 0; Index < TEST_MSG_LEN; Index++) {
		MsgBuffer[Index] = rand();
		xil_printf("W%d: 0x%08x\r\n", Index, MsgBuffer[Index]);
	}

	/* Send a message to the destination and wait for an acknowledgment. */
	XIpiPsu_WriteMessage(InstancePtr, DestCfgPtr->BitMask, MsgBuffer,
			     TEST_MSG_LEN,
			     XIPIPSU_BUF_TYPE_MSG);
	xil_printf("Triggering IPI and Waiting for Response...\r\n");
	XIpiPsu_TriggerIpi(InstancePtr, DestCfgPtr->BitMask);
	Status = XIpiPsu_PollForAck(InstancePtr, DestCfgPtr->BitMask,
				    TIMEOUT_COUNT);

	if (XST_SUCCESS == Status) {

		xil_printf("Received response.\r\n");
		/* Read the response buffer. */
		XIpiPsu_ReadMessage(InstancePtr, DestCfgPtr->BitMask, TmpBuffer,
				    TEST_MSG_LEN, XIPIPSU_BUF_TYPE_RESP);
		/*
		 * Set the status to SUCCESS. The consequent code sets it to
		 * FAILURE if the check fails.
		 */
		Status = XST_SUCCESS;
		/*
		 * Check if RESPONSE == (~MSG)
		 */
		xil_printf("Message : Response\r\n");

		for (Index = 0; Index < TEST_MSG_LEN; Index++) {
			xil_printf("W%d -> 0x%08x : 0x%08x\r\n", Index, MsgBuffer[Index],
				   TmpBuffer[Index]);
			if (MsgBuffer[Index] != (~TmpBuffer[Index])) {
				Status = XST_FAILURE;
				break;
			}
		}

	}
	else {
		xil_printf("Error: Timed Out polling for response\r\n");
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	Runs the IPI self-test example.
 *
 * @return	XST_FAILURE if interrupt setup fails. After setup, the function
 *		runs the self-test, prints the result, and loops to service
 *		IPIs. That loop does not return.
 *
 ******************************************************************************/
int main()
{

	XIpiPsu_Config *CfgPtr;

	int Status = XST_FAILURE;
	xil_printf("Hello IPI! [Build: %s %s]\r\n", __DATE__, __TIME__);

	Xil_DCacheDisable();

	/* Look Up the config data */
#ifndef SDT
	CfgPtr = XIpiPsu_LookupConfig(TEST_CHANNEL_ID);
#else
	CfgPtr = XIpiPsu_LookupConfig(XPAR_XIPIPSU_0_BASEADDR);
#endif

	/* Init with the Cfg Data */
	XIpiPsu_CfgInitialize(&IpiInst, CfgPtr, CfgPtr->BaseAddress);

	/* Setup the GIC */
#ifndef SDT
	Status = SetupInterruptSystem(&GicInst, &IpiInst, (IpiInst.Config.IntId));
#else
	Status = XSetupInterruptSystem(&IpiInst, &IpiIntrHandler,
				       IpiInst.Config.IntId,
				       IpiInst.Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Enable reception of IPIs from all CPUs */
	XIpiPsu_InterruptEnable(&IpiInst, XIPIPSU_ALL_MASK);

	/* Clear Any existing Interrupts */
	XIpiPsu_ClearInterruptStatus(&IpiInst, XIPIPSU_ALL_MASK);

	/* Call the test routine */
	Status = DoIpiTest(&IpiInst);

	/* Print the test result */
	if (XST_SUCCESS == Status) {
		xil_printf("Successfully ran Ipipsu selftest Example\r\n");
	}
	else {
		xil_printf("Ipipsu selftest Example Failed\r\n");
	}

	do {
		/*
		 * Do nothing. Loop to keep receiving IPIs and responding to
		 * them.
		 */
#if defined (__MICROBLAZE__)
		usleep(100);
#else
		__asm("wfi");
#endif
	}
	while (1);

	/* Control never reaches here */
	return Status;

}
