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
/******************************************************************************/
/**
*
* @file xtmr_manager_polled_example.c
*
* This file contains a design example using the TMRManager driver (XTMRManager) and
* hardware device using the polled mode.
*
* @note
*
* The user must provide a physical loopback such that data which is
* transmitted will be received.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xtmr_manager.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define TMRMANAGER_DEVICE_ID	XPAR_TMRMANAGER_0_DEVICE_ID

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the TMRManager, this constant must be 16 bytes or less since
 * this is a single threaded non-interrupt driven example such that the
 * entire buffer will fit into the transmit and receive FIFOs of the TMRManager.
 */
#define TEST_BUFFER_SIZE 16

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int TMRManagerPolledExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XTMRManager TMRManager;		/* Instance of the TMRManager Device */

/*
 * The following buffers are used in this example to send and receive data
 * with the TMRManager.
 */
u8 SendBuffer[TEST_BUFFER_SIZE];	/* Buffer for Transmitting Data */
u8 RecvBuffer[TEST_BUFFER_SIZE];	/* Buffer for Receiving Data */

/*****************************************************************************/
/**
*
* Main function to call the TMR Manager polled example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the TMRManager polled example, specify the Device ID that is
	 * generated in xparameters.h
	 */
	Status = TMRManagerPolledExample(TMRMANAGER_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}


/****************************************************************************/
/**
* This function does a minimal test on the TMRManager device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XTMRManager component.
*
* This function sends data and expects to receive the data thru the TMRManager
* such that a  physical loopback must be done with the transmit and receive
* signals of the TMRManager.
*
* This function polls the TMRManager and does not require the use of interrupts.
*
* @param	DeviceId is the Device ID of the TMRManager and is the
*		XPAR_<tmr_manager_instance>_DEVICE_ID value from xparameters.h.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
*
* @note
*
* This function calls the TMRManager driver functions in a blocking mode such that
* if the transmit data does not loopback to the receive, this function may
* not return.
*
****************************************************************************/
int TMRManagerPolledExample(u16 DeviceId)
{
	int Status;
	unsigned int SentCount;
	unsigned int ReceivedCount = 0;
	int Index;

	/*
	 * Initialize the TMRManager driver so that it is ready to use.
	 */
	Status = XTMRManager_Initialize(&TMRManager, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XTMRManager_SelfTest(&TMRManager);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the send buffer bytes with a pattern to send and the
	 * the receive buffer bytes to zero.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		SendBuffer[Index] = Index;
		RecvBuffer[Index] = 0;
	}

	/*
	 * Send the buffer through the TMRManager waiting til the data can be sent
	 * (block), if the specified number of bytes was not sent successfully,
	 * then an error occurred.
	 */
	SentCount = XTMRManager_Send(&TMRManager, SendBuffer, TEST_BUFFER_SIZE);
	if (SentCount != TEST_BUFFER_SIZE) {
		return XST_FAILURE;
	}

	/*
	 * Receive the number of bytes which is transfered.
	 * Data may be received in fifo with some delay hence we continuously
	 * check the receive fifo for valid data and update the receive buffer
	 * accordingly.
	 */
	while (1) {
		ReceivedCount += XTMRManager_Recv(&TMRManager,
					   RecvBuffer + ReceivedCount,
					   TEST_BUFFER_SIZE - ReceivedCount);
		if (ReceivedCount == TEST_BUFFER_SIZE) {
			break;
		}
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
