/******************************************************************************
* Copyright (C) 2005 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xcan_intr_example.c
*
* @brief Demonstrates the XCan driver in interrupt mode.
*
* Contains an example of how to use the XCan driver directly. The example here
* shows using the driver/device in interrupt mode.
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
* 1.00a xd/sv	01/12/09 First release
* 2.00a ktn	10/22/09 Updated to use the HAL APIs/macros.
*			 The macros have been renamed to remove _m from the
*			 name.
* 2.00a bss	01/11/11 Updated the example to be used with the SCUGIC in
*			 Zynq.
* 2.00a bss	01/16/12 Updated the example to fix CR 694533,
*			 replaced INTC_DEVID with INTC_DEVICE_ID.
* 3.2   ms  01/23/17 Added xil_printf statement in main function to
*                    ensure that "Successfully ran" and "Failed" strings are
*                    available in all examples. This is a fix for CR-965028.
* 3.3   ask  08/01/18 Fixed Cppcheck and GCC warnings in can driver
* 3.7   ht   07/04/23 Added support for system device-tree flow.
* 3.8   ht   12/13/23 Added support for ECC.
* 3.11  sp   21/07/25 Fix GCC Warnings.
* 3.13  vn   07/30/26 Fix Doxygen warnings
* 3.13  vn   09/08/26 Align Doxygen comments with AMD standards
* 3.13  vn   09/30/26 Update Doxygen comments to AMD standards
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcan.h"
#include "xstatus.h"
#include "xil_exception.h"
#include "xparameters.h"

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#include <stdio.h>
#else  /* SCU GIC */
#include "xscugic.h"
#include "xil_printf.h"
#endif
#else
#include "xinterrupt_wrap.h"
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
#define CAN_DEVICE_ID		XPAR_CAN_0_DEVICE_ID /**< Device ID. */
/** CAN interrupt vector ID. */
#define CAN_INTR_VEC_ID		XPAR_INTC_0_CAN_0_VEC_ID

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID /**< INTC device ID. */
#else
/** INTC device ID. */
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif /* XPAR_INTC_0_DEVICE_ID */
#else
#define XCAN_BASEADDRESS	XPAR_CAN_0_BASEADDR /**< CAN base address. */
#endif
/** @} */

/** Maximum CAN frame length in word. */
#define XCAN_MAX_FRAME_SIZE_IN_WORDS (XCAN_MAX_FRAME_SIZE / sizeof(u32))

#define FRAME_DATA_LENGTH	8		/**< Frame Data field length. */

/** Message Id Constant. */
#define TEST_MESSAGE_ID			1024

/*
 * The Baud Rate Prescaler Register (BRPR) and Bit Timing Register (BTR)
 * are setup such that CAN baud rate equals 40Kbps, assuming that the
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
static int XCanIntrExample(u16 DeviceId);
#else
static int XCanIntrExample(UINTPTR BaseAddress);
#endif
static void Config(XCan *InstancePtr);
static void SendFrame(XCan *InstancePtr);

static void SendHandler(void *CallBackRef);
static void RecvHandler(void *CallBackRef);
static void ErrorHandler(void *CallBackRef, u32 ErrorMask);
static void EventHandler(void *CallBackRef, u32 Mask);
#ifndef SDT
static int SetupInterruptSystem(XCan *InstancePtr);
#endif
/************************** Variable Definitions *****************************/
static XCan Can; /**< Instance of the XCan driver. */

/*
 * Buffers to hold frames to send and receive. These are declared as global so
 * that they are not on the stack.
 * These buffers need to be 32-bit aligned
 */
static u32 TxFrame[XCAN_MAX_FRAME_SIZE_IN_WORDS]; /**< Frame to send. */
static u32 RxFrame[XCAN_MAX_FRAME_SIZE_IN_WORDS]; /**< Frame received. */

/*
 * Shared variables used to test the callbacks.
 */
static volatile  int LoopbackError;	/**< Asynchronous error occurred. */
static volatile  int RecvDone;		/**< Received a frame. */
static volatile  int SendDone;		/**< Frame was sent successfully. */

/*****************************************************************************/
/**
*
* @brief Runs the CAN interrupt example.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
******************************************************************************/
int main(void)
{
	/*
	 * Run the Can interrupt example.
	 */
#ifndef SDT
	if (XCanIntrExample(CAN_DEVICE_ID)) {
#else
	if (XCanIntrExample(XCAN_BASEADDRESS)) {
#endif
		xil_printf("Can Interrupt Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Can Interrupt Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Demonstrates the XCan driver in interrupt mode.
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
static int XCanIntrExample(u16 DeviceId)
#else
static int XCanIntrExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	u32 Mask;
#ifdef SDT
	XCan_Config *ConfigPtr;

	ConfigPtr = XCan_LookupConfig(XCAN_BASEADDRESS);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
#endif
	/*
	 * Initialize the XCan driver.
	 */
#ifndef SDT
	Status = XCan_Initialize(&Can, DeviceId);
#else
	Status = XCan_Initialize(&Can, BaseAddress);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Run self-test on the device, which verifies basic sanity of the
	 * device and the driver.
	 */
	Status = XCan_SelfTest(&Can);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Configure the CAN device.
	 */
	Config(&Can);

	/*
	 * Set interrupt handlers.
	 */
	XCan_SetHandler(&Can, XCAN_HANDLER_SEND,
			(void *)SendHandler, (void *)&Can);
	XCan_SetHandler(&Can, XCAN_HANDLER_RECV,
			(void *)RecvHandler, (void *)&Can);
	XCan_SetHandler(&Can, XCAN_HANDLER_ERROR,
			(void *)ErrorHandler, (void *)&Can);
	XCan_SetHandler(&Can, XCAN_HANDLER_EVENT,
			(void *)EventHandler, (void *)&Can);

	/*
	 * Initialize flags.
	 */
	SendDone = FALSE;
	RecvDone = FALSE;
	LoopbackError = FALSE;

	/*
	 * Connect to the interrupt controller.
	 */
#ifndef SDT
	Status = SetupInterruptSystem(&Can);
#else
	Status = XSetupInterruptSystem(&Can, &XCan_IntrHandler,
				       ConfigPtr->IntrId,
				       ConfigPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable all interrupts in CAN device.
	 */
	Mask = XCAN_IXR_ALL;

	if(Can.EnableECC == 1) {
		/* Enable ECC interrupts in CAN device */
		Mask |= XCAN_IXR_ECC_MASK;
	}

	XCan_InterruptEnable(&Can, Mask);

	/*
	 * Enter Loop Back Mode.
	 */
	XCan_EnterMode(&Can, XCAN_MODE_LOOPBACK);
	while (XCan_GetMode(&Can) != XCAN_MODE_LOOPBACK);

	/*
	 * Loop back a frame. The RecvHandler is expected to handle
	 * the frame reception.
	 */
	SendFrame(&Can); /* Send a frame */

	/*
	 * Wait here until both sending and reception have been completed.
	 */
	while ((SendDone != TRUE) || (RecvDone != TRUE));

	/*
	 * Check for errors found in the callbacks.
	 */
	if (LoopbackError == TRUE) {
		return XST_LOOPBACK_ERROR;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Configures the CAN device baud rate and timing.
*
* This function configures CAN device. Baud Rate Prescaler Register (BRPR) and
* Bit Timing Register (BTR) are set in this function.
*
* @param[in]    InstancePtr Pointer to the driver instance.
*
* @note		If the CAN device is not working correctly, this function may
*               enter an infinite loop and never returns to the caller.
*
******************************************************************************/
static void Config(XCan *InstancePtr)
{
	/*
	 * Enter Configuration Mode if the device is not currently in
	 * Configuration Mode.
	 */
	XCan_EnterMode(InstancePtr, XCAN_MODE_CONFIG);
	while (XCan_GetMode(InstancePtr) != XCAN_MODE_CONFIG);

	/*
	 * Setup Baud Rate Prescaler Register (BRPR) and
	 * Bit Timing Register (BTR) such that CAN baud rate equals 40Kbps,
	 * given the CAN clock frequency equal to 24MHz.
	 */
	XCan_SetBaudRatePrescaler(InstancePtr, TEST_BRPR_BAUD_PRESCALAR);
	XCan_SetBitTiming(InstancePtr, TEST_BTR_SYNCJUMPWIDTH,
			  TEST_BTR_SECOND_TIMESEGMENT,
			  TEST_BTR_FIRST_TIMESEGMENT);
}

/*****************************************************************************/
/**
*
* @brief Sends a CAN frame.
*
* @param[in]    InstancePtr Pointer to the driver instance.
*
******************************************************************************/
static void SendFrame(XCan *InstancePtr)
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

	/*
	 * Now wait until the TX FIFO is not full and send the frame.
	 */
	while (XCan_IsTxFifoFull(InstancePtr) == TRUE);

	Status = XCan_Send(InstancePtr, TxFrame);
	if (Status != XST_SUCCESS) {
		/* The frame could not be sent successfully */
		LoopbackError = TRUE;
		SendDone = TRUE;
		RecvDone = TRUE;
	}
}


/*****************************************************************************/
/**
*
* @brief Handles transmit completion callback.
*
* Callback function (called from interrupt handler) to handle confirmation of
* transmit events when in interrupt mode.
*
* @param[in]    CallBackRef Callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
*
* @note		This function is called by the driver within interrupt context.
*
******************************************************************************/
static void SendHandler(void *CallBackRef)
{
	/*
	 * The frame was sent successfully. Notify the task context.
	 */
	SendDone = TRUE;
}


/*****************************************************************************/
/**
*
* @brief Handles frame reception callback.
*
* Callback function (called from interrupt handler) to handle frames received in
* interrupt mode. This function is called once per frame received.
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
	XCan *CanPtr = (XCan *)CallBackRef;
	int Status;
	int Index;
	u8 *FramePtr;

	Status = XCan_Recv(CanPtr, RxFrame);
	if (Status != XST_SUCCESS) {
		LoopbackError = TRUE;
		RecvDone = TRUE;
		return;
	}

	/*
	 * Verify Identifier and Data Length Code.
	 */
	if (RxFrame[0] != XCan_CreateIdValue(TEST_MESSAGE_ID, 0, 0, 0, 0)) {
		LoopbackError = TRUE;
		RecvDone = TRUE;
		return;
	}

	if (RxFrame[1] != XCan_CreateDlcValue(FRAME_DATA_LENGTH)) {
		LoopbackError = TRUE;
		RecvDone = TRUE;
		return;
	}

	/*
	* Verify Data field contents.
	*/
	FramePtr = (u8 *)(&RxFrame[2]);
	for (Index = 0; Index < FRAME_DATA_LENGTH; Index++) {
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
* @brief Handles error interrupt callback.
*
* Callback function (called from interrupt handler) to handle error interrupt.
* Error code read from Error Status register is passed into this function.
*
* @param[in]    CallBackRef Callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
* @param[in]    ErrorMask Bit mask indicating the cause of the error. Its
*               value equals the bitwise OR of one or more XCAN_ESR_* defined
*               in xcan_l.h.
*
* @note		This function is called by the driver within interrupt context.
*
******************************************************************************/
static void ErrorHandler(void *CallBackRef, u32 ErrorMask)
{

	if (ErrorMask & XCAN_ESR_ACKER_MASK) {
		/*
		 * ACK Error handling code should be put here.
		 */
	}

	if (ErrorMask & XCAN_ESR_BERR_MASK) {
		/*
		 * Bit Error handling code should be put here.
		 */
	}

	if (ErrorMask & XCAN_ESR_STER_MASK) {
		/*
		 * Stuff Error handling code should be put here.
		 */
	}

	if (ErrorMask & XCAN_ESR_FMER_MASK) {
		/*
		 * Form Error handling code should be put here.
		 */
	}

	if (ErrorMask & XCAN_ESR_CRCER_MASK) {
		/*
		 * CRC Error handling code should be put here.
		 */
	}

	/*
	 * Set the shared variables.
	 */
	LoopbackError = TRUE;
	RecvDone = TRUE;
	SendDone = TRUE;
}



/*****************************************************************************/
/**
*
* @brief Handles miscellaneous event interrupt callbacks.
*
* Callback function (called from interrupt handler) to handle the following
* interrupts:
*   - XCAN_IXR_BSOFF_MASK:  Bus Off Interrupt
*   - XCAN_IXR_RXOFLW_MASK: RX FIFO Overflow Interrupt
*   - XCAN_IXR_RXUFLW_MASK: RX FIFO Underflow Interrupt
*   - XCAN_IXR_TXBFLL_MASK: TX High Priority Buffer Full Interrupt
*   - XCAN_IXR_TXFLL_MASK:  TX FIFO Full Interrupt
*   - XCAN_IXR_WKUP_MASK:   Wake up Interrupt
*   - XCAN_IXR_SLP_MASK:    Sleep Interrupt
*   - XCAN_IXR_ARBLST_MASK: Arbitration Lost Interrupt
*
* Modify this function to meet specific application needs.
*
* @param[in]    CallBackRef Callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
* @param[in]    IntrMask Bit mask indicating pending interrupts. Its value
*               equals the bitwise OR of one or more of the XCAN_IXR_*_MASK
*               values mentioned above.
*
* @note		This function is called by the driver within interrupt context.
*
******************************************************************************/
static void EventHandler(void *CallBackRef, u32 IntrMask)
{
	XCan *CanPtr = (XCan *)CallBackRef;

	if (IntrMask & XCAN_IXR_BSOFF_MASK) { /* Enter Bus off status */
		/*
		 * Entering Bus off status interrupt requires
		 * the CAN device be reset and re-configurated.
		 */
		XCan_Reset(CanPtr);
		Config(CanPtr);
		return;
	}

	if (IntrMask & XCAN_IXR_RXOFLW_MASK) { /* RX FIFO Overflow Interrupt */
		/*
		 * Code to handle RX FIFO Overflow
		 * Interrupt should be put here.
		 */
	}

	if (IntrMask & XCAN_IXR_RXUFLW_MASK) { /* RX FIFO Underflow Interrupt */
		/*
		 * Code to handle RX FIFO Underflow
		 * Interrupt should be put here.
		 */
	}

	if (IntrMask & XCAN_IXR_TXBFLL_MASK) { /* TX High Priority Full Intr */
		/*
		 * Code to handle TX High Priority Buffer Full
		 * Interrupt should be put here.
		 */
	}

	if (IntrMask & XCAN_IXR_TXFLL_MASK) { /* TX FIFO Full Interrupt */
		/*
		 * Code to handle TX FIFO Full
		 * Interrupt should be put here.
		 */
	}

	if (IntrMask & XCAN_IXR_WKUP_MASK) { /* Wake up from sleep mode */
		/*
		 * Code to handle Wake up from sleep mode
		 * Interrupt should be put here.
		 */
	}

	if (IntrMask & XCAN_IXR_SLP_MASK) { /* Enter sleep mode */
		/*
		 * Code to handle Enter sleep mode
		 * Interrupt should be put here.
		 */
	}

	if (IntrMask & XCAN_IXR_ARBLST_MASK) { /* Lost bus arbitration */

		/*
		 * Code to handle Lost bus arbitration
		 * Interrupt should be put here.
		 */
	}

	if(CanPtr->EnableECC == 1) {
		if (IntrMask & XCAN_IXR_ECC_MASK) { /* ECC error */
			/*
			 * Code to ECC errors
			 * Interrupt should be put here.
			 */
		}
	}
}

#ifndef SDT
/*****************************************************************************/
/**
*
* @brief Sets up the interrupt system for CAN.
*
* This function sets up the interrupt system so interrupts can occur for the
* CAN. This function is application-specific since the actual system may or
* may not have an interrupt controller. The CAN could be directly connected
* to a processor without an interrupt controller. Modify this function to
* fit the application.
*
* @param[in]    InstancePtr Pointer to the instance of the CAN
*		which is going to be connected to the interrupt controller.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
******************************************************************************/
static int SetupInterruptSystem(XCan *InstancePtr)
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
	 * Connect the device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the specific
	 * interrupt processing for the device.
	 */
	Status = XIntc_Connect(&InterruptController,
			       CAN_INTR_VEC_ID,
			       (XInterruptHandler)XCan_IntrHandler,
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

	/*
	 * Enable the interrupt for the CAN.
	 */
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
				 (Xil_ExceptionHandler)XCan_IntrHandler,
				 InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Can device.
	 */
	XScuGic_Enable(&InterruptController, CAN_INTR_VEC_ID);




#endif

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)INTC_HANDLER,
				     &InterruptController);

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif
