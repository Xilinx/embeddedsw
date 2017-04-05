/******************************************************************************
*
* Copyright (C) 2007 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xmbox_tapp_example.c
*
* This file contains a design example for using the Mailbox hardware and
* driver XMbox
*
* This example attempts to send a known message through the mailbox from
* the processor identified as 0 (XPAR_CPU_ID=0) to the other processor.
* The message is received by the receiver and the test passes.
* Since the application is running on two seperate processors, the
* initiator declares success when the message is sent and the receiver
* declares success when the message is received. There is no feedback
* to the initiator so a terminal is required for each processor to verify
* that the test passed for both sides.
*
* The example assumes there are two processors availabile in the system that
* are expected to inter-communicate. If Mailbox is connected to only one
* Processor then Data has to be sent from one port and should be received
* from another port.
*
* This example has been tested on ML505 Hardware Evaluation board.
*
* @note
*
* These code fragments will illustrate how the XMbox component can be used to:
*   - Initialize the Mailbox core
*   - pass data between two processors.
*
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a va	      First release
* 1.00a ecm  06/09/07 Cleanup, new coding standard, check into XCS
* 1.01a ecm  08/28/08 converted to testapp example
* 1.01a ecm  10/28/08 corrected output to match the requirements for TestApp
*		      integration.
* 2.00a hm   04/09/09 Added receiving data from the other processor.
* 3.01a sdm  05/06/10 Cleanup for coding guidelines and removed printfs
* 4.1   ms   01/23/17 Modified xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*       ms   04/05/17 Added tabspace for return statements in functions for
*                     proper documentation while generating doxygen.
*</pre>
*****************************************************************************/

/**************************** Include Files **********************************/

#include "xmbox.h"
#include "xstatus.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/*
* If the XPAR_CPU_ID != 0 for this instance, the only other option is 1
* to make the example work. It is possible that there are more than
* 2 processors in the system but there must always be a 0
*/
#if XPAR_CPU_ID != 0
#define MY_CPU_ID 1
#else
#define MY_CPU_ID XPAR_CPU_ID
#endif /* XPAR_CPU_ID != 0 */

#define MSGSIZ		1024 /* Size of the buffer for received message */

#define HELLO_SIZE	40

#define TIMEOUT_MAX_COUNT	0xF0000000  /* Max count to wait for the message */

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define MBOX_DEVICE_ID		XPAR_MBOX_0_DEVICE_ID

/**************************** Type Definitions *******************************/

#define printf xil_printf	/* A smaller footprint printf */

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
static XMbox Mbox;	/* Instance of the Mailbox driver */

/* Buffer for storing received the message */
char RecvMsg[MSGSIZ] __attribute__ ((aligned(4)));

u32 Temp1pad = 0;		/* alignment */

/* Sent Message */
char *ProducerHello = "Hello! The Producer greets the Consumer...";

/************************** Function Prototypes ******************************/

int MailboxExample(u16 MboxDeviceId);

static int MailboxExample_Send(XMbox *MboxInstancePtr, int CPU_Id);
static int MailboxExample_Receive(XMbox *MboxInstancePtr, int CPU_Id);


/*****************************************************************************/
/**
* This function is the main function for the mailbox example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
*****************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	printf ("MailboxExample :\tStarts for CPU %d.\r\n", MY_CPU_ID);

	if (MailboxExample(MBOX_DEVICE_ID) != XST_SUCCESS) {
		printf("MailboxExample :\t mbox tapp Example Failed.\r\n");
		printf("MailboxExample :\tEnds.\r\n");
		return XST_FAILURE;
	}

	printf("MailboxExample :\tSuccessfully ran mbox tapp Example \r\n");
	printf("MailboxExample :\tEnds for CPU %d.\r\n", MY_CPU_ID);

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function sends a message to and receives a message from the other
* processor.
*
* @param	MboxDeviceID is the device Id of the MailBox.
*
* @return
*		- XST_SUCCESS if the test passes.
*		- XST_FAILURE if the test fails.
*
* @note		None.
*
******************************************************************************/
int MailboxExample(u16 MboxDeviceId)
{

	XMbox_Config *ConfigPtr;
	int Status;

	/*
	 * Lookup configuration data in the device configuration table.
	 * Use this configuration info down below when initializing this
	 * component.
	 */
	ConfigPtr = XMbox_LookupConfig(MboxDeviceId );
	if (ConfigPtr == (XMbox_Config *)NULL) {
		return XST_FAILURE;
	}

	/*
	 * Perform the rest of the initialization.
	 */
	Status = XMbox_CfgInitialize(&Mbox, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Send the hello */
	Status = MailboxExample_Send(&Mbox, MY_CPU_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Receive the hello and verify the message */
	Status = MailboxExample_Receive(&Mbox, MY_CPU_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sends the hello message to the other processor.
*
* @param	MboxInstancePtr is the instance pointer for the XMbox.
* @param	CPU_Id is the CPU ID for the program that is running on.
*
* @return	- XST_SUCCESS if the send succeeds.
*		- XST_FAILURE if the send fails.
*
* @note		None.
*
******************************************************************************/
static int MailboxExample_Send(XMbox *MboxInstancePtr, int CPU_Id)
{
	int Status;
	u32 Nbytes;
	u32 BytesSent;

	Nbytes = 0;

	while (Nbytes != HELLO_SIZE) {
		/* Write a message to the mbox */
		Status = XMbox_Write(MboxInstancePtr,
					(u32 *)((u8 *)ProducerHello + Nbytes),
					HELLO_SIZE - Nbytes,
					&BytesSent);

		if (Status == XST_SUCCESS)
			Nbytes += BytesSent;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function receives a message from the other processor and verifies that
* it's the expected message.
*
* @param	MboxInstancePtr is the instance pointer for the XMbox.
* @param	CPU_Id is the CPU ID for the program that is running on.
*
* @return	- XST_SUCCESS if the receive succeeds.
*		- XST_FAILURE if the receive fails.
*
* @note		None.
*
******************************************************************************/
static int MailboxExample_Receive(XMbox *MboxInstancePtr, int CPU_Id)
{
	int Status;
	u32 Nbytes;
	u32 BytesRcvd;
	int Timeout;

	Nbytes = 0;
	Timeout = 0;

	while (Nbytes < HELLO_SIZE) {
		/* Read a message from the mbox */
		Status = XMbox_Read(MboxInstancePtr,
					(u32 *)(RecvMsg + Nbytes),
					HELLO_SIZE - Nbytes, &BytesRcvd);

		if (Status == XST_SUCCESS)
			Nbytes += BytesRcvd;

		if (Timeout++ > TIMEOUT_MAX_COUNT)
			return XST_FAILURE;
	}

	/* Compare the recieved the message is the same as we expect */
	if (memcmp(RecvMsg, ProducerHello, HELLO_SIZE)) {
		return XST_FAILURE;
	} else {
		return XST_SUCCESS;
	}
}
