/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xipipsu_self_test_example.c
*
* This file consists of a self test example which uses the XIpiPsu driver to
* send an IPI message to self and get a response
* Each IPI channel can trigger an interrupt to itself and can exchange messages
* through the message buffer. This feature is used here to exercise the driver
* APIs.
* Example control flow:
* - Init the IPI and GIC drivers
* - Setup Interrupt System with IPI handler which inverts the received message
*   and sends back as response
* - Write a Message and Trigger IPI to Self.
* - Keep polling for response till timeout
* - Interrupt handler receives IPI and sends back response
* - Read the received response and do a sanity check
* - Print PASS or FAIL based on sanity check of response message
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
#include "xscugic.h"
#include "xipipsu.h"
#include "xipipsu_hw.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

/************************* Test Configuration ********************************/
/* IPI device ID to use for this test */
#ifndef SDT
#define TEST_CHANNEL_ID	XPAR_XIPIPSU_0_DEVICE_ID
#endif
/* Test message length in words. Max is 8 words (32 bytes) */
#define TEST_MSG_LEN	8
/* Interrupt Controller device ID */
#ifndef SDT
#define INTC_DEVICE_ID	XPAR_SCUGIC_0_DEVICE_ID
#endif
/* Time out parameter while polling for response */
#define TIMEOUT_COUNT 10000

/*****************************************************************************/

/* Global Instances of GIC and IPI devices */
XScuGic GicInst;
XIpiPsu IpiInst;

/* Buffers to store Test Data */
u32 MsgBuffer[TEST_MSG_LEN];

static void InvertBuffer(u32 *BufPtr, u32 MsgLen)
{
	u32 l_Index;

	for (l_Index = 0; l_Index < MsgLen; l_Index++) {
		BufPtr[l_Index] = ~BufPtr[l_Index];
	}
}


/**
 * Interrupt Handler :
 * -Polls for each of the valid sources
 * -Checks if there is a message
 * -Reads the message
 * -Inverts the bits
 * -Sends back the inverted message as response
 *
 */
void IpiIntrHandler(void *XIpiPsuPtr)
{

	u32 IpiSrcMask; /**< Holds the IPI status register value */
	u32 Index;

	u32 TmpBufPtr[TEST_MSG_LEN] = { 0 }; /**< Holds the received Message, later inverted and sent back as response*/

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
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler
	 * performs the specific interrupt processing for the device
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

/**
 * @brief	Tests the IPI by sending a message and checking the response
 */

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

	/**
	 * Send a Message to TEST_TARGET and WAIT for ACK
	 */
	XIpiPsu_WriteMessage(InstancePtr, DestCfgPtr->BitMask, MsgBuffer,
			     TEST_MSG_LEN,
			     XIPIPSU_BUF_TYPE_MSG);
	xil_printf("Triggering IPI and Waiting for Response...\r\n");
	XIpiPsu_TriggerIpi(InstancePtr, DestCfgPtr->BitMask);
	Status = XIpiPsu_PollForAck(InstancePtr, DestCfgPtr->BitMask,
				    TIMEOUT_COUNT);

	if (XST_SUCCESS == Status) {

		xil_printf("Received response.\r\n");
		/**
		 * Read the Response buffer
		 */
		XIpiPsu_ReadMessage(InstancePtr, DestCfgPtr->BitMask, TmpBuffer,
				    TEST_MSG_LEN, XIPIPSU_BUF_TYPE_RESP);
		/**
		 * Set the Status to SUCCESS; Status will be set to FAILURE in case the check fails
		 * in the consequent code
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
		/**
		 * Do Nothing
		 * We need to loop on to receive IPIs and respond to them
		 */
		__asm("wfi");
	}
	while (1);

	/* Control never reaches here */
	return Status;

}
