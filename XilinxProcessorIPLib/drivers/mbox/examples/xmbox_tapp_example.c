/******************************************************************************
*
* (c) Copyright 2007-2013 Xilinx, Inc. All rights reserved.
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
		printf("MailboxExample :\tFailed.\r\n");
		printf("MailboxExample :\tEnds.\r\n");
		return XST_FAILURE;
	}

	printf("MailboxExample :\tSucceeded.\r\n");
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
* @return	- XST_SUCCESS if the test passes.
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
