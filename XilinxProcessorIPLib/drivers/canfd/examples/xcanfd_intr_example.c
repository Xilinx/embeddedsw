/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xcanfd_intr_example.c
*
* @brief Demonstrates using the XCanFd driver and device directly in interrupt
* mode.
*
* @note
* This code assumes that the Xilinx interrupt controller (XIntc) forwards the
* CAN device interrupt output to the processor. It also assumes that no
* operating system is used.
* <br><br>
* The Baud Rate Prescaler Register (BRPR) and Bit Timing Register (BTR) are set
* up for a CAN baud rate of 40Kbps. This setting assumes a CAN clock of 24MHz.
* Modify these values based on the desired baud rate and the CAN clock
* frequency. For more information see the CAN 2.0A, CAN 2.0B, ISO 11898-1
* specifications.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who	Date	 Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   nsk 06/04/15 First release
* 1.2   ms  01/23/17 Modified xil_printf statement in main function to
*                    ensure that "Successfully ran" and "Failed" strings are
*                    available in all examples. This is a fix for CR-965028.
*       ms  04/05/17 Added tabspace for return statements in functions
*                    for proper documentation while generating doxygen.
* 2.1   ask 07/03/18 Added timeout, and Code to handle Timestamp Counter
*		     Overflow Interrupt for CR# 992606, CR# 1004222 and
*		     fixed gcc warnings.
* 		ask    08/08/18 Changed the Can ID to 11 bit value as standard Can ID
*                                              is 11 bit.
*		ask    09/12/18 Added timeout, and Code to handle Timestamp Counter
*                              Overflow Interrupt. Made logic generic for both
*                                              scugic and intc vectors.
* 2.1	nsk  03/09/19 Fix build error in example
* 2.3	sne  07/11/19 Added Protocol Exception support in EventHandler and updated
*		      Bus Off EventHandler.
* 2.8	ht   06/19/23 Added support for system device-tree flow
* 2.10	ht   12/01/24 Fixed GCC warnings
* 2.12  vlt  01/27/26 Fixed codespell issues
* 2.13  vn   07/30/26 Fix Doxygen warnings
* 2.13  vn   09/08/26 Align Doxygen comments with AMD standards
* 2.13  vn   09/30/26 Update Doxygen comments to AMD standards
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcanfd.h"
#include "xstatus.h"
#include "xil_exception.h"
#include "sleep.h"
#include "xparameters.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#include <stdio.h>
#else  /* SCU GIC */
#include "xscugic.h"
#include "xil_printf.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
/** @name XPAR parameter mappings
 * @{
 */
#ifndef SDT
/** Device ID. */
#define CANFD_DEVICE_ID		XPAR_CANFD_0_DEVICE_ID
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID /**< INTC device ID. */
/** CAN interrupt vector ID. */
#define CAN_INTR_VEC_ID	XPAR_INTC_0_CANFD_0_VEC_ID
#else
/** INTC device ID. */
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#ifdef XPAR_CANFD_ISPS
/** CAN interrupt vector ID. */
#define CAN_INTR_VEC_ID	XPAR_XCANPS_0_INTR
#else
/** CAN interrupt vector ID. */
#define CAN_INTR_VEC_ID	XPAR_FABRIC_CANFD_0_VEC_ID
#endif
#endif /* XPAR_INTC_0_DEVICE_ID */
/** @} */
#else
#define XCANFD_BASEADDRESS XPAR_XCANFD_0_BASEADDR /**< CAN FD base address. */
#endif

/** Maximum CAN frame length in Bytes. */
#define XCANFD_MAX_FRAME_SIZE_IN_BYTES 72

#define FRAME_DATA_LENGTH	64 /**< Frame Data field length. */

#define TEST_MESSAGE_ID		1024 /**< Message Id Constant. */

#define TEST_CANFD_DLC 15			       /**< Data length code. */

#define TEST_MAILBOX_MASK 0xFFFFFFFF		       /**< Mail Box Mask. */

/*
 * The Baud Rate Prescaler Register (BRPR) and Bit Timing Register (BTR)
 * are setup such that CANFD baud rate equals 40Kbps, assuming that the
 * the CAN clock is 24MHz. The user needs to modify these values based on
 * the desired baud rate and the CAN clock frequency. For more information
 * see the CAN 2.0A, CAN 2.0B, ISO 11898-1 specifications.
 */
/** @name Baud Rate Prescaler and Bit Timing Register values
 * @{
 */
/** Baud Rate Prescaler Register (BRPR) value. */
#define TEST_BRPR_BAUD_PRESCALAR	29

/** Bit Timing Register (BTR) synchronization jump width. */
#define TEST_BTR_SYNCJUMPWIDTH		3
/** Bit Timing Register (BTR) second time segment. */
#define TEST_BTR_SECOND_TIMESEGMENT	2
/** Bit Timing Register (BTR) first time segment. */
#define TEST_BTR_FIRST_TIMESEGMENT	15
/** @} */

/** @name Fast Baud Rate Prescaler and Fast Bit Timing Register values
 * @{
 */
/** Fast Baud Rate Prescaler Register (F_BRPR) value. */
#define TEST_FBRPR_BAUD_PRESCALAR	29

/** Fast Bit Timing Register (F_BTR) synchronization jump width. */
#define TEST_FBTR_SYNCJUMPWIDTH		3
/** Fast Bit Timing Register (F_BTR) second time segment. */
#define TEST_FBTR_SECOND_TIMESEGMENT	2
/** Fast Bit Timing Register (F_BTR) first time segment. */
#define TEST_FBTR_FIRST_TIMESEGMENT	15
/** @} */
/** @name Timeout values
 * @{
 */
#define MAX_TIMEOUT 			100 /**< Maximum timeout count. */
/** Maximum sleep time in microseconds. */
#define MAX_USLEEP_VAL                  10*1000
/** @} */

/** @name Interrupt controller definitions
 * @{
 */
#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc /**< Interrupt controller type. */
#define INTC_HANDLER	XIntc_InterruptHandler /**< Interrupt handler. */
#else
#define INTC		XScuGic /**< Interrupt controller type. */
#define INTC_HANDLER	XScuGic_InterruptHandler /**< Interrupt handler. */
#endif /* XPAR_INTC_0_DEVICE_ID */
#endif
/** @} */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#ifndef SDT
static int XCanFdIntrExample(u16 DeviceId);
static int SetupInterruptSystem(XCanFd  *InstancePtr);
#else
static int XCanFdIntrExample(UINTPTR BaseAddress);
#endif
static void Config(XCanFd  *InstancePtr);
static int SendFrame(XCanFd  *InstancePtr);

static void SendHandler(void *CallBackRef);
static void RecvHandler(void *CallBackRef);
static void ErrorHandler(void *CallBackRef, u32 ErrorMask);
static void EventHandler(void *CallBackRef, u32 Mask);


/************************** Variable Definitions *****************************/
static XCanFd  CanFd; /**< Instance of the XCanFd driver. */

/*
 * Buffers to hold frames to send and receive. These are declared as global so
 * that they are not on the stack.
 * These buffers need to be 32-bit aligned
 */
static u32 TxFrame[XCANFD_MAX_FRAME_SIZE_IN_BYTES]; /**< Frame to send. */
static u32 RxFrame[XCANFD_MAX_FRAME_SIZE_IN_BYTES]; /**< Frame received. */

static volatile int LoopbackError;	/**< Asynchronous error occurred. */
static volatile int RecvDone;		/**< Received a frame. */
static volatile int SendDone;		/**< Frame was sent successfully. */

/*****************************************************************************/
/**
*
* @brief Runs the CAN FD interrupt-mode example as the application entry point.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
*****************************************************************************/
int main(void)
{
	/* Run the Can interrupt example */
#ifndef SDT
	if (XCanFdIntrExample(CANFD_DEVICE_ID)) {
#else
	if (XCanFdIntrExample(XCANFD_BASEADDRESS)) {
#endif
		xil_printf("XCanFd Interrupt Mode example Failed\n\r");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran XCanFd Interrupt Mode example\n\r");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Shows the XCanFd driver in interrupt mode as the main entry point.
*
* The example configures the device for internal loopback mode, then
* sends a CAN frame and receives the same CAN frame.
*
* @if SDT
* @param[in]    BaseAddress CAN device base address.
* @else
* @param[in]    DeviceId CAN device ID.
* @endif
*
* @return	XST_SUCCESS if successful, otherwise driver-specific error code.
*
* @note 	If the device is not working correctly, this function may enter
*               an infinite loop and never returns to the caller.
*
******************************************************************************/
#ifndef SDT
static int XCanFdIntrExample(u16 DeviceId)
#else
static int XCanFdIntrExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	int TimeOut;
	XCanFd *CanFdInstPtr = &CanFd;
	XCanFd_Config *ConfigPtr;

	/* Initialize the XCanFd driver */
#ifndef SDT
	ConfigPtr = XCanFd_LookupConfig(DeviceId);
#else
	ConfigPtr = XCanFd_LookupConfig(BaseAddress);
#endif
	if (CanFdInstPtr == NULL) {
		return XST_FAILURE;
	}
	Status = XCanFd_CfgInitialize(CanFdInstPtr,
				      ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Run self-test on the device, which verifies basic sanity of the
	 * device and the driver.
	 */
	Status = XCanFd_SelfTest(CanFdInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Configure the CAN device */
	Config(CanFdInstPtr);

	XCanFd_SetHandler(CanFdInstPtr, XCANFD_HANDLER_SEND,
			  (void *)SendHandler, (void *)CanFdInstPtr);
	XCanFd_SetHandler(CanFdInstPtr, XCANFD_HANDLER_RECV,
			  (void *)RecvHandler, (void *)CanFdInstPtr);
	XCanFd_SetHandler(CanFdInstPtr, XCANFD_HANDLER_ERROR,
			  (void *)ErrorHandler, (void *)CanFdInstPtr);
	XCanFd_SetHandler(CanFdInstPtr, XCANFD_HANDLER_EVENT,
			  (void *)EventHandler, (void *)CanFdInstPtr);

	/* Initialize flags */
	SendDone = FALSE;
	RecvDone = FALSE;
	LoopbackError = FALSE;

	/* Connect to the interrupt controller */
#ifndef SDT
	Status = SetupInterruptSystem(CanFdInstPtr);
#else
	Status = XSetupInterruptSystem(CanFdInstPtr, (void *)&XCanFd_IntrHandler,
				       ConfigPtr->IntrId,
				       ConfigPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Enable all interrupts in CAN device */
	XCanFd_InterruptEnable(CanFdInstPtr, XCANFD_IXR_ALL);

	/* Set the CAN Mode to LOOP Back */
	XCanFd_EnterMode(CanFdInstPtr, XCANFD_MODE_LOOPBACK);
	while (XCanFd_GetMode(CanFdInstPtr) != XCANFD_MODE_LOOPBACK);

	Status = SendFrame(CanFdInstPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*Wait for a second*/
	TimeOut = MAX_TIMEOUT;

	while ((SendDone != TRUE) || (RecvDone != TRUE)) {

		if (TimeOut) {
			usleep(MAX_USLEEP_VAL);
		} else {
			break;
		}

		TimeOut--;
	}
	if (!TimeOut) {
		if (SendDone != TRUE) {
			return XST_SEND_ERROR;
		} else {
			return XST_RECV_ERROR;
		}
	}
	/* Check for errors found in the callbacks */
	if (LoopbackError == TRUE) {
		return XST_LOOPBACK_ERROR;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Configures the CAN FD device for the interrupt-mode example.
*
* @param[in]    InstancePtr Pointer to the driver instance.
*
* @note		If the CAN device is not working correctly, this function may
*               enter an infinite loop and never returns to the caller.
*		basically this configures BRPR,BTR in both Arbitration and
*               Data phase. also configures Acceptance filters(in
*               SequentialMode) or RxBuffers(in Mailbox).
*
******************************************************************************/
static void Config(XCanFd *InstancePtr)
{
	u32 RxBuffer;
	u32 IdValue;
	u8 RxBuffers;

	XCanFd_EnterMode(InstancePtr, XCANFD_MODE_CONFIG);
	while (XCanFd_GetMode(InstancePtr) != XCANFD_MODE_CONFIG);

	/* Configure the Baud Rate Prescalar in Arbitration Phase */
	XCanFd_SetBaudRatePrescaler(InstancePtr, TEST_BRPR_BAUD_PRESCALAR);

	/* Configure the Bit Timing Values in Arbitration Phase */
	XCanFd_SetBitTiming(InstancePtr, TEST_BTR_SYNCJUMPWIDTH, \
			    TEST_BTR_SECOND_TIMESEGMENT, TEST_BTR_FIRST_TIMESEGMENT);

	/* Configure the Baud Rate Prescalar in Data Phase */
	XCanFd_SetFBaudRatePrescaler(InstancePtr, TEST_FBRPR_BAUD_PRESCALAR);

	/* Configure the Bit Timing Values in Data Phase */
	XCanFd_SetFBitTiming(InstancePtr, TEST_FBTR_SYNCJUMPWIDTH, \
			     TEST_FBTR_SECOND_TIMESEGMENT, TEST_FBTR_FIRST_TIMESEGMENT);

	/*
	 * Disable the Global BRS Disable so that at the time of sending the can
	 * frame we will choose whether we need Bit Rate Switch or not.
	 */
	XCanFd_SetBitRateSwitch_DisableNominal(InstancePtr);

	/* Configure Acceptance Filter/Mail box depends on design */
	if (XCANFD_GET_RX_MODE(InstancePtr) == 0) {
		XCanFd_AcceptFilterDisable(InstancePtr, XCANFD_AFR_UAF_ALL_MASK);
		XCanFd_AcceptFilterEnable(InstancePtr, XCANFD_AFR_UAF_ALL_MASK);
	} else {
		RxBuffers = XCanFd_Get_RxBuffers(InstancePtr);
		IdValue = XCanFd_CreateIdValue(TEST_MESSAGE_ID, 0, 0, 0, 0);
		for (RxBuffer = 0; RxBuffer < RxBuffers; RxBuffer++) {
			XCanFd_RxBuff_MailBox_DeActive(InstancePtr, RxBuffer);
			XCanFd_Set_MailBox_IdMask(InstancePtr, RxBuffer, \
						  TEST_MAILBOX_MASK, IdValue);
			XCanFd_RxBuff_MailBox_Active(InstancePtr, RxBuffer);
		}
	}
	/* Configure the CAN Device to Enter Loop Back Mode */
	XCanFd_EnterMode(InstancePtr, XCANFD_MODE_LOOPBACK);
	while (XCanFd_GetMode(InstancePtr) != XCANFD_MODE_LOOPBACK);
}

/*****************************************************************************/
/**
*
* @brief Sends a CAN frame.
*
* @param[in,out] InstancePtr Pointer to the driver instance.
*
* @return        XST_SUCCESS if successful, a driver-specific return code if
*                not.
*
******************************************************************************/
static int SendFrame(XCanFd *InstancePtr)
{
	int Status;
	u32 TxBufferNumber;
	u8 *FramePtr;
	u32 Index;
	u32 NofBytes;

	/*
	 * Create correct values for Identifier and Data Length Code Register.
	 * Here Data Length Code value is 8
	 * but CAN FD Can support up to DLC 15(64Bytes).
	 */
	TxFrame[0] = XCanFd_CreateIdValue(TEST_MESSAGE_ID, 0, 0, 0, 0);
	TxFrame[1] = XCanFd_Create_CanFD_Dlc_BrsValue(TEST_CANFD_DLC);
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

	if (Status != XST_SUCCESS) {
		/* The frame could not be sent successfully */
		LoopbackError = TRUE;
		SendDone = TRUE;
		RecvDone = TRUE;
	}

	return Status;
}


/*****************************************************************************/
/**
*
* @brief Handles confirmation of transmit events when in interrupt mode.
*
* This callback function is called from the interrupt handler.
*
* @param[in]    CallBackRef Callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
*
* @note		This function is called by the driver within interrupt context.
*
******************************************************************************/
static void SendHandler(void *CallBackRef)
{
	Xil_AssertVoid(CallBackRef != NULL);
	/* The frame was sent successfully. Notify the task context */
	SendDone = TRUE;
}


/*****************************************************************************/
/**
*
* @brief Handles frames received in interrupt mode.
*
* This callback function is called from the interrupt handler once per frame
* received.
* The driver's receive function is called to read the frame from RX FIFO.
*
* @param[in]    CallBackRef Callback reference passed from the interrupt
*		handler, which in our case is a pointer to the device instance.
*
* @note		This function is called by the driver within interrupt context.
*
******************************************************************************/
static void RecvHandler(void *CallBackRef)
{
	XCanFd *CanPtr = (XCanFd *)CallBackRef;
	int Status;
	u32 Index;
	u8 *FramePtr;
	u32 Dlc;

	/* Check for the design 1 - MailBox 0 - Sequential */
	if (XCANFD_GET_RX_MODE(CanPtr) == 1) {
		Status = XCanFd_Recv_Mailbox(CanPtr, RxFrame);
	} else {
		Status = XCanFd_Recv_Sequential(CanPtr, RxFrame);
	}

	/* Get the Dlc in the form of bytes */
	Dlc = XCanFd_GetDlc2len(RxFrame[1] & XCANFD_DLCR_DLC_MASK,
				EDL_CANFD);
	if (Status != XST_SUCCESS) {
		LoopbackError = TRUE;
		RecvDone = TRUE;
		return;
	}

	/* Verify Identifier and Data Length Code */
	if (RxFrame[0] != XCanFd_CreateIdValue(TEST_MESSAGE_ID, 0, 0, 0, 0)) {
		LoopbackError = TRUE;
		RecvDone = TRUE;
		return;
	}

	if (TEST_CANFD_DLC != XCanFd_GetLen2Dlc(Dlc)) {
		LoopbackError = TRUE;
		RecvDone = TRUE;
		return;
	}

	/* Verify Data field contents */
	FramePtr = (u8 *)(&RxFrame[2]);
	for (Index = 0; Index < Dlc; Index++) {
		if (*FramePtr++ != (u8)Index) {
			LoopbackError = TRUE;
			break;
		}
	}
	RecvDone = TRUE;
}


/*****************************************************************************/
/**
*
* @brief Handles the error interrupt.
*
* This callback function is called from the interrupt handler. Error code read
* from Error Status register is passed into this function.
*
* @param[in]    CallBackRef Callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
* @param[in]    ErrorMask Bit mask indicating the cause of the error. Its
*               value equals the bitwise OR of one or more XCANFD_ESR_*
*               defined in xcanfd_l.h.
*
* @note		This function is called by the driver within interrupt context.
*
******************************************************************************/
static void ErrorHandler(void *CallBackRef, u32 ErrorMask)
{
	XCanFd *CanPtr = (XCanFd *)CallBackRef;
	Xil_AssertVoid(CanPtr != NULL);

	if (ErrorMask & XCANFD_ESR_ACKER_MASK) {

		/* ACK Error handling code should be put here */

	}

	if (ErrorMask & XCANFD_ESR_BERR_MASK) {

		/* Bit Error handling code should be put here */
	}

	if (ErrorMask & XCANFD_ESR_STER_MASK) {

		/* Stuff Error handling code should be put here */
	}

	if (ErrorMask & XCANFD_ESR_FMER_MASK) {

		/* Form Error handling code should be put here */
	}

	if (ErrorMask & XCANFD_ESR_CRCER_MASK) {

		/* CRC Error handling code should be put here */
	}

	if (ErrorMask & XCANFD_ESR_F_BERR_MASK) {

		/* Bit Error handling code should be put here */
	}

	if (ErrorMask & XCANFD_ESR_F_STER_MASK) {

		/* Stuff Error handling code should be put here */
	}

	if (ErrorMask & XCANFD_ESR_F_FMER_MASK) {

		/* Form Error handling code should be put here */
	}

	if (ErrorMask & XCANFD_ESR_CRCER_MASK) {

		/* CRC Error handling code should be put here */
	}

	/* Set the shared variables */
	LoopbackError = TRUE;
	RecvDone = TRUE;
	SendDone = TRUE;
}



/*****************************************************************************/
/**
*
* @brief Handles the event interrupts.
*
* This callback function is called from the interrupt handler to handle the
* following interrupts:
*   - XCANFD_IXR_BSOFF_MASK:  Bus Off Interrupt
*   - XCANFD_IXR_RXOFLW_MASK: RX FIFO Overflow Interrupt
*   - XCANFD_IXR_RXUFLW_MASK: RX FIFO Underflow Interrupt
*   - XCANFD_IXR_TXBFLL_MASK: TX High Priority Buffer Full Interrupt
*   - XCANFD_IXR_TXFLL_MASK:  TX FIFO Full Interrupt
*   - XCANFD_IXR_WKUP_MASK:   Wake up Interrupt
*   - XCANFD_IXR_SLP_MASK:    Sleep Interrupt
*   - XCANFD_IXR_ARBLST_MASK: Arbitration Lost Interrupt
*   - XCANFD_IXR_TSCNT_OFLW_MASK : Timestamp Counter Overflow Interrupt
*
* Change this function to meet specific application needs.
*
* @param[in]    CallBackRef Callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
* @param[in]    IntrMask Bit mask indicating pending interrupts. Its value
*               equals the bitwise OR of one or more of the XCANFD_IXR_*_MASK
*               values mentioned above.
*
* @note		This function is called by the driver within interrupt context.
*
******************************************************************************/
static void EventHandler(void *CallBackRef, u32 IntrMask)
{
	XCanFd *CanPtr = (XCanFd *)CallBackRef;

	if (IntrMask & XCANFD_IXR_BSOFF_MASK) {
		/*
		 * The CAN device requires 128 * 11 consecutive recessive bits
		 * to recover from bus off.
		 */
		XCanFd_Pee_BusOff_Handler(CanPtr);
		return;
	}

	if (IntrMask & XCANFD_IXR_PEE_MASK) {
		XCanFd_Pee_BusOff_Handler(CanPtr);
		return;
	}

	if (IntrMask & XCANFD_IXR_RXFOFLW_MASK) {
		/*
		 * Code to handle RX FIFO Overflow Interrupt should be put here
		 */
	}
	if (IntrMask & XCANFD_IXR_RXMNF_MASK) {
		/*
		 * Code to handle RX Match Not Finished Interrupt should be put
		 *   here
		 */
	}
	if (IntrMask & XCANFD_IXR_RXBOFLW_MASK) {
		/*
		 * Code to handle RX Buffer Overflow Interrupt should be put
		 * here
		 */
	}
	if (IntrMask & XCANFD_IXR_RXRBF_MASK) {
		/*
		 * Code to handle RX Buffer Full Interrupt should be put here
		 */
	}
	if (IntrMask & XCANFD_IXR_TXCRS_MASK) {
		/*
		 * Code to handle Tx Cancellation Request Served Interrupt
		 * should be put here.
		 */
	}
	if (IntrMask & XCANFD_IXR_TXRRS_MASK) {
		/*
		 * Code to handle Tx Ready Request Served Interrupt should be
		 * put here
		 */
	}
	if (IntrMask & XCANFD_IXR_WKUP_MASK) {
		/*
		 * Code to handle Wake up from sleep mode Interrupt should be
		 *  put here.
		 */
	}

	if (IntrMask & XCANFD_IXR_SLP_MASK) {
		/*
		 * Code to handle Enter sleep mode Interrupt should be put here
		 */
	}

	if (IntrMask & XCANFD_IXR_ARBLST_MASK) {
		/*
		 * Code to handle Lost bus arbitration Interrupt should be put
		 *  here.
		 */
	}

	if (IntrMask & XCANFD_IXR_TSCNT_OFLW_MASK) {
		/*
		 * Code to handle Timestamp Counter Overflow Interrupt should be put
		 *  here.
		 */
	}

}

#ifndef SDT
/*****************************************************************************/
/**
*
* @brief Sets up the interrupt system so interrupts can occur for the CAN.
*
* This function is application-specific since the actual system may or
* may not have an interrupt controller. The CAN could be directly connected
* to a processor without an interrupt controller. Modify this function to fit
* the application.
*
* @param[in]    InstancePtr Pointer to the instance of the CAN
*		which is going to be connected to the interrupt controller.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
****************************************************************************/
static int SetupInterruptSystem(XCanFd *InstancePtr)
{
	static INTC InterruptController;
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID
	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 * INTC_DEVICE_ID specifies the XINTC device ID that is generated in
	 * xparameters.h.
	 */
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler performs
	 * the specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(&InterruptController,
			       CAN_INTR_VEC_ID,
			       (XInterruptHandler)XCanFd_IntrHandler,
			       InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller so interrupts are enabled for all
	 * devices that cause interrupts. Specify real mode so that the CAN
	 * can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Enable the interrupt for the CAN. */
	XIntc_Enable(&InterruptController, CAN_INTR_VEC_ID);
#else /* SCUGIC */


	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	XScuGic_SetPriorityTriggerType(&InterruptController, CAN_INTR_VEC_ID,
				       0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(&InterruptController, CAN_INTR_VEC_ID,
				 (Xil_ExceptionHandler)XCanFd_IntrHandler,
				 InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Enable the interrupt for the Can device */
	XScuGic_Enable(&InterruptController, CAN_INTR_VEC_ID);

#endif

	/* Initialize the exception table */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)INTC_HANDLER,
				     &InterruptController);

	/* Enable exceptions */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif
