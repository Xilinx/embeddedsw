/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
*
* @file xtmr_manager_low_level_example.c
*
* This file contains a design example using the low-level driver functions
* and macros of the TMRManager driver (XTMRManager).
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
* 1.0   sa   04/05/17 First release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xtmr_manager_l.h"

/************************** Constant Definitions *****************************/


/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define TMRMANAGER_BASEADDR	   XPAR_TMRMANAGER_0_BASEADDR

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the TMRManager, this constant must be 16 bytes or less so the
 * entire buffer will fit into the transmit and receive FIFOs of the TMRManager.
 */
#define TEST_BUFFER_SIZE 16


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int TMRManagerLowLevelExample(u32 TMRManagerBaseAddress);

/************************** Variable Definitions *****************************/

/*
 * The following buffers are used in this example to send and receive data
 * with the TMRManager.
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
	 * Run the TMRManager Low level example, specify the BaseAddress that is
	 * generated in xparameters.h.
	 */
	Status = TMRManagerLowLevelExample(TMRMANAGER_BASEADDR);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This function does a minimal test on the TMRManager device using the low-level
* driver macros and functions. This function sends data and expects to receive
* the data through the TMRManager. A physical loopback must be done by the user
* with the transmit and receive signals of the TMRManager.
*
* @param	TMRManagerBaseAddress is the base address of the TMRManager
*		device and is the XPAR_<TMRMANAGER_instance>_BASEADDR value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
int TMRManagerLowLevelExample(u32 TMRManagerBaseAddress)
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
		XTMRManager_SendByte(TMRManagerBaseAddress, SendBuffer[Index]);
	}

	/*
	 * Receive the entire buffer's worth. Note that the RecvByte function
	 * blocks waiting for a character.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		RecvBuffer[Index] = XTMRManager_RecvByte(TMRManagerBaseAddress);
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
