/******************************************************************************
*
* Copyright (C) 2005 - 2018 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xcan_intr_example.c
*
* Contains an example of how to use the XCan driver directly.  The example here
* shows using the driver/device in interrupt mode.
*
* @note
*
* This code assumes that Xilinx interrupt controller (XIntc) is used in the
* system to forward the CAN device interrupt output to the processor and no
* operating system is being used.
*
* The Baud Rate Prescaler Register (BRPR) and Bit Timing Register (BTR)
* are setup such that CAN baud rate equals 40Kbps, assuming that the
* the CAN clock is 24MHz. The user needs to modify these values based on
* the desired baud rate and the CAN clock frequency. For more information
* see the CAN 2.0A, CAN 2.0B, ISO 11898-1 specifications.
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
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcan.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xil_exception.h"

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
#define CAN_DEVICE_ID		XPAR_CAN_0_DEVICE_ID
#define CAN_INTR_VEC_ID		XPAR_INTC_0_CAN_0_VEC_ID

#ifdef XPAR_INTC_0_DEVICE_ID
 #define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#else
 #define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif /* XPAR_INTC_0_DEVICE_ID */


/* Maximum CAN frame length in word */
#define XCAN_MAX_FRAME_SIZE_IN_WORDS (XCAN_MAX_FRAME_SIZE / sizeof(u32))

#define FRAME_DATA_LENGTH	8 /* Frame Data field length */

/*
 * Message Id Constant.
 */
#define TEST_MESSAGE_ID			1024

/*
 * The Baud Rate Prescaler Register (BRPR) and Bit Timing Register (BTR)
 * are setup such that CAN baud rate equals 40Kbps, assuming that the
 * the CAN clock is 24MHz. The user needs to modify these values based on
 * the desired baud rate and the CAN clock frequency. For more information
 * see the CAN 2.0A, CAN 2.0B, ISO 11898-1 specifications.
 */
#define TEST_BRPR_BAUD_PRESCALAR	29

#define TEST_BTR_SYNCJUMPWIDTH		3
#define TEST_BTR_SECOND_TIMESEGMENT	2
#define TEST_BTR_FIRST_TIMESEGMENT	15


#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC		XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

static int XCanIntrExample(u16 DeviceId);
static void Config(XCan *InstancePtr);
static void SendFrame(XCan *InstancePtr);

static void SendHandler(void *CallBackRef);
static void RecvHandler(void *CallBackRef);
static void ErrorHandler(void *CallBackRef, u32 ErrorMask);
static void EventHandler(void *CallBackRef, u32 Mask);

static int SetupInterruptSystem(XCan *InstancePtr);

/************************** Variable Definitions *****************************/

/*
 * Allocate an instance of the XCan driver
 */
static XCan Can;

/*
 * Buffers to hold frames to send and receive. These are declared as global so
 * that they are not on the stack.
 * These buffers need to be 32-bit aligned
 */
static u32 TxFrame[XCAN_MAX_FRAME_SIZE_IN_WORDS];
static u32 RxFrame[XCAN_MAX_FRAME_SIZE_IN_WORDS];

/*
 * Shared variables used to test the callbacks.
 */
volatile static int LoopbackError;	/* Asynchronous error occurred */
volatile static int RecvDone;		/* Received a frame */
volatile static int SendDone;		/* Frame was sent successfully */

/*****************************************************************************/
/**
*
* This function is the main function of the Can interrupt example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
*****************************************************************************/
int main()
{
	/*
	 * Run the Can interrupt example.
	 */
	if (XCanIntrExample(CAN_DEVICE_ID)) {
		xil_printf("Can Interrupt Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Can Interrupt Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The main entry point for showing the XCan driver in interrupt mode.
* The example configures the device for internal loopback mode, then
* sends a CAN frame and receives the same CAN frame.
*
* @param	DeviceId contains the CAN device ID.
*
* @return	XST_SUCCESS if successful, otherwise driver-specific error code.
*
* @note 	If the device is not working correctly, this function may enter
*		an infinite loop and will never return to the caller.
*
******************************************************************************/
static int XCanIntrExample(u16 DeviceId)
{
	int Status;

	/*
	 * Initialize the XCan driver.
	 */
	Status = XCan_Initialize(&Can, DeviceId);
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
	Status = SetupInterruptSystem(&Can);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable all interrupts in CAN device.
	 */
	XCan_InterruptEnable(&Can, XCAN_IXR_ALL);

	/*
	 * Enter Loop Back Mode.
	 */
	XCan_EnterMode(&Can, XCAN_MODE_LOOPBACK);
	while(XCan_GetMode(&Can) != XCAN_MODE_LOOPBACK);

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
* This function configures CAN device. Baud Rate Prescaler Register (BRPR) and
* Bit Timing Register (BTR) are set in this function.
*
* @param	InstancePtr is a pointer to the driver instance
*
* @return	None.
*
* @note		If the CAN device is not working correctly, this function may
*		enter an infinite loop and will never return to the caller.
*
******************************************************************************/
static void Config(XCan *InstancePtr)
{
	/*
	 * Enter Configuration Mode if the device is not currently in
	 * Configuration Mode.
	 */
	XCan_EnterMode(InstancePtr, XCAN_MODE_CONFIG);
	while(XCan_GetMode(InstancePtr) != XCAN_MODE_CONFIG);

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
* Send a CAN frame.
*
* @param	InstancePtr is a pointer to the driver instance.
*
* @return	None.
*
* @note		None.
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
* Callback function (called from interrupt handler) to handle confirmation of
* transmit events when in interrupt mode.
*
* @param	CallBackRef is the callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
*
* @return	None.
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
* Callback function (called from interrupt handler) to handle frames received in
* interrupt mode.  This function is called once per frame received.
* The driver's receive function is called to read the frame from RX FIFO.
*
* @param	CallBackRef is the callback reference passed from the interrupt
*		handler, which in our case is a pointer to the device instance.
*
* @return	None.
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
	for (Index = 0; Index < FRAME_DATA_LENGTH; Index++){
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
* Callback function (called from interrupt handler) to handle error interrupt.
* Error code read from Error Status register is passed into this function
*
* @param	CallBackRef is the callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
*		value equals 'OR'ing one or more XCAN_ESR_* defined in xcan_l.h
*
* @return	None.
*
* @note		This function is called by the driver within interrupt context.
*
******************************************************************************/
static void ErrorHandler(void *CallBackRef, u32 ErrorMask)
{

	if(ErrorMask & XCAN_ESR_ACKER_MASK) {
		/*
		 * ACK Error handling code should be put here.
		 */
	}

	if(ErrorMask & XCAN_ESR_BERR_MASK) {
		/*
		 * Bit Error handling code should be put here.
		 */
	}

	if(ErrorMask & XCAN_ESR_STER_MASK) {
		/*
		 * Stuff Error handling code should be put here.
		 */
	}

	if(ErrorMask & XCAN_ESR_FMER_MASK) {
		/*
		 * Form Error handling code should be put here.
		 */
	}

	if(ErrorMask & XCAN_ESR_CRCER_MASK) {
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
* Please feel free to change this function to meet specific application needs.
*
* @param	CallBackRef is the callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
* @param	IntrMask is a bit mask indicating pending interrupts. Its value
*		equals 'OR'ing one or more of the XCAN_IXR_*_MASK value(s)
*		mentioned above.
*
* @return	None.
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

	if(IntrMask & XCAN_IXR_RXOFLW_MASK) { /* RX FIFO Overflow Interrupt */
		/*
		 * Code to handle RX FIFO Overflow
		 * Interrupt should be put here.
		 */
	}

	if(IntrMask & XCAN_IXR_RXUFLW_MASK) { /* RX FIFO Underflow Interrupt */
		/*
		 * Code to handle RX FIFO Underflow
		 * Interrupt should be put here.
		 */
	}

	if(IntrMask & XCAN_IXR_TXBFLL_MASK) { /* TX High Priority Full Intr */
		/*
		 * Code to handle TX High Priority Buffer Full
		 * Interrupt should be put here.
		 */
	}

	if(IntrMask & XCAN_IXR_TXFLL_MASK) { /* TX FIFO Full Interrupt */
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
}

/*****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* CAN. This function is application-specific since the actual system may or
* may not have an interrupt controller.  The CAN could be directly connected
* to a processor without an interrupt controller.  The user should modify this
* function to fit the application.
*
* @para		InstancePtr is a pointer to the instance of the CAN
*		which is going to be connected to the interrupt controller.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
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
	if (Status != XST_SUCCESS){
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

