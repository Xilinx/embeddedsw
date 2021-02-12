/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xcanps_watermark_intr_example.c
*
* This example shows how to use the CAN driver/device in interrupt mode using
* the Rx Watermark Interrupt.
*
*
* @note
* The Baud Rate Prescaler Register (BRPR) and Bit Timing Register (BTR)
* are setup such that CAN baud rate equals 40Kbps, assuming that the
* the CAN clock is 24MHz. The user needs to modify these values based on
* the desired bau rate and the CAN clock frequency. For more information
* see the CAN 2.0A, CAN 2.0B, ISO 11898-1 specifications.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.00a xd/sv  01/12/10 First release
* 3.1   adk    10/11/15 Fixed CR#911958 Add support for Tx Watermark testing.
*
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xcanps.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define CAN_DEVICE_ID		XPAR_XCANPS_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define CAN_INTR_VEC_ID		XPAR_XCANPS_0_INTR

/*
 * Maximum CAN frame length in words.
 */
#define XCANPS_MAX_FRAME_SIZE_IN_WORDS (XCANPS_MAX_FRAME_SIZE / sizeof(u32))

#define FRAME_DATA_LENGTH	8 /* Frame Data field length */

#define TEST_THRESHOLD		25 /* This is Rx FIFO WaterMark Threshold */

/*
 * Message Id Constant.
 */
#define TEST_MESSAGE_ID		2000

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
#define TEST_BTR_SYNCJUMPWIDTH		3
#define TEST_BTR_SECOND_TIMESEGMENT	2
#define TEST_BTR_FIRST_TIMESEGMENT	15

/*
 * The Baud rate Prescalar value in the Baud Rate Prescaler Register
 * needs to be set based on the input clock  frequency to the CAN core and
 * the desired CAN baud rate.
 * This value is for a 40 Kbps baudrate assuming the CAN input clock frequency
 * is 24 MHz.
 */
#define TEST_BRPR_BAUD_PRESCALAR	29

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int CanPsWatermarkIntrExample(XScuGic *IntcInstPtr,
				XCanPs *CanInstPtr,
				u16 CanDeviceId,
				u16 CanIntrId);

static void Config(XCanPs *InstancePtr);
static void SendFrame(XCanPs *InstancePtr);
static int ReceiveData(XCanPs *InstancePtr);

static void SendHandler(void *CallBackRef);
static void RecvHandler(void *CallBackRef);
static void ErrorHandler(void *CallBackRef, u32 ErrorMask);
static void EventHandler(void *CallBackRef, u32 Mask);

static int SetupInterruptSystem(XScuGic *IntcInstancePtr,
				XCanPs *CanInstancePtr,
				u16 CanIntrId);


/************************** Variable Definitions *****************************/

static XCanPs CanInstance;   /* Instance of the Can driver */
static XScuGic IntcInstance; /* Instance of the Interrupt Controller driver */


/*
 * Buffers to hold frames to send and receive. These are declared as global so
 * that they are not on the stack.
 * These buffers need to be 32-bit aligned
 */
static u32 TxFrame[XCANPS_MAX_FRAME_SIZE_IN_WORDS];
static u32 RxFrame[XCANPS_MAX_FRAME_SIZE_IN_WORDS];

/*
 * Shared variables used to test the callbacks.
 */
volatile static int LoopbackError;	/* Asynchronous error occurred */
volatile static int RecvDone;		/* Received a frame */
volatile static int SendDone;		/* Frame was sent successfully */

static u8 TestDataOffset;	/* Test Data value added to the CAN data */

/****************************************************************************/
/**
*
* This function is the main function of the Can Rx Watermark interrupt example.
*
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None
*
*****************************************************************************/
int main(void)
{
	int Status;

	xil_printf("CAN Watermark Example Test \r\n");

	/*
	 * Run the Can Rx Watermark interrupt example.
	 */
	Status = CanPsWatermarkIntrExample(&IntcInstance, &CanInstance,
					    CAN_DEVICE_ID, CAN_INTR_VEC_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("CAN Watermark Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran CAN Watermark Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The main entry point for showing the usage of XCanPs driver in interrupt
* mode. The example configures the device for internal loop back mode, then
* sends multiple CAN frames and receives the same number of CAN frame's
* using the Rx Watermark Interrupt.
*
* @param	IntcInstPtr is a pointer to the instance of the ScuGic driver.
* @param	CanInstPtr is a pointer to the instance of the CAN driver which
*		is going to be connected to the interrupt controller.
* @param	CanDeviceId is the device Id of the CAN device and is typically
*		XPAR_<CANPS_instance>_DEVICE_ID value from xparameters.h.
* @param	CanIntrId is the interrupt Id and is typically
*		XPAR_<CANPS_instance>_INTR value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise driver-specific error code.
*
* @note		If the device is not working correctly, this function may enter
*		an infinite loop and will never return to the caller.
*
******************************************************************************/
int CanPsWatermarkIntrExample(XScuGic *IntcInstPtr, XCanPs *CanInstPtr,
				u16 CanDeviceId, u16 CanIntrId)
{
	int Status;
	XCanPs_Config *ConfigPtr;
	u32 Index;

	/*
	 * Initialize the Can device.
	 */
	ConfigPtr = XCanPs_LookupConfig(CanDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XCanPs_CfgInitialize(CanInstPtr,
					ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Run self-test on the device, which verifies basic sanity of the
	 * device and the driver.
	 */
	Status = XCanPs_SelfTest(CanInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Configure the CAN device.
	 */
	Config(CanInstPtr);

	/*
	 * Set the interrupt handlers.
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
	 * Initialize flags.
	 */
	SendDone = FALSE;
	RecvDone = FALSE;
	LoopbackError = FALSE;

	/*
	 * Connect to the interrupt controller.
	 */
	Status =  SetupInterruptSystem(IntcInstPtr,
					CanInstPtr,
					CanIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable all interrupts in CAN device.
	 */
	XCanPs_IntrEnable(CanInstPtr, XCANPS_IXR_ALL);
	XCanPs_IntrEnable(CanInstPtr, XCANPS_IXR_TXFWMEMP_MASK);

	/*
	 * Disable the Receive FIFO Not Empty Interrupt and the
	 * New Message Received Interrupt.
	 */
	XCanPs_IntrDisable(CanInstPtr,
				XCANPS_IXR_RXNEMP_MASK |
				XCANPS_IXR_RXOK_MASK | XCANPS_IXR_TXOK_MASK);

	/*
	 * Enter Loop Back Mode.
	 */
	XCanPs_EnterMode(CanInstPtr, XCANPS_MODE_LOOPBACK);
	while(XCanPs_GetMode(CanInstPtr) != XCANPS_MODE_LOOPBACK);

	/*
	 * Send a number of frames.
	 */
	TestDataOffset = 1;
	for (Index = 0; Index < TEST_THRESHOLD; Index++) {

		SendFrame(CanInstPtr); /* Send a frame */
		TestDataOffset++;
	}

	/*
	 * Wait here until both sending and reception have been completed.
	 */
	while ((SendDone != TRUE) || (RecvDone != TRUE));

	/*
	 * Check for errors found in the callbacks.
	 */
	if (LoopbackError == TRUE) {
		return XST_FAILURE;
	}

	/*
	 * Read the Received Frames from the FIFO.
	 */
	TestDataOffset = 1;
	Status = ReceiveData(CanInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

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
* This function configures CAN device. Baud Rate Prescaler Register (BRPR),
* Bit Timing Register (BTR) and RXFIFO Watermark Interrupt Register (RXWIR)
* are set in this function.
*
* @param	InstancePtr is a pointer to the driver instance.
*
* @return	None.
*
* @note		If the CAN device is not working correctly, this function may
*		enter an infinite loop and will never return to the caller.
*
******************************************************************************/
static void Config(XCanPs *InstancePtr)
{
	/*
	 * Enter Configuration Mode if the device is not currently in
	 * Configuration Mode.
	 */
	XCanPs_EnterMode(InstancePtr, XCANPS_MODE_CONFIG);
	while(XCanPs_GetMode(InstancePtr) != XCANPS_MODE_CONFIG);

	/*
	 * Setup Baud Rate Prescaler Register (BRPR) and
	 * Bit Timing Register (BTR) .
	 */
	XCanPs_SetBaudRatePrescaler(InstancePtr, TEST_BRPR_BAUD_PRESCALAR);
	XCanPs_SetBitTiming(InstancePtr, TEST_BTR_SYNCJUMPWIDTH,
					TEST_BTR_SECOND_TIMESEGMENT,
					TEST_BTR_FIRST_TIMESEGMENT);

	/*
	 * Set the threshold value for the Rx FIFO Watermark interrupt.
	 */
	XCanPs_SetRxIntrWatermark(InstancePtr, TEST_THRESHOLD - 1);
	XCanPs_SetTxIntrWatermark(InstancePtr, TEST_THRESHOLD - 1);
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
		*FramePtr++ = ((u8)Index + TestDataOffset);
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
		SendDone = TEST_THRESHOLD;
		RecvDone = TRUE;
	}
}

/*****************************************************************************/
/**
*
* Read the Received CAN frames from the FIFO.
*
* @param	InstancePtr is a pointer to the driver instance.
*
* @return
*		- XST_SUCCESS if all the CAN Frames are received and the
*		data is the same as that was sent.
*		- XST_FAILURE if the required number of CAN frames have not
*		been received or if the Received Data is not the same as the
*		data that was sent.
*
* @note		None.
*
******************************************************************************/
static int ReceiveData(XCanPs *InstancePtr)
{
	int Status;
	int Index;
	u8 *FramePtr;
	u8 NumRxFrames;

	/*
	 * Initialize the number of received frames to Zero.
	 */
	NumRxFrames = 0;

	/*
	 * Read the received CAN Frames from the FIFO till the FIFO is Empty.
	 */
	while (XCanPs_IntrGetStatus(InstancePtr) & XCANPS_IXR_RXNEMP_MASK) {

		Status = XCanPs_Recv(InstancePtr, RxFrame);
		if (Status != XST_SUCCESS) {
			LoopbackError = TRUE;
			return XST_FAILURE;
		}

		/*
		 * Verify Identifier and Data Length Code.
		 */
		if (RxFrame[0] !=
			(u32)XCanPs_CreateIdValue((u32)TEST_MESSAGE_ID, 0, 0, 0, 0)) {

			LoopbackError = TRUE;
			return XST_FAILURE;
		}
		if ((RxFrame[1] & ~XCANPS_DLCR_TIMESTAMP_MASK) != TxFrame[1]) {
			LoopbackError = TRUE;
			return XST_FAILURE;
		}

		/*
		 * Verify Data field contents.
		 */
		FramePtr = (u8 *)(&RxFrame[2]);
		for (Index = 0; Index < FRAME_DATA_LENGTH; Index++) {
			if (*FramePtr++ != ((u8)Index + TestDataOffset)) {
				LoopbackError = TRUE;
				return XST_FAILURE;
			}
		}

		/*
		 * Increment the number of frames received.
		 */
		TestDataOffset++;
		NumRxFrames++;

	}

	if (NumRxFrames == TEST_THRESHOLD) {
		LoopbackError = FALSE;
		return XST_SUCCESS;
	}
	return XST_FAILURE;
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

	XCanPs *CanInstPtr = (XCanPs *)CallBackRef;

	/*
	 * The Tx FIFO Empty Watermark level specified.
	 */
	XCanPs_IntrDisable(CanInstPtr, XCANPS_IXR_TXFWMEMP_MASK);
	SendDone = TRUE;
}


/*****************************************************************************/
/**
*
* Callback function (called from interrupt handler) to handle frames received in
* interrupt mode.  This function is called once all the frames are received.
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
	XCanPs *CanInstPtr = (XCanPs *)CallBackRef;

	/*
	 * The RX FIFO is Full to the Watermark level specified.
	 */
	XCanPs_IntrDisable(CanInstPtr, XCANPS_IXR_RXFWMFLL_MASK);
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
* @param	ErrorMask is a bit mask indicating the cause of the error.
*		Its value equals 'OR'ing one or more XCANPS_ESR_* defined in
*		xcanps_hw.h
*
* @return	None.
*
* @note		This function is called by the driver within interrupt context.
*
******************************************************************************/
static void ErrorHandler(void *CallBackRef, u32 ErrorMask)
{
	if(ErrorMask & XCANPS_ESR_ACKER_MASK) {
		/*
		 * ACK Error handling code should be put here.
		 */
	}

	if(ErrorMask & XCANPS_ESR_BERR_MASK) {
		/*
		 * Bit Error handling code should be put here.
		 */
	}

	if(ErrorMask & XCANPS_ESR_STER_MASK) {
		/*
		 * Stuff Error handling code should be put here.
		 */
	}

	if(ErrorMask & XCANPS_ESR_FMER_MASK) {
		/*
		 * Form Error handling code should be put here.
		 */
	}

	if(ErrorMask & XCANPS_ESR_CRCER_MASK) {
		/*
		 * CRC Error handling code should be put here.
		 */
	}

	/*
	 * Set the shared variables.
	 */
	LoopbackError = TRUE;
	RecvDone = TRUE;
	SendDone = TEST_THRESHOLD;
}


/*****************************************************************************/
/**
*
* Callback function (called from interrupt handler) to handle the following
* interrupts:
*   - XCANPS_IXR_BSOFF_MASK:  Bus Off Interrupt
*   - XCANPS_IXR_RXOFLW_MASK: RX FIFO Overflow Interrupt
*   - XCANPS_IXR_RXUFLW_MASK: RX FIFO Underflow Interrupt
*   - XCANPS_IXR_TXBFLL_MASK: TX High Priority Buffer Full Interrupt
*   - XCANPS_IXR_TXFLL_MASK:  TX FIFO Full Interrupt
*   - XCANPS_IXR_WKUP_MASK:   Wake up Interrupt
*   - XCANPS_IXR_SLP_MASK:    Sleep Interrupt
*   - XCANPS_IXR_ARBLST_MASK: Arbitration Lost Interrupt
*
*
* @param	CallBackRef is the callback reference passed from the
*		interrupt Handler, which in our case is a pointer to the
*		driver instance.
* @param	IntrMask is a bit mask indicating pending interrupts.
*		Its value equals 'OR'ing one or more of the XCANPS_IXR_*_MASK
*		value(s) mentioned above.
*
* @return	None.
*
* @note		This function is called by the driver within interrupt context.
*		This function needs to be changed to meet specific application
*		needs.
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

	if(IntrMask & XCANPS_IXR_RXOFLW_MASK) {
		/*
		 * Code to handle RX FIFO Overflow
		 * Interrupt should be put here.
		 */
	}

	if(IntrMask & XCANPS_IXR_RXUFLW_MASK) {
		/*
		 * Code to handle RX FIFO Underflow
		 * Interrupt should be put here.
		 */
	}

	if(IntrMask & XCANPS_IXR_TXBFLL_MASK) {
		/*
		 * Code to handle TX High Priority Buffer Full
		 * Interrupt should be put here.
		 */
	}

	if(IntrMask & XCANPS_IXR_TXFLL_MASK) {
		/*
		 * Code to handle TX FIFO Full
		 * Interrupt should be put here.
		 */
	}

	if (IntrMask & XCANPS_IXR_WKUP_MASK) {
		/*
		 * Code to handle Wake up from sleep mode
		 * Interrupt should be put here.
		 */
	}

	if (IntrMask & XCANPS_IXR_SLP_MASK) {
		/*
		 * Code to handle Enter sleep mode
		 * Interrupt should be put here.
		 */
	}

	if (IntrMask & XCANPS_IXR_ARBLST_MASK) {
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
* may not have an interrupt controller. The CAN could be directly connected
* to a processor without an interrupt controller. The user should modify this
* function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the instance of ScuGic driver.
* @param	CanInstancePtr contains a pointer to the instance of the CAN
*		which is going to be connected to the interrupt controller.
* @param	CanIntrId is the interrupt Id and is typically
*		XPAR_<CANPS_instance>_INTR value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
static int SetupInterruptSystem(XScuGic *IntcInstancePtr,
				XCanPs *CanInstancePtr,
				u16 CanIntrId)
{
	int Status;

#ifndef TESTAPP_GEN
	XScuGic_Config *IntcConfig; /* Instance of the interrupt controller */

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

#ifndef TESTAPP_GEN
	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();
#endif
	return XST_SUCCESS;
}
