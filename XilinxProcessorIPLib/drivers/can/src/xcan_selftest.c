/******************************************************************************
*
* Copyright (C) 2005 - 2015 Xilinx, Inc.  All rights reserved.
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
* @file xcan_selftest.c
* @addtogroup can_v3_2
* @{
*
* This file contains a diagnostic self-test function for the XCan driver.
*
* Please see xcan.h for more information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------
* 1.00a xd   04/12/05 First release
* 1.10a mta  05/13/07 Updated to new coding style
* 2.00a ktn  10/22/09 Updated to use the HAL APIs/macros.
*		      The macros have been renamed to remove _m from the name.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xstatus.h"
#include "xcan.h"

/************************** Constant Definitions ****************************/

#define XCAN_MAX_FRAME_SIZE_IN_WORDS (XCAN_MAX_FRAME_SIZE / sizeof(u32))

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/* Buffers to hold frames to send and receive. */
static u32 TxFrame[XCAN_MAX_FRAME_SIZE_IN_WORDS];
static u32 RxFrame[XCAN_MAX_FRAME_SIZE_IN_WORDS];

/************************** Function Prototypes *****************************/

/*****************************************************************************/
/**
*
* This function runs a self-test on the CAN driver/device. The test resets
* the device, sets up the Loop Back mode, sends a standard frame, receives the
* frame, verifies the contents, and resets the device again.
*
* Note that this is a destructive test in that resets of the device are
* performed. Refer to the device specification for the device status
* after the reset operation.
*
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
*
* @return
*		- XST_SUCCESS   if the self-test passed. i.e., the frame
*		received via the internal loop back has the same contents as the
* 		sent frame.
* 		- XST_FAILURE   Otherwise.
*
* @note
*
* If the CAN device does not work properly, this function may enter an
* infinite loop and will never return to the caller.
* <br><br>
* If XST_FAILURE is returned, the device is not reset so that the caller could
* have a chance to check reason(s) causing the failure.
*
******************************************************************************/
int XCan_SelfTest(XCan *InstancePtr)
{
	u8 *FramePtr;
	u32 Result;
	u32 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Reset device first */
	XCan_Reset(InstancePtr);

	/*
	 * The device should enter Configuration Mode immediately after the
	 * reset above is finished. Now check the mode and return error code if
	 * it is not Configuration Mode.
	 */
	if (XCan_GetMode(InstancePtr) != XCAN_MODE_CONFIG) {
		return XST_FAILURE;
	}

	/*
	 * Setup Baud Rate Prescaler Register (BRPR) and Bit Timing Register
	 * (BTR) such that CAN baud rate equals 40Kbps, given the CAN clock
	 * equal to 24MHz.
	 */
	XCan_SetBaudRatePrescaler(InstancePtr, 29);
	XCan_SetBitTiming(InstancePtr, 3, 2, 15);

	/* Enter loopback mode  */
	XCan_EnterMode(InstancePtr, XCAN_MODE_LOOPBACK);
	while (XCan_GetMode(InstancePtr) != XCAN_MODE_LOOPBACK);

	/*
	 * Create a frame to send with known values so we can verify them
	 * on receive.
	 */
	TxFrame[0] = XCan_CreateIdValue(2650, 0, 0, 0, 0);
	TxFrame[1] = XCan_CreateDlcValue(8);
	FramePtr = (u8 *) (&TxFrame[2]);
	for (Index = 0; Index < 8; Index++) {
		*FramePtr++ = (u8) Index;
	}

	/* Send the frame. */
	Result = XCan_Send(InstancePtr, TxFrame);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Wait until the frame arrives RX FIFO via internal loop back */
	while (XCan_IsRxEmpty(InstancePtr) == TRUE);

	/* Receive the frame */
	Result = XCan_Recv(InstancePtr, RxFrame);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Compare received frame with sent frame */
	for (Index = 0; Index < XCAN_MAX_FRAME_SIZE_IN_WORDS; Index++) {
		if (RxFrame[Index] != TxFrame[Index]) {
			return XST_FAILURE;
		}
	}

	/* Reset device again before return to the caller */
	XCan_Reset(InstancePtr);

	return XST_SUCCESS;
}


/** @} */
