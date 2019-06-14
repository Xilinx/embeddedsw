
/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xilmailbox_example.c
*
* This example demonstrates the usage of mailbox library API's.
* Each IPI channel can trigger an interrupt to itself and can exchange messages
* through the message buffer. This feature is used here to exercise the library
* APIs.
* Example control flow:
* - Initialize the XMailbox instance
* - Write a Message and Trigger IPI to Self in Blocing mode.
* - Interrupt handler receives IPI and Sends back response.
* - Read the received response and do a sanity check.
* - Print PASS or FAIL based on sanity check of response message
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <stdlib.h>
#include "xparameters.h"
#include "xscugic.h"
#include "xilmailbox.h"
#include "xdebug.h"

/************************* Test Configuration ********************************/
/* IPI device ID to use for this test */
#define TEST_CHANNEL_ID	XPAR_XIPIPSU_0_DEVICE_ID
/* Test message length in words. Max is 8 words (32 bytes) */
#define TEST_MSG_LEN	8

/*****************************************************************************/
XMailbox XMboxInstance;
volatile static int RecvDone = 0;	/**< Done flag */
volatile static int ErrorStatus = 0;	/**< Error Status flag*/
static u32 ReqBuffer[TEST_MSG_LEN];
static u32 RespBuffer[TEST_MSG_LEN];

int XMailbox_Example(XMailbox *InstancePtr, u8 DeviceId);
static void DoneHandler(void *CallBackRefPtr);
static void ErrorHandler(void *CallBackRefPtr, u32 Mask);

int main(void)
{
	int Status;

	xil_printf("Inside XMailbox Example\r\n");
	Status = XMailbox_Example(&XMboxInstance, TEST_CHANNEL_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("XMailbox Example Failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran XMailbox Example\n\r");
	return XST_SUCCESS;
}

int XMailbox_Example(XMailbox *InstancePtr, u8 DeviceId)
{
	u32 Index;
	u32 Status;
	u32 *TmpBufPtr;

	Status = XMailbox_Initialize(InstancePtr, DeviceId);
	if (Status != XST_SUCCESS) {
		goto Done;
	}

	/* Register callbacks for Error and Read */
	XMailbox_SetCallBack(InstancePtr, XMAILBOX_RECV_HANDLER,
			     DoneHandler, InstancePtr);
	XMailbox_SetCallBack(InstancePtr, XMAILBOX_ERROR_HANDLER,
			     ErrorHandler, InstancePtr);

	xdbg_printf(XDBG_DEBUG_GENERAL, "Req Message Content:\r\n");
	for (Index = 0; Index < TEST_MSG_LEN; Index++) {
		ReqBuffer[Index] = rand();
		xdbg_printf(XDBG_DEBUG_GENERAL, "W%d: 0x%08x\r\n", Index,
			    ReqBuffer[Index]);
	}

	/* Send an IPI Req Message */
	Status = XMailbox_SendData(InstancePtr, XMAILBOX_IPI0, ReqBuffer,
				   TEST_MSG_LEN, XILMBOX_MSG_TYPE_REQ, 1);
	if (Status != XST_SUCCESS) {
		xil_printf("Sending Req Message Failed\n\r");
		goto Done;
	}

	while(!ErrorStatus && !RecvDone);

	if (ErrorStatus) {
		xil_printf("Error occurred during IPI transfer\n\r");
		Status = XST_FAILURE;
		goto Done;
	}

	RecvDone = 0;
	ErrorStatus = 0;

	/* Read an IPI Message */
	Status = XMailbox_Recv(InstancePtr, XMAILBOX_IPI0, TmpBufPtr,
			       TEST_MSG_LEN, XILMBOX_MSG_TYPE_REQ);
	if (Status != XST_SUCCESS) {
		xil_printf("Reading an IPI Req message Failed\n\r");
		goto Done;
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "Message Received:\r\n");
	for (Index = 0; Index < TEST_MSG_LEN; Index++) {
		xdbg_printf(XDBG_DEBUG_GENERAL, "W%d: 0x%08x\r\n", Index,
			    TmpBufPtr[Index]);
	}

	/* Response Message */
	for (Index = 0; Index < TEST_MSG_LEN; Index++) {
		RespBuffer[Index] = ~TmpBufPtr[Index];
	}

	/* Send an IPI Response Message */
	Status = XMailbox_SendData(InstancePtr, XMAILBOX_IPI0, RespBuffer,
				   TEST_MSG_LEN, XILMBOX_MSG_TYPE_RESP, 0);
	if (Status != XST_SUCCESS) {
		xil_printf("Sending Resp Message Failed\n\r");
		goto Done;
	}

	while(!ErrorStatus && !RecvDone);
	if (ErrorStatus) {
		Status = XST_FAILURE;
		xil_printf("Error occurred during IPI transfer\n\r");
		goto Done;
	}

	/* Read an IPI Resp Message */
	Status = XMailbox_Recv(InstancePtr, XMAILBOX_IPI0, TmpBufPtr,
			       TEST_MSG_LEN, XILMBOX_MSG_TYPE_RESP);
	if (Status != XST_SUCCESS) {
		xil_printf("Reading an IPI Resp message Failed\n\r");
		goto Done;
	}

	/* Compare Data */
	for (Index = 0; Index < TEST_MSG_LEN; Index++) {
		if (ReqBuffer[Index] != (~TmpBufPtr[Index])) {
			xil_printf("Data Mismatch Expected: 0x%08x"
				   "Got: 0x%08x\r\n", RespBuffer[Index],
				   TmpBufPtr[Index]);
			goto Done;
		}
	}

Done:
	return Status;
}

static void DoneHandler(void *CallBackRef)
{
	RecvDone = 1;
}

static void ErrorHandler(void *CallBackRef, u32 Mask)
{
	ErrorStatus = Mask;
}
