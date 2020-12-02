/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanps_selftest.c
* @addtogroup canps_v3_5
* @{
*
* This file contains a diagnostic self-test function for the XCanPs driver.
*
* Read xcanps.h file for more information.
*
* @note
* The  Baud Rate Prescaler Register (BRPR) and Bit Timing Register(BTR)
* are setup such that CAN baud rate equals 40Kbps, given the CAN clock
* equal to 24MHz. These need to be changed based on the desired baudrate
* and CAN clock frequency.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.00a xd/sv  01/12/10 First release
* 2.1 adk 		23/08/14 Fixed CR:798792 Peripheral test for CANPS IP in
*						 SDK claims a 40kbps baud rate but it's not.
* 3.00  kvn    02/13/15 Modified code for MISRA_C:2012 compliance.
* 3.5	sne    07/01/20 Fixed MISRAC warnings.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xstatus.h"
#include "xcanps.h"

/************************** Constant Definitions ****************************/

#define XCANPS_MAX_FRAME_SIZE_IN_WORDS ((XCANPS_MAX_FRAME_SIZE) / (sizeof(u32)))

#define FRAME_DATA_LENGTH	8U /* Frame Data field length */

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/*
 * Buffers to hold frames to send and receive. These are declared as global so
 * that they are not on the stack.
 */
static u32 TxFrame[XCANPS_MAX_FRAME_SIZE_IN_WORDS];
static u32 RxFrame[XCANPS_MAX_FRAME_SIZE_IN_WORDS];

/************************** Function Prototypes *****************************/

/*****************************************************************************/
/**
*
* This function runs a self-test on the CAN driver/device. The test resets
* the device, sets up the Loop Back mode, sends a standard frame, receives the
* frame, verifies the contents, and resets the device again.
*
* Note that this is a destructive test in that resets of the device are
* performed. Refer the device specification for the device status after
* the reset operation.
*
*
* @param	InstancePtr is a pointer to the XCanPs instance.
*
* @return
*		- XST_SUCCESS if the self-test passed. i.e., the frame
*		  received via the internal loop back has the same contents as
*		  the frame sent.
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
s32 XCanPs_SelfTest(XCanPs *InstancePtr)
{
	u8 *FramePtr;
	s32 Status;
	u32 Index;
	u8 GetModeResult;
	u32 RxEmptyResult;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XCanPs_Reset(InstancePtr);

	/*
	 * The device should enter Configuration Mode immediately after
	 * reset above is finished. Now check the mode and return error code if
	 * it is not Configuration Mode.
	 */
	if (XCanPs_GetMode(InstancePtr) != XCANPS_MODE_CONFIG) {
		Status = (s32)XST_FAILURE;
		return Status;
	}

	/*
	 * Setup Baud Rate Prescaler Register (BRPR) and Bit Timing Register
	 * (BTR) such that CAN baud rate equals 40Kbps, given the CAN clock
	 * equal to 24MHz. For more information see the CAN 2.0A, CAN 2.0B,
	 * ISO 11898-1 specifications.
	 */
	(void)XCanPs_SetBaudRatePrescaler(InstancePtr, (u8)29U);
	(void)XCanPs_SetBitTiming(InstancePtr, (u8)3U, (u8)2U, (u8)15U);

	/*
	 * Enter the loop back mode.
	 */
	XCanPs_EnterMode(InstancePtr, XCANPS_MODE_LOOPBACK);
	GetModeResult = XCanPs_GetMode(InstancePtr);
	while (GetModeResult != ((u8)XCANPS_MODE_LOOPBACK)) {
		GetModeResult = XCanPs_GetMode(InstancePtr);
	}

	/*
	 * Create a frame to send with known values so we can verify them
	 * on receive.
	 */
	TxFrame[0] = (u32)XCanPs_CreateIdValue((u32)2000U, (u32)0U, (u32)0U, (u32)0U, (u32)0U);
	TxFrame[1] = (u32)XCanPs_CreateDlcValue((u32)8U);

	FramePtr = (u8 *)(&TxFrame[2]);
	for (Index = 0U; Index < 8U; Index++) {
		if(*FramePtr != 0U) {
			*FramePtr = (u8)Index;
			FramePtr++;
		}
	}

	/*
	 * Send the frame.
	 */
	Status = XCanPs_Send(InstancePtr, TxFrame);
	if (Status != (s32)XST_SUCCESS) {
		Status = (s32)XST_FAILURE;
		return Status;
	}

	/*
	 * Wait until the frame arrives RX FIFO via internal loop back.
	 */
	RxEmptyResult = XCanPs_ReadReg(((InstancePtr)->CanConfig.BaseAddr),
			XCANPS_ISR_OFFSET) & XCANPS_IXR_RXNEMP_MASK;

	while (RxEmptyResult == (u32)0U) {
		RxEmptyResult = XCanPs_ReadReg(((InstancePtr)->CanConfig.BaseAddr),
				XCANPS_ISR_OFFSET) & XCANPS_IXR_RXNEMP_MASK;
	}

	/*
	 * Receive the frame.
	 */
	Status = XCanPs_Recv(InstancePtr, RxFrame);
	if (Status != (s32)XST_SUCCESS) {
		Status = (s32)XST_FAILURE;
		return Status;
	}

	/*
	 * Verify Identifier and Data Length Code.
	 */
	if (RxFrame[0] !=
		(u32)XCanPs_CreateIdValue((u32)2000U, (u32)0U, (u32)0U, (u32)0U, (u32)0U)) {
		Status = (s32)XST_FAILURE;
		return Status;
	}

	if ((RxFrame[1] & ~XCANPS_DLCR_TIMESTAMP_MASK) != TxFrame[1]) {
		Status = (s32)XST_FAILURE;
		return Status;
	}


	for (Index = 2U; Index < (XCANPS_MAX_FRAME_SIZE_IN_WORDS); Index++) {
		if (RxFrame[Index] != TxFrame[Index]) {
			Status = (s32)XST_FAILURE;
			return Status;
		}
	}

	/*
	 * Reset device again before returning to the caller.
	 */
	XCanPs_Reset(InstancePtr);

	Status = (s32)XST_SUCCESS;
	return Status;
}


/** @} */
