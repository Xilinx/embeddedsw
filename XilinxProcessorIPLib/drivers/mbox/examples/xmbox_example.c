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
*******************************************************************************/
/******************************************************************************/
/**
* @file mbox_example.c
*
* This file contains a design example for using the Mailbox hardware and driver
* XMbox
*
* The example assumes there are two processors availabile in the system that
* are expected to inter-communicate.
*
* This example has been tested on ML505 Hardware Evaluation board.
*
* @note
*
* These code fragments will illustrate how the XMbox component can be used to:
*   - Initialize the Mailbox core
*   - Pass data between two processes
*
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a va            First release
* 1.00a ecm  06/09/07 Cleanup, new coding standard, check into XCS
* 3.01a sdm  05/06/10 Cleanup for coding guidelines
*</pre>
*******************************************************************************/

/***************************** Include Files **********************************/

#include "xmbox.h"
#include "xstatus.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

#define MSGSIZ  1024	/* Size of the actual Message */

#define HELLO_SIZE 40	/* Size of the Hello Message */

#if XPAR_CPU_ID == 2
#define MY_CPU_ID 1
#else
#define MY_CPU_ID XPAR_CPU_ID
#endif


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define printf xil_printf	/* A smaller footprint printf */

/************************** Variable Definitions *****************************/

char *Role[2] = { "PRODUCER",
		  "CONSUMER" };

extern char data[];
char *SendMsg = data;
char RecvMsg[MSGSIZ] __attribute__ ((aligned(4)));

u32 temp1pad = 0;
char *hello = "Hello! The Producer greets the Consumer...";
u32 temp2pad = 0;
char *rhello = "Hello! The Consumer greets the Producer...";

/************************** Function Prototypes ******************************/

int ProdCon (void);

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
******************************************************************************/
int main(void)
{
	int Status;

	printf ("-- Entering main() --\r\n");
	printf ("PRODCON :\tStarts.\r\n");

	Status = ProdCon();
	if (Status != XST_SUCCESS) {
		printf ("PRODCON :\tFailed.\r\n");
	} else {
		printf ("PRODCON :\tSucceeded.\r\n");
	}

	printf ("PRODCON :\tEnds.\r\n");
	printf ("-- Exiting main() --\r\n");

	return Status;
}

/******************************************************************************/
/**
*
* This function performs the producer and consumer console functionality.
* At compile time the role, producer or consumer, is determined.
*
* @param	None.
*
* @return	XST_SUCCESS.
*
* @note		None.
*
*******************************************************************************/
int ProdCon ()
{
	XMbox Mbox;
	XMbox_Config *ConfigPtr;
	int Status;
	u32 NumBytes;
	u32 Sent;
	u32  Rcvd;
	int Index;

	printf ("(%s):\tStarts.\r\n", Role[MY_CPU_ID]);

	/*
	 * Lookup configuration data in the device configuration table.
	 * Use this configuration info down below when initializing this
	 * component.
	 */
	ConfigPtr = XMbox_LookupConfig(XPAR_MBOX_4_DEVICE_ID );
	if (ConfigPtr == (XMbox_Config *)NULL) {
		printf ("(%s):\tLookupConfig Failed.%8.8x\r\n",
			Role[MY_CPU_ID], XPAR_MBOX_4_DEVICE_ID );
		return XST_FAILURE;
	}

	/*
	 * Perform the rest of the initialization
	 */
	Status = XMbox_CfgInitialize(&Mbox, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#if MY_CPU_ID == 0
	/* First send the hello */
	printf ("CPU 0 WriteBlocking call\n");
	/* Write a message to the Mbox */
	XMbox_WriteBlocking(&Mbox, (u32*)((u8*)hello), HELLO_SIZE);

	printf ("(%s):\tsent %d bytes.\r\n", Role[MY_CPU_ID], HELLO_SIZE);

	printf ("(%s):\tSuccessfully sent the message --> \r\n\r\n\t--[%s]--\r\n\r\n", Role[MY_CPU_ID], hello);
#else
	/* First recv the hello */
	printf ("CPU 1 ReadBlocking call\n");
	/* Read a message from the Mbox */
	XMbox_ReadBlocking(&Mbox, (u32*)(RecvMsg), HELLO_SIZE);

	printf ("(%s):\tSuccessfully Rcvd the message --> \r\n\r\n\t--[%s]--\r\n\r\n", Role[MY_CPU_ID], RecvMsg);
#endif

#if MY_CPU_ID == 0
	/* Now get back a hello */
	/* Read a message from the Mbox */
	printf ("CPU 0 ReadBlocking call\n");
	XMbox_ReadBlocking (&Mbox, (u32*)(RecvMsg), HELLO_SIZE);

	printf ("(%s):\tSuccessfully Rcvd the message --> \r\n\r\n\t--[%s]--\r\n\r\n", Role[MY_CPU_ID], RecvMsg);
#else
	/* First recv the hello */
	NumBytes = 0;
	/* Write a message to the Mbox */
	printf ("CPU 1 WriteBlocking call\n");
	XMbox_WriteBlocking (&Mbox, (u32*)((u8*)rhello), HELLO_SIZE);

	printf ("(%s):\tsent %d bytes.\r\n", Role[MY_CPU_ID], Sent);

	printf ("(%s):\tSuccessfully sent the message --> \r\n\r\n\t--[%s]--\r\n\r\n",
		    Role[MY_CPU_ID], rhello);
#endif


#if MY_CPU_ID == 0
	/* Now send the full message */
	printf ("(%s):\tNow sending the actual message -->\r\n",
		    Role[MY_CPU_ID]);
	/* Write a message to the Mbox */
	XMbox_WriteBlocking (&Mbox, (u32*)(SendMsg), MSGSIZ);

	printf ("(%s):\tSuccessfully sent the full message.\r\n",
			Role[MY_CPU_ID]);
#else
	/* Now recv the full message */
	printf ("(%s):\tNow receiving the actual message -->\r\n",
		    Role[MY_CPU_ID]);
	/* Read a message from the Mbox */
	XMbox_ReadBlocking (&Mbox, (u32*)(RecvMsg), MSGSIZ);

	printf ("(%s):\tSuccessfully Rcvd the full message.\r\n",
		    Role[MY_CPU_ID]);

	if (memcmp (RecvMsg, data, MSGSIZ)) {
		printf ("(%s):\tError! Rcvd message does not match expected.\r\n",
			    Role[MY_CPU_ID]);
		return XST_FAILURE;
	} else {
		printf ("(%s):\tSuccess! Rcvd message does match expected.\r\n",
			    Role[MY_CPU_ID]);
	}
#endif

	printf ("(%s):\tEnds.\r\n", Role[MY_CPU_ID]);
	return XST_SUCCESS;
}
