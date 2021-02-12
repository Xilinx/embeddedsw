/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanfd_selftest.c
* @addtogroup canfd_v2_5
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

#define XCANFD_MAX_FRAME_SIZE_IN_BYTES 72 /**< Maximum Frame size */

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/* Buffers to hold frames to send and receive. */
static u32 TxFrame[XCANFD_MAX_FRAME_SIZE_IN_BYTES]; /**< TxFrame Buffer */
static u32 RxFrame[XCANFD_MAX_FRAME_SIZE_IN_BYTES]; /**< RxFrame Buffer */

/************************** Function Prototypes *****************************/

/* Message Id Constant. */
#define TEST_MESSAGE_ID	1024	/**< Message id */

/* CAN Dlc Value */
#define TEST_CANFD_DLC	8	/**< DLC Value */

/* CAN FD FilterIndex Value */
#define TEST_MAIL_BOX_MASK 0xFFFFFFFFU	/**< Mailbox Fileter Index Value */

#define TEST_BRPR_BAUD_PRESCALAR	29 /**< Baud Rate Prescalar */

#define TEST_BTR_SYNCJUMPWIDTH		3  /**< Synchronization Jump Width */
#define TEST_BTR_SECOND_TIMESEGMENT	2  /**< Time Segment 2 */
#define TEST_BTR_FIRST_TIMESEGMENT	15 /**< Time Segment 1 */

#define TEST_FBRPR_BAUD_PRESCALAR	29 /**< Baud Rate Prescalar for canfd */

#define TEST_FBTR_SYNCJUMPWIDTH		3  /**< Synchronization Jump Width for Canfd */
#define TEST_FBTR_SECOND_TIMESEGMENT	2  /**< Time Segment 2 for Canfd */
#define TEST_FBTR_FIRST_TIMESEGMENT	15 /**< Time Segment 1 for Canfd */

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
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XCanFd_Reset(InstancePtr);

	/*
	 * The device should enter Configuration Mode immediately after the
	 * reset above is finished. Now check the mode and return error code if
	 * it is not Configuration Mode.
	 */

	if (XCanFd_GetMode(InstancePtr) != (u8)XCANFD_MODE_CONFIG) {
		return (s32)XST_FAILURE;
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
	Status = (u32)XCanFd_SetBaudRatePrescaler(InstancePtr, TEST_BRPR_BAUD_PRESCALAR);
	if (Status != (u32)XST_SUCCESS) {
		return (s32)Status;
	}

	/*
	 * Configure the Bit Timing Values in
	 * Arbitration Phase.
	 */
	Status = (u32)XCanFd_SetBitTiming(InstancePtr, TEST_BTR_SYNCJUMPWIDTH,
		TEST_BTR_SECOND_TIMESEGMENT,TEST_BTR_FIRST_TIMESEGMENT);
	if (Status != (u32)XST_SUCCESS) {
		return (s32)Status;
	}

	 /* Configure the Baud Rate Prescalar in Data Phase */
	Status = (u32)XCanFd_SetFBaudRatePrescaler(InstancePtr, TEST_FBRPR_BAUD_PRESCALAR);
	if (Status != (u32)XST_SUCCESS) {
		return (s32)Status;
	}

	/* Configure the Bit Timing Values in Data Phase */
	Status = (u32)XCanFd_SetFBitTiming(InstancePtr,TEST_FBTR_SYNCJUMPWIDTH,
		TEST_FBTR_SECOND_TIMESEGMENT,TEST_FBTR_FIRST_TIMESEGMENT);
	if (Status != (u32)XST_SUCCESS) {
		return (s32)Status;
	}

	XCanFd_EnterMode(InstancePtr, XCANFD_MODE_LOOPBACK);
	Status = XCanFd_GetMode(InstancePtr);
	while (Status != (u32)XCANFD_MODE_LOOPBACK) {
		Status = XCanFd_GetMode(InstancePtr);
	}

	/*
	 * Create a frame to send with known values so we can verify them
	 * on receive.
	 */
	TxFrame[0] = XCanFd_CreateIdValue((u32)TEST_MESSAGE_ID, (u32)0, (u32)0, (u32)0, (u32)0);
	TxFrame[1] = XCanFd_Create_CanFD_Dlc_BrsValue((u32)TEST_CANFD_DLC);

	Dlc = (u32)XCanFd_GetDlc2len(TxFrame[1] & XCANFD_DLCR_DLC_MASK,
		EDL_CANFD);
	FramePtr = (u8 *) (&TxFrame[2]);

	for (Index = 0; Index < Dlc; Index++) {
		*FramePtr = (u8) Index;
		FramePtr = FramePtr +(u8)1;
	}

	/*Check the design, if it is in MailBox Mode */
	if (XCANFD_GET_RX_MODE(InstancePtr) == (u32)1) {
		IdValue = XCanFd_CreateIdValue((u32)TEST_MESSAGE_ID, (u32)0, (u32)0, (u32)0, (u32)0);
		for (BuffNr= 0;BuffNr < InstancePtr->CanFdConfig.NumofRxMbBuf;
				BuffNr++)
		{
			(void)XCanFd_RxBuff_MailBox_DeActive(InstancePtr,BuffNr);
			(void)XCanFd_Set_MailBox_IdMask(InstancePtr,BuffNr,
				TEST_MAIL_BOX_MASK,IdValue);
			Status = XCanFd_RxBuff_MailBox_Active(InstancePtr,BuffNr);
			if (Status != (u32)XST_SUCCESS) {
				return (s32)Status;
			}
		}
	}
	else {
		/*In Sequential Mode */
		XCanFd_AcceptFilterDisable(InstancePtr,XCANFD_AFR_UAF_ALL_MASK);
		XCanFd_AcceptFilterEnable(InstancePtr,XCANFD_AFR_UAF_ALL_MASK);
	}

	/* Send the frame. */
	Status = (u32)XCanFd_Send(InstancePtr,TxFrame,&TxBuffer);
	if (Status != (u32)XST_SUCCESS) {
		return (s32)Status;
	}

	/* Wait until buffer is transmitted. */
	Value = (u32)XCanFd_IsBufferTransmitted(InstancePtr,TxBuffer);
	while ( Value == (u32)FALSE) {
		Value = (u32)XCanFd_IsBufferTransmitted(InstancePtr,TxBuffer);
	}

	if (XCANFD_GET_RX_MODE(InstancePtr) == (u32)1) {
		Status = XCanFd_Recv_Mailbox(InstancePtr, RxFrame);
	}
	else{
		Status = XCanFd_Recv_Sequential(InstancePtr, RxFrame);
	}
	if (Status != (u32)XST_SUCCESS) {
		return (s32)Status;
	}
	Dlc = (u32)XCanFd_GetDlc2len(RxFrame[1] & XCANFD_DLCR_DLC_MASK,
		EDL_CANFD);

	/* Verify Identifier and Data Length Code. */
	if (RxFrame[0] != XCanFd_CreateIdValue((u32)TEST_MESSAGE_ID, (u32)0, (u32)0, (u32)0, (u32)0)) {
		return (s32)XST_FAILURE;
	}
	if ((u8)TEST_CANFD_DLC != (u8)XCanFd_GetLen2Dlc((s32)Dlc)) {
		return (s32)XST_FAILURE;
	}

	/* Verify Data field contents. */
	FramePtr = (u8 *)(&RxFrame[2]);
	for (Index = 0; Index < Dlc; Index++) {
		if (*FramePtr++ != (u8)Index) {
			return (s32)XST_FAILURE;
		}
	}

	XCanFd_Reset(InstancePtr);

	return (s32)XST_SUCCESS;
}
/** @} */
