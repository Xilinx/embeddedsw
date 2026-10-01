/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xcanps_intr_example.c
*
* @brief Demonstrates XCanPs driver usage in interrupt mode.
*
* @note
* The Baud Rate Prescaler Register (BRPR) and Bit Timing Register (BTR) are set
* up for a CAN baud rate of 40Kbps. This setting assumes a CAN clock of 24MHz.
* Modify these values based on the desired baud rate and the CAN clock
* frequency. For more information see the CAN 2.0A, CAN 2.0B, ISO 11898-1
* specifications.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.00a xd/sv  01/12/10 First release
* 2.1 adk 		23/08/14 Fixed CR:798792 Peripheral test for CANPS IP in
*						 SDK claims a 40kbps baud rate but it's not.
* 3.1   mus    01/14/16 Added support for intc interrupt controller
* 3.7   ht     06/28/23 Added support for system device-tree flow.
*              07/10/23 Updated conditional macros for interrupt headers.
* 3.8   rma    01/12/23 Update example code to fix compilation warnings.
* 3.10  ht     12/13/24 Fix C++ compilation errors and warnings in SDT flow.
* 3.12  vlt    12/17/25 Update Doxygen comments to include SDT flow details.
* 3.13  vn   09/08/26 Align Doxygen comments with AMD standards
* 3.13  vn   09/30/26 Update Doxygen comments to AMD standards
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xcanps.h"
#include "xil_exception.h"
#include "xil_printf.h"

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
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
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc /**< Interrupt controller type. */
#define CAN_DEVICE_ID		XPAR_XCANPS_0_DEVICE_ID /**< Device ID. */
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID /**< INTC device ID. */
/** CAN interrupt vector ID. */
#define CAN_INTR_VEC_ID		XPAR_INTC_0_CANPS_0_VEC_ID
#else
#define INTC		XScuGic /**< Interrupt controller type. */
#define CAN_DEVICE_ID		XPAR_XCANPS_0_DEVICE_ID /**< Device ID. */
/** INTC device ID. */
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
/** CAN interrupt vector ID. */
#define CAN_INTR_VEC_ID		XPAR_XCANPS_0_INTR
#endif
#endif
/** @} */
/** Maximum CAN frame length in word. */
#define XCANPS_MAX_FRAME_SIZE_IN_WORDS (XCANPS_MAX_FRAME_SIZE / sizeof(u32))

#define FRAME_DATA_LENGTH	8	/**< Frame Data field length. */

#define TEST_MESSAGE_ID		2000	/**< Message Id Constant. */

/*
 * The Baud Rate Prescaler Register (BRPR) and Bit Timing Register (BTR)
 * are setup such that CAN baud rate equals 40Kbps, assuming that the
 * the CAN clock is 24MHz. The user needs to modify these values based on
 * the desired baud rate and the CAN clock frequency. For more information
 * see the CAN 2.0A, CAN 2.0B, ISO 11898-1 specifications.
 */

/*
 * Timing parameters to be set in the Bit Timing Register (BTR).
 * These values are for a 40 Kbps baudrate assuming the CAN input clock
 * frequency is 24 MHz.
 */
/** @name Bit Timing Register values
 * @{
 */
/** Bit Timing Register (BTR) synchronization jump width. */
#define TEST_BTR_SYNCJUMPWIDTH		3
/** Bit Timing Register (BTR) second time segment. */
#define TEST_BTR_SECOND_TIMESEGMENT	2
/** Bit Timing Register (BTR) first time segment. */
#define TEST_BTR_FIRST_TIMESEGMENT	15
/** @} */

/*
 * The Baud rate Prescalar value in the Baud Rate Prescaler Register
 * needs to be set based on the input clock  frequency to the CAN core and
 * the desired CAN baud rate.
 * This value is for a 40 Kbps baudrate assuming the CAN input clock frequency
 * is 24 MHz.
 */
/** Baud Rate Prescaler Register (BRPR) value. */
#define TEST_BRPR_BAUD_PRESCALAR	29

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifndef SDT
int CanPsIntrExample(INTC *IntcInstPtr,
		     XCanPs *CanInstPtr,
		     u16 CanDeviceId,
		     u16 CanIntrId);
#else
int CanPsIntrExample(XCanPs *CanInstPtr,
		     UINTPTR BaseAddress);
#endif
static void Config(XCanPs *InstancePtr);
static void SendFrame(XCanPs *InstancePtr);

static void SendHandler(void *CallBackRef);
static void RecvHandler(void *CallBackRef);
static void ErrorHandler(void *CallBackRef, u32 ErrorMask);
static void EventHandler(void *CallBackRef, u32 Mask);

#ifndef SDT
static int SetupInterruptSystem(INTC *IntcInstancePtr,
				XCanPs *CanInstancePtr,
				u16 CanIntrId);
#endif

/************************** Variable Definitions *****************************/

#ifndef TESTAPP_GEN
static XCanPs CanInstance;    /**< Instance of the Can driver. */
#ifndef SDT
static INTC IntcInstance; /**< Instance of the Interrupt Controller driver. */
#endif
#endif

/*
 * Buffers to hold frames to send and receive. These are declared as global so
 * that they are not on the stack.
 * These buffers need to be 32-bit aligned
 */
static u32 TxFrame[XCANPS_MAX_FRAME_SIZE_IN_WORDS]; /**< Frame to send. */
static u32 RxFrame[XCANPS_MAX_FRAME_SIZE_IN_WORDS]; /**< Frame received. */

/*
 * Shared variables used to test the callbacks.
 */
static volatile int LoopbackError;	/**< Asynchronous error occurred. */
static volatile int RecvDone;		/**< Received a frame. */
static volatile int SendDone;		/**< Frame was sent successfully. */

/****************************************************************************/
/**
*
* @brief Runs the CAN interrupt example.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
*****************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	xil_printf("CAN Interrupt Example Test \r\n");

	/*
	 * Run the Can interrupt example.
	 */
#ifndef SDT
	Status = CanPsIntrExample(&IntcInstance, &CanInstance,
				  CAN_DEVICE_ID, CAN_INTR_VEC_ID);
#else
	Status = CanPsIntrExample(&CanInstance,
				  XPAR_XCANPS_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("CAN Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran CAN Interrupt Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* @brief Demonstrates XCanPs driver operation in interrupt mode with loopback.
*
* The example configures the device for internal loop back mode, then
* sends a CAN frame and receives the same CAN frame.
*
* @param[out]    CanInstPtr Pointer to the instance of the CAN driver which
*                is going to be connected to the interrupt controller.
* @if SDT
* @param[in]     BaseAddress Base address of the device.
* @else
* @param[in,out] IntcInstPtr Pointer to the instance of the INTC driver.
* @param[in]     CanDeviceId Device Id of the CAN device and is typically
*                XPAR_<CANPS_instance>_DEVICE_ID value from xparameters.h.
* @param[in]     CanIntrId Interrupt Id and is typically
*                XPAR_<CANPS_instance>_INTR value from xparameters.h.
* @endif
*
* @return        XST_SUCCESS if successful, otherwise driver-specific error
*                code.
*
* @note          - In XSCT/classic flow, DeviceId is used to look up the
*                  device configuration.
*                - If the device is not working correctly, this function may
*                  enter an infinite loop and never returns to the caller.
*
*****************************************************************************/
#ifndef SDT
int CanPsIntrExample(INTC *IntcInstPtr, XCanPs *CanInstPtr,
		     u16 CanDeviceId, u16 CanIntrId)
#else
int CanPsIntrExample(XCanPs *CanInstPtr, UINTPTR BaseAddress)
#endif
{
	int Status;
	XCanPs_Config *ConfigPtr;

	/*
	 * Initialize the Can device.
	 */
#ifndef SDT
	ConfigPtr = XCanPs_LookupConfig(CanDeviceId);
#else
	ConfigPtr = XCanPs_LookupConfig(BaseAddress);
#endif
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XCanPs_CfgInitialize(CanInstPtr,
			     ConfigPtr,
			     ConfigPtr->BaseAddr);

	/*
	 * Run self-test on the device, which verifies basic sanity of the
	 * device and the driver.
	 */
	Status = XCanPs_SelfTest(CanInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Configure CAN device.
	 */
	Config(CanInstPtr);

	/*
	 * Set interrupt handlers.
	 */
	XCanPs_SetHandler(CanInstPtr, XCANPS_HANDLER_SEND,
			  (void *)SendHandler, (void *)CanInstPtr);
	XCanPs_SetHandler(CanInstPtr, XCANPS_HANDLER_RECV,
			  (void *)RecvHandler, (void *)CanInstPtr);
	XCanPs_SetHandler(CanInstPtr, XCANPS_HANDLER_ERROR,
			  (void *)ErrorHandler, (void *)CanInstPtr);
	XCanPs_SetHandler(CanInstPtr, XCANPS_HANDLER_EVENT,
			  (void *)EventHandler, (void *)CanInstPtr);

	/*
	 * Initialize the flags.
	 */
	SendDone = FALSE;
	RecvDone = FALSE;
	LoopbackError = FALSE;

	/*
	 * Connect to the interrupt controller.
	 */
#ifndef SDT
	Status =  SetupInterruptSystem(IntcInstPtr,
				       CanInstPtr,
				       CanIntrId);
#else
	Status = XSetupInterruptSystem(CanInstPtr, (void *)&XCanPs_IntrHandler,
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
	XCanPs_IntrEnable(CanInstPtr, XCANPS_IXR_ALL);

	/*
	 * Enter Loop Back Mode.
	 */
	XCanPs_EnterMode(CanInstPtr, XCANPS_MODE_LOOPBACK);
	while (XCanPs_GetMode(CanInstPtr) != XCANPS_MODE_LOOPBACK);

	/*
	 * Loop back a frame. The RecvHandler is expected to handle
	 * the frame reception.
	 */
	SendFrame(CanInstPtr); /* Send a frame */

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
* @brief Configures the CAN device baud rate and timing parameters.
*
* Baud Rate Prescaler Register (BRPR) and Bit Timing Register (BTR) are set
* in this function.
*
* @param[in]    InstancePtr Pointer to the driver instance.
*
* @note		If the CAN device is not working correctly, this function may
*               enter an infinite loop and never returns to the caller.
*
******************************************************************************/
static void Config(XCanPs *InstancePtr)
{
	/*
	 * Enter Configuration Mode if the device is not currently in
	 * Configuration Mode.
	 */
	XCanPs_EnterMode(InstancePtr, XCANPS_MODE_CONFIG);
	while (XCanPs_GetMode(InstancePtr) != XCANPS_MODE_CONFIG);

	/*
	 * Setup Baud Rate Prescaler Register (BRPR) and
	 * Bit Timing Register (BTR).
	 */
	XCanPs_SetBaudRatePrescaler(InstancePtr, TEST_BRPR_BAUD_PRESCALAR);
	XCanPs_SetBitTiming(InstancePtr, TEST_BTR_SYNCJUMPWIDTH,
			    TEST_BTR_SECOND_TIMESEGMENT,
			    TEST_BTR_FIRST_TIMESEGMENT);

}

/*****************************************************************************/
/**
*
* @brief Sends a CAN frame.
*
* @param[in,out] InstancePtr Pointer to the driver instance.
*
******************************************************************************/
static void SendFrame(XCanPs *InstancePtr)
{
	u8 *FramePtr;
	int Index;
	int Status;

	/*
	 * Create correct values for Identifier and Data Length Code Register.
	 */
	TxFrame[0] = (u32)XCanPs_CreateIdValue((u32)TEST_MESSAGE_ID, 0, 0, 0, 0);
	TxFrame[1] = (u32)XCanPs_CreateDlcValue((u32)FRAME_DATA_LENGTH);

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
	while (XCanPs_IsTxFifoFull(InstancePtr) == TRUE);

	Status = XCanPs_Send(InstancePtr, TxFrame);
	if (Status != XST_SUCCESS) {
		/*
		 * The frame could not be sent successfully.
		 */
		LoopbackError = TRUE;
		SendDone = TRUE;
		RecvDone = TRUE;
	}
}


/*****************************************************************************/
/**
*
* @brief Handles transmit completion callbacks.
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
	(void)CallBackRef;
	SendDone = TRUE;
}


/*****************************************************************************/
/**
*
* @brief Handles frame reception callbacks.
*
* This function is called once per frame received.
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
	XCanPs *CanPtr = (XCanPs *)CallBackRef;
	int Status;
	int Index;
	u8 *FramePtr;

	Status = XCanPs_Recv(CanPtr, RxFrame);
	if (Status != XST_SUCCESS) {
		LoopbackError = TRUE;
		RecvDone = TRUE;
		return;
	}

	/*
	 * Verify Identifier and Data Length Code.
	 */
	if (RxFrame[0] != (u32)XCanPs_CreateIdValue((u32)TEST_MESSAGE_ID, 0, 0, 0, 0)) {
		LoopbackError = TRUE;
		RecvDone = TRUE;
		return;
	}

	if ((RxFrame[1] & ~XCANPS_DLCR_TIMESTAMP_MASK) != TxFrame[1]) {
		LoopbackError = TRUE;
		RecvDone = TRUE;
		return;
	}

	/*
	 * Verify the Data field contents.
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
* @brief Handles error interrupt callbacks.
*
* Error code read from Error Status register is passed into this function.
*
* @param[in]   CallBackRef Callback reference passed from the interrupt
*              handler, which in our case is a pointer to the driver instance.
* @param[in]   ErrorMask Bit mask indicating the cause of the error.
*              Its value is a bitwise OR of one or more XCANPS_ESR_* values
*              defined in xcanps_hw.h.
*
* @note        This function is called by the driver within interrupt context.
*
******************************************************************************/
static void ErrorHandler(void *CallBackRef, u32 ErrorMask)
{
	(void)CallBackRef;
	if (ErrorMask & XCANPS_ESR_ACKER_MASK) {
		/*
		 * ACK Error handling code should be put here.
		 */
	}

	if (ErrorMask & XCANPS_ESR_BERR_MASK) {
		/*
		 * Bit Error handling code should be put here.
		 */
	}

	if (ErrorMask & XCANPS_ESR_STER_MASK) {
		/*
		 * Stuff Error handling code should be put here.
		 */
	}

	if (ErrorMask & XCANPS_ESR_FMER_MASK) {
		/*
		 * Form Error handling code should be put here.
		 */
	}

	if (ErrorMask & XCANPS_ESR_CRCER_MASK) {
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
* Handles the following interrupts:
*   - XCANPS_IXR_BSOFF_MASK:	Bus Off Interrupt
*   - XCANPS_IXR_RXOFLW_MASK:	RX FIFO Overflow Interrupt
*   - XCANPS_IXR_RXUFLW_MASK:	RX FIFO Underflow Interrupt
*   - XCANPS_IXR_TXBFLL_MASK:	TX High Priority Buffer Full Interrupt
*   - XCANPS_IXR_TXFLL_MASK:	TX FIFO Full Interrupt
*   - XCANPS_IXR_WKUP_MASK:	Wake up Interrupt
*   - XCANPS_IXR_SLP_MASK:	Sleep Interrupt
*   - XCANPS_IXR_ARBLST_MASK:	Arbitration Lost Interrupt
*
* @param[in]   CallBackRef Callback reference passed from the
*              interrupt Handler, which in our case is a pointer to the
*              driver instance.
* @param[in]   IntrMask Bit mask indicating pending interrupts.
*              Its value is a bitwise OR of one or more of the
*              XCANPS_IXR_*_MASK values mentioned above.
*
* @note        This function is called by the driver within interrupt context.
*              This function should be changed to meet specific application
*              needs.
*
******************************************************************************/
static void EventHandler(void *CallBackRef, u32 IntrMask)
{
	XCanPs *CanPtr = (XCanPs *)CallBackRef;

	if (IntrMask & XCANPS_IXR_BSOFF_MASK) {
		/*
		 * Entering Bus off status interrupt requires
		 * the CAN device be reset and reconfigured.
		 */
		XCanPs_Reset(CanPtr);
		Config(CanPtr);
		return;
	}

	if (IntrMask & XCANPS_IXR_RXOFLW_MASK) {
		/*
		 * Code to handle RX FIFO Overflow Interrupt should be put here.
		 */
	}

	if (IntrMask & XCANPS_IXR_RXUFLW_MASK) {
		/*
		 * Code to handle RX FIFO Underflow Interrupt
		 * should be put here.
		 */
	}

	if (IntrMask & XCANPS_IXR_TXBFLL_MASK) {
		/*
		 * Code to handle TX High Priority Buffer Full
		 * Interrupt should be put here.
		 */
	}

	if (IntrMask & XCANPS_IXR_TXFLL_MASK) {
		/*
		 * Code to handle TX FIFO Full Interrupt should be put here.
		 */
	}

	if (IntrMask & XCANPS_IXR_WKUP_MASK) {
		/*
		 * Code to handle Wake up from sleep mode Interrupt
		 * should be put here.
		 */
	}

	if (IntrMask & XCANPS_IXR_SLP_MASK) {
		/*
		 * Code to handle Enter sleep mode Interrupt should be put here.
		 */
	}

	if (IntrMask & XCANPS_IXR_ARBLST_MASK) {
		/*
		 * Code to handle Lost bus arbitration Interrupt
		 * should be put here.
		 */
	}
}

#ifndef SDT
/*****************************************************************************/
/**
*
* @brief Sets up the interrupt system for CAN.
*
* This function is application-specific since the actual system may or
* may not have an interrupt controller. The CAN could be directly connected
* to a processor without an interrupt controller. Modify this function to fit
* the application.
*
* @param[in,out] IntcInstancePtr Pointer to the instance of the ScuGic.
* @param[in]     CanInstancePtr Pointer to the instance of the CAN
*                which is going to be connected to the interrupt
*                controller.
* @param[in]     CanIntrId Interrupt Id and is typically
*                XPAR_<CANPS_instance>_INTR value from xparameters.h.
*
* @return        XST_SUCCESS if successful, otherwise XST_FAILURE.
*
****************************************************************************/
static int SetupInterruptSystem(INTC *IntcInstancePtr,
				XCanPs *CanInstancePtr,
				u16 CanIntrId)
{
	int Status;
#ifdef XPAR_INTC_0_DEVICE_ID
#ifndef TESTAPP_GEN
	/* Initialize the interrupt controller and connect the ISRs */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed init intc\r\n");
		return XST_FAILURE;
	}
#endif
	/*
	 * Connect the driver interrupt handler
	 */
	Status = XIntc_Connect(IntcInstancePtr, CanIntrId,
			       (XInterruptHandler)XCanPs_IntrHandler, CanInstancePtr);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed connect intc\r\n");
		return XST_FAILURE;
	}
#ifndef TESTAPP_GEN
	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Enable the interrupt for the CAN device.
	 */
	XIntc_Enable(IntcInstancePtr, CanIntrId);
#ifndef TESTAPP_GEN
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XIntc_InterruptHandler,
				     (void *)IntcInstancePtr);
#endif
#else
#ifndef TESTAPP_GEN
	/** Instance of the interrupt controller. */
	XScuGic_Config *IntcConfig;

	Xil_ExceptionInit();

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				     IntcInstancePtr);
#endif

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, CanIntrId,
				 (Xil_InterruptHandler)XCanPs_IntrHandler,
				 (void *)CanInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the CAN device.
	 */
	XScuGic_Enable(IntcInstancePtr, CanIntrId);
#endif
#ifndef TESTAPP_GEN
	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();
#endif
	return XST_SUCCESS;
}
#endif
