/******************************************************************************
*
* Copyright (C) 2002 - 2015 Xilinx, Inc.  All rights reserved.
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
*
* @file xuartlite_low_level_example.c
*
* This file contains a design example using the low-level driver functions
* and macros of the UartLite driver (XUartLite).
*
* @note
*
* The user must provide a physical loopback such that data which is
* transmitted will be received.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who  Date	 Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00b rpm  04/25/02 First release
* 1.00b sv   06/13/05 Minor changes to comply to Doxygen and coding guidelines
* 2.00a ktn  10/20/09 Updated to use HAL processor APIs and minor changes
*		      for coding guidelines.
* 3.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xuartlite_l.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/


/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define UARTLITE_BASEADDR	   XPAR_UARTLITE_0_BASEADDR

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the UartLite, this constant must be 16 bytes or less so the
 * entire buffer will fit into the transmit and receive FIFOs of the UartLite.
 */
#define TEST_BUFFER_SIZE 16


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int UartLiteLowLevelExample(u32 UartliteBaseAddress);

/************************** Variable Definitions *****************************/

/*
 * The following buffers are used in this example to send and receive data
 * with the UartLite.
 */
u8 SendBuffer[TEST_BUFFER_SIZE]; /* Buffer for Transmitting Data */
u8 RecvBuffer[TEST_BUFFER_SIZE]; /* Buffer for Receiving Data */


/*****************************************************************************/
/**
*
* Main function to call the example.
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

	/*
	 * Run the UartLite Low level example, specify the BaseAddress that is
	 * generated in xparameters.h.
	 */
	Status = UartLiteLowLevelExample(UARTLITE_BASEADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Uartlite lowlevel Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Uartlite lowlevel Example\r\n");
	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This function does a minimal test on the UartLite device using the low-level
* driver macros and functions. This function sends data and expects to receive
* the data through the UartLite. A physical loopback must be done by the user
* with the transmit and receive signals of the UartLite.
*
* @param	UartliteBaseAddress is the base address of the UartLite device
*		and is the XPAR_<UARTLITE_instance>_BASEADDR value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
int UartLiteLowLevelExample(u32 UartliteBaseAddress)
{
	int Index;

	/*
	 * Initialize the send buffer bytes with a pattern to send and the
	 * the receive buffer bytes to zero.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		SendBuffer[Index] = Index + 'B';
		RecvBuffer[Index] = 0;
	}


	/*
	 * Send the entire transmit buffer.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		XUartLite_SendByte(UartliteBaseAddress, SendBuffer[Index]);
	}

	/*
	 * Receive the entire buffer's worth. Note that the RecvByte function
	 * blocks waiting for a character.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		RecvBuffer[Index] = XUartLite_RecvByte(UartliteBaseAddress);
	}

	/*
	 * Check the receive buffer data against the send buffer and verify the
	 * data was correctly received.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		if (SendBuffer[Index] != RecvBuffer[Index]) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
