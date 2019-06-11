/******************************************************************************
*
* Copyright (C) 2015 - 2019 Xilinx, Inc.  All rights reserved.
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
* @file xcanfd_selftest.c
* @addtogroup canfd_v2_1
* @{
*
* This file contains a diagnostic self-test function for the XCanFd driver.
*
* Please see xcanfd.h for more information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------
* 1.0   nsk  06/04/15 First release
* 1.2   mi   09/22/16 Fixed compilation warnings.
* 1.3   ask  08/08/18 Fixed Cppcheck warnings and updated the Canfd Id with
*						11 bit value
* 2.1	ask  09/21/18 Fixed CanFD hang issue in selftest by correcting the
*                     Configuration regarding the Baud Rate and bit timing
*                     for both Arbitration and Data Phase.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xstatus.h"
#include "xcanfd.h"

/************************** Constant Definitions ****************************/

#define XCANFD_MAX_FRAME_SIZE_IN_BYTES 72

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/* Buffers to hold frames to send and receive. */
static u32 TxFrame[XCANFD_MAX_FRAME_SIZE_IN_BYTES];
static u32 RxFrame[XCANFD_MAX_FRAME_SIZE_IN_BYTES];

/************************** Function Prototypes *****************************/

/* Message Id Constant. */
#define TEST_MESSAGE_ID	1024

/* CAN Dlc Value */
#define TEST_CANFD_DLC	8

/* CAN FD FilterIndex Value */
#define TEST_MAIL_BOX_MASK 0xFFFFFFFF

#define TEST_BRPR_BAUD_PRESCALAR	29

#define TEST_BTR_SYNCJUMPWIDTH		3
#define TEST_BTR_SECOND_TIMESEGMENT	2
#define TEST_BTR_FIRST_TIMESEGMENT	15

#define TEST_FBRPR_BAUD_PRESCALAR	29

#define TEST_FBTR_SYNCJUMPWIDTH		3
#define TEST_FBTR_SECOND_TIMESEGMENT	2
#define TEST_FBTR_FIRST_TIMESEGMENT	15

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
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	- XST_SUCCESS   if the self-test passed. i.e., the frame
*		received via the internal loop back has the same contents as the
* 		sent frame.
* 		- XST_FAILURE   Otherwise.
*
* @note		If the CAN device does not work properly, this function may enter an
* 		infinite loop and will never return to the caller.
* 		<br><br>
* 		If XST_FAILURE is returned, the device is not reset so that the caller could
* 		have a chance to check reason(s) causing the failure.
*
******************************************************************************/
int XCanFd_SelfTest(XCanFd *InstancePtr)
{
	u8 *FramePtr;
	u32 Index;
	u32 TxBuffer;
	u32 Dlc;
	u32 Status;

	u32 IdValue;
	u32 BuffNr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XCanFd_Reset(InstancePtr);

	/*
	 * The device should enter Configuration Mode immediately after the
	 * reset above is finished. Now check the mode and return error code if
	 * it is not Configuration Mode.
	 */

	if (XCanFd_GetMode(InstancePtr) != XCANFD_MODE_CONFIG) {
		return XST_FAILURE;
	}

	/*
	 * Setup Baud Rate Prescaler Register (BRPR) and Bit Timing Register
	 * (BTR) such that CAN baud rate equals 40Kbps, given the CAN clock
	 * equal to 24MHz.
	 */

	/*
	 * Configure the Baud Rate Prescalar in
	 * Arbitration Phase
	 */
	XCanFd_SetBaudRatePrescaler(InstancePtr, TEST_BRPR_BAUD_PRESCALAR);

	/*
	 * Configure the Bit Timing Values in
	 * Arbitration Phase.
	 */
	XCanFd_SetBitTiming(InstancePtr, TEST_BTR_SYNCJUMPWIDTH,
		TEST_BTR_SECOND_TIMESEGMENT,TEST_BTR_FIRST_TIMESEGMENT);

	 /* Configure the Baud Rate Prescalar in Data Phase */
	XCanFd_SetFBaudRatePrescaler(InstancePtr, TEST_FBRPR_BAUD_PRESCALAR);

	/* Configure the Bit Timing Values in Data Phase */
	XCanFd_SetFBitTiming(InstancePtr,TEST_FBTR_SYNCJUMPWIDTH,
		TEST_FBTR_SECOND_TIMESEGMENT,TEST_FBTR_FIRST_TIMESEGMENT);

	XCanFd_EnterMode(InstancePtr, XCANFD_MODE_LOOPBACK);
	while (XCanFd_GetMode(InstancePtr) != XCANFD_MODE_LOOPBACK);

	/*
	 * Create a frame to send with known values so we can verify them
	 * on receive.
	 */
	TxFrame[0] = XCanFd_CreateIdValue(TEST_MESSAGE_ID, 0, 0, 0, 0);
	TxFrame[1] = XCanFd_Create_CanFD_Dlc_BrsValue(TEST_CANFD_DLC);

	Dlc = XCanFd_GetDlc2len(TxFrame[1] & XCANFD_DLCR_DLC_MASK,
		EDL_CANFD);
	FramePtr = (u8 *) (&TxFrame[2]);

	for (Index = 0; Index < Dlc; Index++) {
		*FramePtr++ = (u8) Index;
	}

	/*Check the design, if it is in MailBox Mode */
	if (XCANFD_GET_RX_MODE(InstancePtr) == 1) {
		IdValue = XCanFd_CreateIdValue(TEST_MESSAGE_ID, 0, 0, 0, 0);
		for (BuffNr= 0;BuffNr < InstancePtr->CanFdConfig.NumofRxMbBuf;
				BuffNr++)
		{
			XCanFd_RxBuff_MailBox_DeActive(InstancePtr,BuffNr);
			XCanFd_Set_MailBox_IdMask(InstancePtr,BuffNr,
				TEST_MAIL_BOX_MASK,IdValue);
			XCanFd_RxBuff_MailBox_Active(InstancePtr,BuffNr);
		}
	}
	else {
		/*In Sequential Mode */
		XCanFd_AcceptFilterDisable(InstancePtr,XCANFD_AFR_UAF_ALL_MASK);
		XCanFd_AcceptFilterEnable(InstancePtr,XCANFD_AFR_UAF_ALL_MASK);
	}

	/* Send the frame. */
	Status = XCanFd_Send(InstancePtr,TxFrame,&TxBuffer);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Wait untill buffer is transmitted. */
	while (XCanFd_IsBufferTransmitted(InstancePtr,TxBuffer) == FALSE);

	if (XCANFD_GET_RX_MODE(InstancePtr) == 1) {
		Status = XCanFd_Recv_Mailbox(InstancePtr, RxFrame);
	}
	else{
		Status = XCanFd_Recv_Sequential(InstancePtr, RxFrame);
	}
	if (Status != XST_SUCCESS) {
		return Status;
	}
	Dlc = XCanFd_GetDlc2len(RxFrame[1] & XCANFD_DLCR_DLC_MASK,
		EDL_CANFD);

	/* Verify Identifier and Data Length Code. */
	if (RxFrame[0] != XCanFd_CreateIdValue(TEST_MESSAGE_ID, 0, 0, 0, 0)) {
		return XST_FAILURE;
	}
	if (TEST_CANFD_DLC != XCanFd_GetLen2Dlc(Dlc)) {
		return XST_FAILURE;
	}

	/* Verify Data field contents. */
	FramePtr = (u8 *)(&RxFrame[2]);
	for (Index = 0; Index < Dlc; Index++) {
		if (*FramePtr++ != (u8)Index) {
			return XST_FAILURE;
		}
	}

	XCanFd_Reset(InstancePtr);

	return XST_SUCCESS;
}
/** @} */
