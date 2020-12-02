/******************************************************************************
* Copyright (C) 2005 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xcan_polled_example.c
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
* 1.00a xd     04/12/05 First release
* 2.00a ktn    10/22/09 Updated driver to use the HAL APIs/macros.
*		        The macros have been renamed to remove _m from the name.
* 3.2   ms     01/23/17 Added xil_printf statement in main function to
*               ensure that "Successfully ran" and "Failed" strings are
*               available in all examples. This is a fix for CR-965028.
* 3.3   ask  08/01/18 Fixed Cppcheck and GCC warnings in can driver
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcan.h"
#include "xparameters.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define CAN_DEVICE_ID	XPAR_CAN_0_DEVICE_ID

/*
 * Maximum CAN frame length in words.
 */
#define XCAN_MAX_FRAME_SIZE_IN_WORDS (XCAN_MAX_FRAME_SIZE / sizeof(u32))

#define FRAME_DATA_LENGTH 		8  /* Frame Data field length */

/*
 * Message Id Constant.
 */
#define TEST_MESSAGE_ID			1024

/*
 * The Baud Rate Prescaler Register (BRPR) and Bit Timing Register (BTR)
 * are setup such that CAN baud rate equals 40Kbps, assuming that the
 * the CAN clock frequency is 24MHz. The user needs to modify these values
 * based on the desired baud rate and the CAN clock frequency. For more
 * information see the CAN 2.0A, CAN 2.0B, ISO 11898-1 specifications.
 */
#define TEST_BRPR_BAUD_PRESCALAR	29

#define TEST_BTR_SYNCJUMPWIDTH		3
#define TEST_BTR_SECOND_TIMESEGMENT	2
#define TEST_BTR_FIRST_TIMESEGMENT	15

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int XCanPolledExample(u16 DeviceId);
static int SendFrame(XCan *InstancePtr);
static int RecvFrame(XCan *InstancePtr);

/************************** Variable Definitions *****************************/


/*
 * Buffers to hold frames to send and receive. These are declared as global so
 * that they are not on the stack.
 * These buffers need to be 32-bit aligned
 */
static u32 TxFrame[XCAN_MAX_FRAME_SIZE_IN_WORDS];
static u32 RxFrame[XCAN_MAX_FRAME_SIZE_IN_WORDS];

/* Driver instance */
static XCan Can;

/*****************************************************************************/
/**
*
* Main function to call the example. This function is not included if the
* example is generated from the TestAppGen test tool.
*
* @param	None
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
	/*
	 * Run the Can Polled example, specify the Device ID that is generated
	 * in xparameters.h .
	 */
	if (XCanPolledExample(CAN_DEVICE_ID)) {
		xil_printf("Can polled Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Can polled Example\r\n");
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
int XCanPolledExample(u16 DeviceId)
{
	int Status;

	/*
	 * Initialize the XCan driver.
	 */
	Status = XCan_Initialize(&Can, DeviceId);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Run self-test on the device, which verifies basic sanity of the
	 * device and the driver.
	 */
	Status = XCan_SelfTest(&Can);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enter Configuration Mode so we can setup Baud Rate Prescaler
	 * Register (BRPR) and Bit Timing Register (BTR)
	 */
	XCan_EnterMode(&Can, XCAN_MODE_CONFIG);
	while(XCan_GetMode(&Can) != XCAN_MODE_CONFIG);

	/*
	 * Setup Baud Rate Prescaler Register (BRPR) and Bit Timing Register
	 * (BTR) such that CAN baud rate equals 40Kbps, given the CAN clock
	 * frequency equal to 24MHz.
	 */
	XCan_SetBaudRatePrescaler(&Can, TEST_BRPR_BAUD_PRESCALAR);
	XCan_SetBitTiming(&Can, TEST_BTR_SYNCJUMPWIDTH,
				TEST_BTR_SECOND_TIMESEGMENT,
				TEST_BTR_FIRST_TIMESEGMENT);

	/*
	 * Enter Loop Back Mode.
	 */
	XCan_EnterMode(&Can, XCAN_MODE_LOOPBACK);
	while(XCan_GetMode(&Can) != XCAN_MODE_LOOPBACK);

	/*
	 * Send a frame, receive the frame via the loopback and verify its
	 * contents.
	 */
	Status = SendFrame(&Can);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = RecvFrame(&Can);
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
* @note
*
* This function waits until TX FIFO has room for at least one frame before
* sending a frame. So this function may block if the hardware is not built
* correctly.
*
******************************************************************************/
static int SendFrame(XCan *InstancePtr)
{
	u8 *FramePtr;
	int Index;
	int Status;

	/*
	 * Create correct values for Identifier and Data Length Code Register.
	 */
	TxFrame[0] = XCan_CreateIdValue(TEST_MESSAGE_ID, 0, 0, 0, 0);
	TxFrame[1] = XCan_CreateDlcValue(FRAME_DATA_LENGTH);

	/*
	 * Now fill in the data field with known values so we can verify them
	 * on receive.
	 */
	FramePtr = (u8 *)(&TxFrame[2]);
	for (Index = 0; Index < FRAME_DATA_LENGTH; Index++) {
		*FramePtr++ = (u8)Index;
	}

	/* Wait until TX FIFO has room */
	while (XCan_IsTxFifoFull(InstancePtr) == TRUE);

	/*
	 * Now send the frame.
	 *
	 * Another way to send a frame is keep calling XCan_Send() until it
	 * returns XST_SUCCESS. No check on if TX FIFO is full is needed anymore
	 * in that case.
	 */
	Status = XCan_Send(InstancePtr, TxFrame);

	return Status;
}


/*****************************************************************************/
/**
*
* This function receives a frame and verifies its contents.
*
* @param	InstancePtr is a pointer to the driver instance
*
* @return	XST_SUCCESS if successful, a driver-specific return code if not.
*
* @note
*
* This function waits until RX FIFO becomes not empty before reading a frame
* from it. So this function may block if the hardware is not built
* correctly.
*
******************************************************************************/
static int RecvFrame(XCan *InstancePtr)
{
	u8 *FramePtr;
	int Status;
	int Index;

	/*
	 * Wait until a frame is received.
	 */
	while (XCan_IsRxEmpty(InstancePtr) == TRUE);

	/*
	 * Receive a frame and verify its contents.
	 */
	Status = XCan_Recv(InstancePtr, RxFrame);
	if (Status == XST_SUCCESS) {

		/*
		 * Verify Identifier and Data Length Code.
		 */
		if (RxFrame[0] !=
				XCan_CreateIdValue(TEST_MESSAGE_ID, 0, 0, 0, 0))
			return XST_LOOPBACK_ERROR;

		if (RxFrame[1] != XCan_CreateDlcValue(FRAME_DATA_LENGTH))
			return XST_LOOPBACK_ERROR;

		/*
		 * Verify Data field contents.
		 */
		FramePtr = (u8 *)(&RxFrame[2]);
		for (Index = 0; Index < FRAME_DATA_LENGTH; Index++) {
			if (*FramePtr++ != (u8)Index) {
				return XST_LOOPBACK_ERROR;
			}
		}
	}

	return Status;
}
