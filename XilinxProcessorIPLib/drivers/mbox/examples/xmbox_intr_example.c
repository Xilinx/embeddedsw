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
* @file xmbox_intr_example.c
*
* This file contains a design example for using the Mailbox hardware and
* driver XMbox.
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
* are expected to inter-communicate.
*
* This example has been tested on ML505 Hardware Evaluation board.
*
* @note
*
* These code fragments will illustrate how the XMbox component can be used to:
*   - Initialize the Mailbox core.
*   - pass data between two processors.
*
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 2.00a hm		Example that sends and receives and uses interrupts
* 3.01a sdm  05/06/10	Cleanup for coding guidelines, removed printfs from all
*			the functions except the main function
* 4.1   ms   01/23/17 Modified xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*       ms   04/05/17 Added tabspace for return statements in functions for
*                     proper documentation while generating doxygen and
*                     modified filename tag to include the file in doxygen
*                     examples.
*</pre>
*******************************************************************************/

/***************************** Include Files **********************************/

#include "xmbox.h"
#include "xstatus.h"
#include "xparameters.h"
#include "xintc.h"

#include "xil_exception.h"
#include "xil_cache.h"

/************************** Constant Definitions *****************************/

/*
* If the XPAR_CPU_ID != 0 for this instance, the only other option is 1
* to make the example work. It is possible that there are more than
* 2 processors in the system but there must always be a 0.
*/

#if XPAR_CPU_ID != 0
#define MY_CPU_ID 1
#else
#define MY_CPU_ID XPAR_CPU_ID
#endif /* XPAR_CPU_ID != 0 */

int Timeout;

#define MSGSIZ  1024

#define HELLO_SIZE 40

#define TIMEOUT_MAX_COUNT	0x10000000  /* max count to wait for message */


#define MAILBOX_RIT	4	/* mailbox receive interrupt threshold */
#define MAILBOX_SIT	4	/* mailbox send interrupt threshold */


/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define MBOX_DEVICE_ID		XPAR_MBOX_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define MBOX_INTR_ID		XPAR_INTC_0_MBOX_0_VEC_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define printf		xil_printf	/* A smaller footprint printf */

/************************** Variable Definitions *****************************/
static XMbox Mbox;

#ifndef TESTAPP_GEN
static XIntc IntcInst;
#endif

static volatile int IntrCount = 0;
static volatile int IntrSTACount = 0;
static volatile int IntrRTACount = 0;
static volatile int IntrERRCount = 0;

char RecvMsg[MSGSIZ] __attribute__ ((aligned(4)));

u32 Temp1pad = 0;		/* alignment */
char *ProducerHello = "Hello! The Producer greets the Consumer...";


/************************** Function Prototypes ******************************/

int MailboxExample(XIntc *IntcInstancePtr, u16 MboxDeviceId, u16 MboxIntrId);

int MailboxExample_Send(XMbox *MboxInstancePtr, int CPU_Id, int Blocking);
int MailboxExample_Receive(XMbox *MboxInstancePtr, int CPU_Id, int Blocking);
int MailboxExample_Wait(volatile int *Count, char *Name, int Threshold);

static void MailboxIntrHandler(void *CallbackRef);

static int MailboxSetupIntrSystem(XIntc *IntcInstancePtr,
				  XMbox *MboxInstPtr,
				  u16 IntcDevId,
				  u16 MboxIntrId);

/*****************************************************************************/
/**
* This function is the main function for the mailbox interrupt example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	printf ("MailboxExample :\tStarts for CPU %d.\r\n", MY_CPU_ID);

	if (MailboxExample(&IntcInst, MBOX_DEVICE_ID, MBOX_INTR_ID)
				!= XST_SUCCESS) {
		printf("MailboxExample :\t mbox intr Example Failed.\r\n");
		return XST_FAILURE;
	}

	printf("MailboxExample :\tSuccessfully ran mbox intr Example\r\n");
	printf("MailboxExample :\tEnds for CPU %d.\r\n", MY_CPU_ID);

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function sends a message to and receives a message from the other
* processor. It also uses the interrupt to check whether the other processor
* has started to send or receive.
*
* @param	IntcInstancePtr is the device instance of the interrupt
*		controller that is being worked on.
* @param	MboxDeviceId is the Mailbox device ID.
* @param	MboxIntrId is the Mailbox interrupt ID.
*
* @return
*		- XST_SUCCESS if the test passes
*		- XST_FAILURE  if the test fails
*
* @note		None
*
*****************************************************************************/
int MailboxExample(XIntc *IntcInstancePtr, u16 MboxDeviceId, u16 MboxIntrId)
{
	XMbox_Config *ConfigPtr;
	int Status;
	u32 Nbytes;
	u32 BytesSent;
	u32 BytesRcvd;

	/*
	 * Lookup configuration data in the device configuration table.
	 * Use this configuration info down below when initializing this
	 * component.
	 */
	ConfigPtr = XMbox_LookupConfig(MboxDeviceId );
	if (ConfigPtr == (XMbox_Config *)NULL){
		return XST_FAILURE;
	}

	/*
	 * Perform the rest of the initialization
	 */
	Status = XMbox_CfgInitialize(&Mbox, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	/*
	 * Setup the interrupt system.
	 */
	Status = MailboxSetupIntrSystem(IntcInstancePtr,
					&Mbox,
					INTC_DEVICE_ID,
					MboxIntrId);

	/* Send the hello */
	Status = MailboxExample_Send(&Mbox, MY_CPU_ID, 0);

	if (Mbox.Config.UseFSL) {
		/* wait for FIFO not empty interrupt */
		Status = MailboxExample_Wait(&IntrCount, "FIFO Data", 0);
	} else {
		/* wait for RTA */
		Status = MailboxExample_Wait(&IntrRTACount, "RTA", 0);
	}

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* recv the hello */
	Status = MailboxExample_Receive(&Mbox, MY_CPU_ID, 0);

	/* wait for STA */
	if (Mbox.Config.UseFSL == 0) {
		if (MailboxExample_Wait(&IntrSTACount, "STA", 0))
			return XST_FAILURE;
	}

	/* test blocking send and receive */
	Status = MailboxExample_Send(&Mbox, MY_CPU_ID, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = MailboxExample_Receive(&Mbox, MY_CPU_ID, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Xil_ExceptionDisable();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function waits for a particular variable to exceed the threshold
* value. If it waits too long, it will time out and returns XST_FAILURE.
*
* @param	Count is the pointer to the variable to be examined.
* @param	Name is the name of the variable.
* @param	Threshold is the threshold value to be exceeded.
*
* @return
*		- XST_SUCCESS if the examined variable exceeds the threshold
*		 value.
*		- XST_FAILURE if the wait times out.
*
* @note		None
*
*****************************************************************************/
int MailboxExample_Wait(volatile int *Count, char *Name, int Threshold)
{
	int Timeout = 0;

	while (*Count <= Threshold) {
		Timeout++;
		if (Timeout > TIMEOUT_MAX_COUNT) {
			return XST_FAILURE;
		}
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
* @param	Blocking is set to 1 for the send to block till the data is
*		sent.
*
* @return
*		- XST_SUCCESS if the send succeeds
*		- XST_FAILURE if the send fails
*
* @note		None
*
******************************************************************************/
int MailboxExample_Send(XMbox *MboxInstancePtr, int CPU_Id, int Blocking)
{
	int Status;
	u32 Nbytes;
	u32 BytesSent;
	u32 BytesRcvd;

	Nbytes = 0;
	if (Blocking) {
		XMbox_WriteBlocking(MboxInstancePtr,
					(u32 *)ProducerHello,
					HELLO_SIZE);
	} else {
		while (Nbytes != HELLO_SIZE) {
			/* Write a message to the mbox */
			Status = XMbox_Write(MboxInstancePtr,
					(u32*)((u8*)ProducerHello + Nbytes),
					HELLO_SIZE - Nbytes,
					&BytesSent);
			if (Status == XST_SUCCESS)
				Nbytes += BytesSent;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function receives a message from the other processor verifies it's the
* expected message.
*
* @param	MboxInstancePtr is the instance pointer for the XMbox.
* @param	CPU_Id is the CPU ID for the program that is running on.
*
* @return
*		- XST_SUCCESS if the receive succeeds
*		- XST_FAILURE if the receive fails
*
* @note		None
*
******************************************************************************/
int MailboxExample_Receive(XMbox *MboxInstancePtr, int CPU_Id, int Blocking)
{
	int Status;
	u32 Nbytes;
	u32 BytesSent;
	u32 BytesRcvd;
	int Timeout;

	Nbytes = 0;
	Timeout = 0;

	if (Blocking) {
		XMbox_ReadBlocking(MboxInstancePtr, (u32 *)RecvMsg, HELLO_SIZE);
	} else {
		while (Nbytes < HELLO_SIZE) {
			/* Read a message from the mbox */
			Status = XMbox_Read(MboxInstancePtr,
					    (u32*)(RecvMsg + Nbytes),
					    HELLO_SIZE - Nbytes, &BytesRcvd);

			if (Status == XST_SUCCESS) {
				Nbytes += BytesRcvd;
			}

			if (Timeout++ > TIMEOUT_MAX_COUNT) {
				return XST_FAILURE;
			}
		}
	}

	if (memcmp(RecvMsg, ProducerHello, HELLO_SIZE)) {
		return XST_FAILURE;
	} else {
		return XST_SUCCESS;
	}
}

/*****************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the Mailbox device. This function is application specific since the
* actual system may or may not have an interrupt controller. The Mailbox
* device could be directly connected to a processor without an Interrupt
* controller.  The  user should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC
*		component.
* @param	MboxInstInstPtr is a pointer to the instance of the Mailbox.
* @param	MboxIntrId is the interrupt Id and is typically
*		XPAR_<INTC_instance>_<MBOX_instance>_IP2INTC_IRPT_INTR
*		value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
static int MailboxSetupIntrSystem(XIntc *IntcInstancePtr,
				  XMbox *MboxInstPtr,
				  u16 IntcDevId,
				  u16 MboxIntrId)
{
	int Status;

#ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use
	 */
	Status = XIntc_Initialize(IntcInstancePtr, IntcDevId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Sets the Threshold
	 */
	if (MboxInstPtr->Config.UseFSL == 0) {
		/*
		 * The Send Threshold and Receive Threshold are not for
		 * FSL interface.
		 */
		XMbox_SetSendThreshold(MboxInstPtr, MAILBOX_SIT);
		XMbox_SetReceiveThreshold(MboxInstPtr, MAILBOX_RIT);
	}

	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler
	 * performs the specific interrupt processing for the device
	 */
	Status = XIntc_Connect(IntcInstancePtr,
				MboxIntrId,
				(XInterruptHandler)MailboxIntrHandler,
				(void *)MboxInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * The interrupt bits are not for FSL interface.
	 */
	if (MboxInstPtr->Config.UseFSL == 0)
		XMbox_SetInterruptEnable(MboxInstPtr,
					 XMB_IX_STA | XMB_IX_RTA | XMB_IX_ERR);


#ifndef TESTAPP_GEN
	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts. Specify real mode so that
	 * the Mbox can generate interrupts through
	 * the interrupt controller
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Enable the interrupt for the Mbox
	 */
	XIntc_Enable(IntcInstancePtr, MboxIntrId);

#ifndef TESTAPP_GEN
	Xil_ExceptionInit();

	Xil_ExceptionEnable();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XIntc_InterruptHandler,
				IntcInstancePtr);
#endif /* TESTAPP_GEN */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the interrupt handler for this example.
*
* @param 	CallBackRef is a callback reference passed in by the upper layer
*		when setting the interrupt handler, and is passed back to the
*		upper layer when the interrupt handler is called.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void MailboxIntrHandler(void *CallbackRef)
{
	u32 Mask;
	XMbox *MboxInstPtr = (XMbox *)CallbackRef;

	IntrCount++;

	/*
	 * There is no Interrupt Status for FSL interface.
	 */
	if (MboxInstPtr->Config.UseFSL) {
		return;
	}

	Mask = XMbox_GetInterruptStatus(MboxInstPtr);

	if (Mask & XMB_IX_STA) {
		IntrSTACount++;
	}

	if (Mask & XMB_IX_RTA) {
		IntrRTACount++;
	}

	if (Mask & XMB_IX_ERR) {
		IntrERRCount++;
	}

	XMbox_ClearInterrupt(MboxInstPtr, Mask);
}
