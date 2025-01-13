/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xcanfd_polled_example.c
*
* Contains an example of how to use the XCan driver directly. The example here
* shows using the driver/device in polled mode.
*
* @note
*
*
* The Baud Rate Prescaler Register (BRPR) and Bit Timing Register (BTR)
* are setup such that CAN baud rate equals 40Kbps, assuming that the
* the CAN clock frequency is 24MHz. The user needs to modify these values
* based on the desired baud rate and the CAN clock frequency. For more
* information see the CAN 2.0A, CAN 2.0B, ISO 11898-1 specifications.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   nsk    06/04/15 First release
* 1.2   ms     01/23/17 Modified xil_printf statement in main function to
*                       ensure that "Successfully ran" and "Failed" strings are
*                       available in all examples. This is a fix for CR-965028.
*       ms     04/05/17 Added tabspace for return statements in functions
*                       for proper documentation while generating doxygen.
* 1.3   ask    08/08/18 Changed the Can ID to 11 bit value as standard Can ID
*						is 11 bit.
* 2.8   ht     06/19/23 Added support for system device-tree flow.
* 2.8   gm     06/22/23 Call XCanFd_stop to release canfd node.
* 2.10  ht     01/09/25 Fix C++ warning.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xcanfd.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define CANFD_DEVICE_ID	XPAR_CANFD_0_DEVICE_ID
#else
#define XCANFD_BASEADDRESS XPAR_XCANFD_0_BASEADDR
#endif

/* Maximum CAN frame length in Bytes */
#define XCANFD_MAX_FRAME_SIZE_IN_BYTES 72
#define FRAME_DATA_LENGTH 		64

/* Message Id Constant */
#define TEST_MESSAGE_ID			1024

/*
 * The Baud Rate Prescaler Register (BRPR) and Bit Timing Register (BTR)
 * are setup such that CAN baud rate equals 40Kbps, assuming that the
 * the CAN clock frequency is 24MHz. The user needs to modify these values
 * based on the desired baud rate and the CAN clock frequency. For more
 * information see the CAN 2.0A, CAN 2.0B, ISO 11898-1 specifications.
 * both in DataPhase and Arbitration Phase.
 */
#define TEST_BRPR_BAUD_PRESCALAR	29

#define TEST_BTR_SYNCJUMPWIDTH		3
#define TEST_BTR_SECOND_TIMESEGMENT	2
#define TEST_BTR_FIRST_TIMESEGMENT	15

#define TEST_FBRPR_BAUD_PRESCALAR	29

#define TEST_FBTR_SYNCJUMPWIDTH		3
#define TEST_FBTR_SECOND_TIMESEGMENT	2
#define TEST_FBTR_FIRST_TIMESEGMENT	15

/* Mask for MailBox */
#define TEST_MAILBOX_MASK 0xFFFFFFFF

/* Test Message Dlc */
#define TESTMSG_DLC	15


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#ifndef SDT
int XCanFdPolledExample(u16 DeviceId);
#else
int XCanFdPolledExample(UINTPTR BaseAddress);
#endif
static int SendFrame(XCanFd  *InstancePtr);
static int RecvFrame(XCanFd  *InstancePtr);

/************************** Variable Definitions *****************************/


/*
 * Buffers to hold frames to send and receive. These are declared as global so
 * that they are not on the stack.
 * These buffers need to be 32-bit aligned
 */
static u32 TxFrame[XCANFD_MAX_FRAME_SIZE_IN_BYTES];
static u32 RxFrame[XCANFD_MAX_FRAME_SIZE_IN_BYTES];

/* Driver instance */
static XCanFd CanFd;

/*****************************************************************************/
/**
*
* Main function to call the example. This function is not included if the
* example is generated from the TestAppGen test tool.
*
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	xil_printf("XCanFd Polled Mode example\n\r");
	/*
	 * Run the Can Polled example, specify the Device ID that is generated
	 * in xparameters.h .
	 */
#ifndef SDT
	if (XCanFdPolledExample(CANFD_DEVICE_ID)) {
#else
	if (XCanFdPolledExample(XCANFD_BASEADDRESS)) {
#endif
		xil_printf("XCanFd Polled Mode example Failed\n\r");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran XCanFd Polled Mode example\n\r");
	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
*
* The entry point for showing the XCan driver in polled mode. The example
* configures the device for internal loopback mode, then sends a Can
* frame, receives the same Can frame, and verifies the frame contents.
*
* @param	DeviceId is the XPAR_CAN_<instance_num>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*
* XST_SUCCESS if successful, otherwise driver-specific error code.
*
* @note
*
* If the device is not working correctly, this function may enter an infinite
* loop and will never return to the caller.
*
******************************************************************************/
#ifndef SDT
int XCanFdPolledExample(u16 DeviceId)
#else
int XCanFdPolledExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XCanFd *CanFdInstPtr = &CanFd;
	XCanFd_Config *ConfigPtr;
	u32 IdValue;
	u32 BuffNr;
	u8 RxBuffers;

	/* Initialize the Can device */
#ifndef SDT
	ConfigPtr = XCanFd_LookupConfig(DeviceId);
#else
	ConfigPtr = XCanFd_LookupConfig(BaseAddress);
#endif
	if (CanFdInstPtr == NULL) {
		return XST_FAILURE;
	}
	Status = XCanFd_CfgInitialize(CanFdInstPtr,
				      ConfigPtr,
				      ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Run self-test on the device, which verifies basic sanity of the
	 * device and the driver.
	 */
	Status = XCanFd_SelfTest(CanFdInstPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	/*
	 * Enter Configuration Mode so we can setup Baud Rate Prescaler
	 * Register (BRPR) and Bit Timing Register (BTR)
	 */
	XCanFd_EnterMode(CanFdInstPtr, XCANFD_MODE_CONFIG);
	while (XCanFd_GetMode(CanFdInstPtr) != XCANFD_MODE_CONFIG);

	/*
	 * Configure the Baud Rate Prescalar in
	 * Arbitration Phase
	 */
	XCanFd_SetBaudRatePrescaler(CanFdInstPtr, TEST_BRPR_BAUD_PRESCALAR);

	/*
	 * Configure the Bit Timing Values in
	 * Arbitration Phase.
	 */
	XCanFd_SetBitTiming(CanFdInstPtr, TEST_BTR_SYNCJUMPWIDTH,
			    TEST_BTR_SECOND_TIMESEGMENT, TEST_BTR_FIRST_TIMESEGMENT);

	/* Configure the Baud Rate Prescalar in Data Phase */
	XCanFd_SetFBaudRatePrescaler(CanFdInstPtr, TEST_FBRPR_BAUD_PRESCALAR);

	/* Configure the Bit Timing Values in Data Phase */
	XCanFd_SetFBitTiming(CanFdInstPtr, TEST_FBTR_SYNCJUMPWIDTH,
			     TEST_FBTR_SECOND_TIMESEGMENT, TEST_FBTR_FIRST_TIMESEGMENT);

	/*
	 * Disable the Global BRS Disable so that
	 * at the time of sending the can frame
	 * we will choose whether we need Bit
	 * Rate Switch or not.
	 */
	XCanFd_SetBitRateSwitch_DisableNominal(CanFdInstPtr);
	/*
	 * Check for the design
	 * 0 - Sequential Mode
	 * 1 - MailBox Mode.
	 */
	if (XCANFD_GET_RX_MODE(CanFdInstPtr) == 0) {
		XCanFd_AcceptFilterDisable(CanFdInstPtr,
					   XCANFD_AFR_UAF_ALL_MASK);
		XCanFd_AcceptFilterEnable(CanFdInstPtr,
					  XCANFD_AFR_UAF_ALL_MASK);
	} else {
		RxBuffers = XCanFd_Get_RxBuffers(CanFdInstPtr);
		IdValue = XCanFd_CreateIdValue(TEST_MESSAGE_ID, 0, 0, 0, 0);
		for (BuffNr = 0; BuffNr < RxBuffers; BuffNr++) {
			XCanFd_RxBuff_MailBox_DeActive(CanFdInstPtr, BuffNr);
			XCanFd_Set_MailBox_IdMask(CanFdInstPtr, BuffNr,
						  TEST_MAILBOX_MASK, IdValue);
			XCanFd_RxBuff_MailBox_Active(CanFdInstPtr, BuffNr);
		}
	}

	/* Configure CAN Device to enter to Loop Back Mode */
	XCanFd_EnterMode(CanFdInstPtr, XCANFD_MODE_LOOPBACK);
	while (XCanFd_GetMode(CanFdInstPtr) != XCANFD_MODE_LOOPBACK);

	/* Send Frame */
	Status = SendFrame(CanFdInstPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Receive The Frame */
	Status = RecvFrame(CanFdInstPtr);

	/* Stop Canfd */
	XCanFd_stop(CanFdInstPtr);

	return Status;

}

/*****************************************************************************/
/**
*
* Send a CAN frame.
*
* @param	InstancePtr is a pointer to the driver instance
*
* @return	XST_SUCCESS if successful, a driver-specific return code if not.
*
* @note		This function waits until TX FIFO has room for at least one frame before
* 		sending a frame. So this function may block if the hardware is not built
* 		correctly.
*
******************************************************************************/

static int SendFrame(XCanFd *InstancePtr)
{
	int Status;
	u32 TxBufferNumber;
	u8 *FramePtr;
	u32 Index;
	u32 NofBytes;

	/* Create correct values for Identifier and Data Length Code Register */
	TxFrame[0] = XCanFd_CreateIdValue(TEST_MESSAGE_ID, 0, 0, 0, 0);
	TxFrame[1] = XCanFd_Create_CanFD_Dlc_BrsValue(TESTMSG_DLC);

	/*
	* Get the number of bytes to transmit so that we can buffer with those
	* many bytes
	*/
	NofBytes = XCanFd_GetDlc2len(TxFrame[1] & XCANFD_DLCR_DLC_MASK,
				     EDL_CANFD);

	/*
	 * Now fill in the data field with known values so we can verify them
	 * on receive.
	 */

	FramePtr = (u8 *)(&TxFrame[2]);
	for (Index = 0; Index < NofBytes; Index++) {
		*FramePtr++ = (u8)Index;
	}

	/* Now send the frame */
	Status = XCanFd_Send(InstancePtr, TxFrame, &TxBufferNumber);
	if (Status == XST_FIFO_NO_ROOM) {
		return Status;
	}

	/* Wait until Buffer is Transmitted */
	while (XCanFd_IsBufferTransmitted(InstancePtr, TxBufferNumber)
	       == FALSE);

	return Status;
}

/*****************************************************************************/
/**
*
* This function receives a frame and verifies its contents.
*
* @param	InstancePtr is a pointer to the driver instance.
*
* @return	XST_SUCCESS if successful, a driver-specific return code if not.
*
* @note	 	This function waits until RX FIFO becomes not empty before
*		reading a frame	from it. So this function may block if the
*		hardware is not built correctly.
*
******************************************************************************/
static int RecvFrame(XCanFd *InstancePtr)
{
	int Status;
	u32 Dlc;
	u8 *FramePtr;
	u32 Index;

	/* Receive a frame and verify its contents */
	if (XCANFD_GET_RX_MODE(InstancePtr) == 1) {
		Status = XCanFd_Recv_Mailbox(InstancePtr, RxFrame);
	} else {
		Status = XCanFd_Recv_Sequential(InstancePtr, RxFrame);
	}
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Get Dlc value */
	Dlc = XCanFd_GetDlc2len(RxFrame[1] & XCANFD_DLCR_DLC_MASK,
				EDL_CANFD);

	/* Verify Identifier and Data Length Code */
	if (RxFrame[0] != XCanFd_CreateIdValue(TEST_MESSAGE_ID, 0, 0, 0, 0)) {
		return XST_FAILURE;
	}
	if (TESTMSG_DLC != XCanFd_GetLen2Dlc(Dlc)) {
		return XST_FAILURE;
	}

	/* Verify Data field contents */
	FramePtr = (u8 *)(&RxFrame[2]);
	for (Index = 0; Index < Dlc; Index++) {
		if (*FramePtr++ != (u8)Index) {
			return XST_FAILURE;
		}
	}
	return Status;
}
