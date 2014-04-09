/******************************************************************************
*
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* (c) Copyright 2007-2013 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
 * @file xusb_mouse.c
 *
 * This file contains an example for USB Mouse.
 *
 * @note
 *		- The example is tested on ML403 and ML507 evaluation boards.
 *		  The push buttons SW3,SW4,SW5 and SW7 on the ML403 evaluation
 *		  board are used in the example for the mouse movement of the
 *		  USB mouse. The push buttons used on ML507 board are SW10,SW11
 *		  SW12,SW13 and SW14.
 *		  The cursor on the host PC moves as and when any of the SW3 or
 *		  SW4 or SW5 and SW7 push button is pressed. The SW6 push button
 *		  switch is used to complete the test.
 *		  The push buttons on the ML507 board for cursor movement are
 *		  SW10, SW11, SW 12 and SW13. The push button for completing the
 *		  test is SW14.
 *		- The GPIO device has to be added to the hardware design so that
 *		  the push buttons on the evaluation board could be used. If we
 *		  enable the debug statements in the xusb_cp9.c file, we must
 *		  add the UARTLite core to the hardware design. Debug messages
 *		  can be enabled by defining the constant XUSB_DEBUG,
 *		- To run this example, the evaluation board is to be connected
 *		  to a windows Host PC over the USB port.
 *		- The example configures the USB device for endpoint 0 and
 *		  endpoint 1. Endpoint 0 is the control endpoint and is
 *		  configured for a maximum packet length of 64 bytes. End point
 *		  1 is configured for INTERRUPT IN transactions and the maximum
 *		  packet size is configured as 16 bytes.
 *		- The USB mouse example code has to be compiled along with the
 *		  xusb_cp9.c file. The xusb_cp9.c file contains all the USB
 *		  enumeration related functions. To compile the code for USB
 *		  mouse example, the constant definitions HID_DEVICES
 *		  and USB_MOUSE are to be defined and the definitions the
 *		  constants USB_KEYBOARD and MASS_STORAGE_DEVICE are to be
 *		  undefined. These definitions can be found in the xusb_types.h
 *		  file.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -----------------------------------------------------------------
 * 1.00a hvm  6/21/07 First release
 * 3.00a hvm  11/18/09 Updated to use HAL processor APIs.
 *		       XUsb_mReadReg is renamed to XUsb_ReadReg and
 *		       XUsb_mWriteReg is renamed to XUsb_WriteReg.
 * 4.02a bss  11/01/11 Modified UsbIfIntrHandler function to unconditionally
 *				reset when USB reset is asserted (CR 627574).
 *
 * </pre>
 *****************************************************************************/
/***************************** Include Files *********************************/

#include "xusb.h"
#include "xintc.h"
#include "xusb_mouse.h"
#include "stdio.h"
#include "xgpio.h"
#include "xil_exception.h"

/************************** Constant Definitions *****************************/

#define USB_DEVICE_ID		XPAR_USB_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define USB_INTR		XPAR_INTC_0_USB_0_VEC_ID

/*
 * The GPIO_DEVICE_ID is to be modified as per the name provided during the
 * system design. Default name for default system built for ML403 board is
 * XPAR_PUSH_BUTTONS_POSITION_DEVICE_ID and the one for ML507 board is
 * XPAR_PUSH_BUTTONS_5BIT_DEVICE_ID.
 *
 */
#define GPIO_DEVICE_ID		XPAR_PUSH_BUTTONS_4BITS_DEVICE_ID
#define INTC_GPIO_INTERRUPT_ID  XPAR_INTC_0_GPIO_0_VEC_ID

#define GPIO_ALL_BUTTONS  0x1F	/* The GPIO bits 0 to 4. */
#define EXIT_BUTTON	0x0010  /* The GPIO_SW_C on the ML403 board */
#define BUTTON_CHANNEL 1	/* Channel 1 of the GPIO Device */
#define BUTTON_INTERRUPT XGPIO_IR_CH1_MASK  /* Channel 1 Interrupt Mask */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int UsbMouseExample (u16 UsbId,	u16 GpioId);

static int SetupInterruptSystem(XUsb *UsbInstancePtr,
				XGpio *Gpio);

/************************** Variable Definitions *****************************/

XUsb UsbInstance;		/* The instance of the USB device */
static XGpio Gpio; 		/* The Instance of the GPIO Driver */

XUsb_Config	*UsbConfigPtr;	/* Instance of the USB config structure */
XGpio_Config *GpioConfigPtr;	/* Pointer to the GPIO config structure */

XIntc Intc;			/* Instance of the Interrupt Controller */
volatile int StopTest = FALSE;

/****************************************************************************/
/**
* This function is the main function of the USB Mouse example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful,
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
*
*****************************************************************************/
int main(void)
{
	int Status;

	Status = UsbMouseExample(USB_DEVICE_ID, GPIO_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
 * This function implements the USB mouse application.
 * This function sets up the ML403 evaluation board as a USB mouse.
 * The mouse cursor movement can be seen on the PC as and when any of the push
 * buttons SW3, SW4, SW5 and SW7 on the ML403 Evaluation board is pressed.
 * Pressing the push button SW6 stops the test.
 *
 * @param	UsbId is the USB device id.
 * @param	GpioId is the GPIO device id.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if test fails.
 *
 * @note	None.
 *
 *****************************************************************************/
int UsbMouseExample (u16 UsbId,	u16 GpioId)
{
	int Status;

	/*
	 * Initialize the USB driver.
	 */
	UsbConfigPtr = XUsb_LookupConfig(UsbId);
	if (NULL == UsbConfigPtr) {
		return XST_FAILURE;
	}

	/*
	 * We are passing the physical base address as the third argument
	 * because the physical and virtual base address are the same in our
	 * example. For systems that support virtual memory, the third
	 * argument needs to be the virtual base address.
	 */
	Status = XUsb_CfgInitialize(&UsbInstance,
				       UsbConfigPtr,
				       UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the GPIO driver.
	 */
	GpioConfigPtr = XGpio_LookupConfig(GpioId);
	if (GpioConfigPtr == NULL) {
		return XST_FAILURE;
	}

	/*
	 * We are passing the physical base address as the third argument
	 * because the physical and virtual base address are the same in our
	 * example. For systems that support virtual memory, the third
	 * argument needs to be the virtual base address.
	 */
	Status = XGpio_CfgInitialize(&Gpio,
					GpioConfigPtr,
					GpioConfigPtr->BaseAddress);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XGpio_SetDataDirection(&Gpio, BUTTON_CHANNEL, GPIO_ALL_BUTTONS);

	/*
	 * Initialize the USB instance as required for the USB mouse
	 * application.
	 */
	InitUsbInterface(&UsbInstance);

	/*
	 * Set USB device address to 0 which is the unenumerated state.
	 */
	Status = XUsb_SetDeviceAddress(&UsbInstance, 0);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/*
	 * Setup the interrupt handlers.
	 */
	XUsb_IntrSetHandler(&UsbInstance, (void *)UsbIfIntrHandler,
				&UsbInstance);

	XUsb_EpSetHandler(&UsbInstance, 0,
				(XUsb_EpHandlerFunc *)Ep0IntrHandler,
				&UsbInstance);

	XUsb_EpSetHandler(&UsbInstance, 1,
				(XUsb_EpHandlerFunc *)Ep1IntrHandler,
				&UsbInstance);

	/*
	 * Setup the interrupt system.
	 */
	Status = SetupInterruptSystem(&UsbInstance, &Gpio);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupts.
	 */
	XUsb_IntrEnable(&UsbInstance, XUSB_STATUS_GLOBAL_INTR_MASK |
				XUSB_STATUS_RESET_MASK |
				XUSB_STATUS_SUSPEND_MASK |
				XUSB_STATUS_DISCONNECT_MASK |
				XUSB_STATUS_FIFO_BUFF_RDY_MASK |
				XUSB_STATUS_FIFO_BUFF_FREE_MASK |
				XUSB_STATUS_EP0_BUFF1_COMP_MASK |
				XUSB_STATUS_EP1_BUFF1_COMP_MASK |
				XUSB_STATUS_EP1_BUFF2_COMP_MASK );

	XUsb_Start(&UsbInstance);

	/*
	 * Set the device configuration to unenumerated state.
	 */
	UsbInstance.DeviceConfig.CurrentConfiguration = 0;

	/*
	 * Observe that the mouse movement is seen on the PC whenever any of the
	 * ML403 evaluation board push button is pressed. The test ends when the
	 * center push button on the ML403 Evaluation board is pressed.
	 */
	while (!StopTest);


	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This is the USB initialization function. This example initializes the USB
 * device for USB mouse example. The following configuration is done.
 *	- EP0 : CONTROL end point, Bidirectional, Packet size 64 bytes.
 *	- EP1 : NON_ISOCHRONOUS, INTERRUPT_IN, packet size of 16 bytes.
 *
 * @param	InstancePtr is a pointer to the XUsb instance.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void InitUsbInterface(XUsb *InstancePtr)
{

	XUsb_DeviceConfig	DeviceConfig;

	/*
	 * Setup Endpoint 0.
	 */
	DeviceConfig.Ep[0].RamBase = 0x22;
	DeviceConfig.Ep[0].Size	= 0x40;
	DeviceConfig.Ep[0].EpType = 0;
	DeviceConfig.Ep[0].OutIn = XUSB_EP_DIRECTION_OUT;


	/*
	 * Setup EP 1.
	 */
	DeviceConfig.Ep[1].RamBase = 0x1000;
	DeviceConfig.Ep[1].Size = 0x10;
	DeviceConfig.Ep[1].EpType = 0;
	DeviceConfig.Ep[1].OutIn = XUSB_EP_DIRECTION_IN;

	InstancePtr->DeviceConfig.NumEndpoints = 2;
	DeviceConfig.NumEndpoints = 2;

	/*
	 * Initialize the device configuration.
	 */
	XUsb_ConfigureDevice(InstancePtr, &DeviceConfig);

	XUsb_EpEnable(InstancePtr, 0);
	XUsb_EpEnable(InstancePtr, 1);

	MaxControlSize = 64;

	/*
	 * Store the actual RAM address offset in the device structure, so as to
	 * avoid the multiplication during processing.
	 */
	InstancePtr->DeviceConfig.Ep[1].RamBase <<= 2 ;

}

/*****************************************************************************/
/**
 * This function is the interrupt handler for the USB reset, suspend and
 * disconnect events.
 *
 * @param	CallBackRef is the callback reference passed from the interrupt
 *		handler, which in our case is a pointer to the driver instance.
 * @param	IntrStatus is a bit mask indicating pending interrupts.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void UsbIfIntrHandler(void *CallBackRef, u32 IntrStatus)
{

	XUsb *InstancePtr;
	u8 Index;

	InstancePtr = (XUsb *) CallBackRef;

	if (IntrStatus & XUSB_STATUS_RESET_MASK) {

			XUsb_Stop(InstancePtr);
			InstancePtr->DeviceConfig.CurrentConfiguration = 0;
			InstancePtr->DeviceConfig.Status = XUSB_RESET;
			for (Index = 0; Index < 3; Index++) {
				XUsb_WriteReg(InstancePtr->Config.BaseAddress,
					       InstancePtr->
					       EndPointOffset[Index], 0);
			}
			/*
			 * Re-initialize the device and set the device address
			 * to 0 and re-start the device.
			 */
			InitUsbInterface(InstancePtr);
			XUsb_SetDeviceAddress(InstancePtr, 0);
			XUsb_Start(InstancePtr);

		XUsb_IntrDisable(InstancePtr, XUSB_STATUS_RESET_MASK);
		XUsb_IntrEnable(InstancePtr, (XUSB_STATUS_DISCONNECT_MASK |
					      XUSB_STATUS_SUSPEND_MASK));
	}
	if (IntrStatus & XUSB_STATUS_SUSPEND_MASK) {
		/*
		 * Process the suspend event.
		 */
		XUsb_IntrDisable(InstancePtr, XUSB_STATUS_SUSPEND_MASK);
		XUsb_IntrEnable(InstancePtr, (XUSB_STATUS_RESET_MASK |
					      XUSB_STATUS_DISCONNECT_MASK));
	}

}

/*****************************************************************************/
/**
 * This function is the interrupt handler for the USB End point Zero events.
 *
 *
 * @param	CallBackRef is the callback reference passed from the interrupt.
 *		handler, which in our case is a pointer to the driver instance.
 * @param	EpNum is the end point number.
 * @param	IntrStatus is a bit mask indicating pending interrupts.
 *
 * @return	None.
 *
 * @note	EpNum is not used in this function as the handler is attached
 *		specific to end point zero. This parameter is useful when a
 *		single handler is used for processing all end point interrupts.
 *
 ******************************************************************************/
void Ep0IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus)
{

	XUsb *InstancePtr;
	int SetupRequest;

	InstancePtr = (XUsb *)CallBackRef;

	/*
	 * Process the end point zero buffer interrupt.
	 */
	if (IntrStatus & XUSB_BUFFREADY_EP0_BUFF_MASK){
		if (IntrStatus & XUSB_STATUS_SETUP_PACKET_MASK) {
			/*
			 * Received a setup packet. Execute the chapter 9
			 * command.
			 */
			XUsb_IntrEnable(InstancePtr,
					(XUSB_STATUS_DISCONNECT_MASK |
					XUSB_STATUS_SUSPEND_MASK |
					XUSB_STATUS_RESET_MASK));
			SetupRequest = Chapter9(InstancePtr);
			if (SetupRequest != XST_SUCCESS) {
				switch(SetupRequest){
				case 0x9:
					break;
				case 0x10:
					break;
				/*
				 * Unsupported command. Stall
				 * the end point.
				 */
				 default:
					XUsb_EpStall(InstancePtr, 0);
				break;
				}
			}
		} else if (IntrStatus & XUSB_STATUS_FIFO_BUFF_RDY_MASK) {
				EP0ProcessOutToken(InstancePtr);
		} else if (IntrStatus & XUSB_STATUS_FIFO_BUFF_FREE_MASK) {
				EP0ProcessInToken(InstancePtr);
		}
	}
}

/*****************************************************************************/
/**
 * This function is the interrupt handler for the USB End point one events.
 *
 * @param	CallBackRef is the callback reference passed from the interrupt
 *		handler, which in our case is a pointer to the driver instance.
 * @param	EpNum is the end point number.
 * @param	IntrStatus is a bit mask indicating pending interrupts.
 *
 * @return	None.
 *
 * @note	EpNum is not used in this function as the handler is attached
 *		specific to end point one. This parameter is useful when a
 *		single handler is used for processing all end point interrupts.
 *
 ******************************************************************************/
void Ep1IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus)
{

	XUsb *InstancePtr;

	InstancePtr = (XUsb *)CallBackRef;

	u32 EpReg;

	/*
	 * Process the End point 1 interrupts.
	 */
	if (IntrStatus & XUSB_BUFFREADY_EP1_BUFF1_MASK) {
		EpReg = XUsb_ReadReg(InstancePtr->Config.BaseAddress,
		(InstancePtr->EndPointOffset[1] + XUSB_EP_BUF0COUNT_OFFSET));
		InstancePtr->DeviceConfig.Ep[1].Buffer0Count = EpReg;
		InstancePtr->DeviceConfig.Ep[1].Buffer0Ready = 0;
	}

	if (IntrStatus & XUSB_BUFFREADY_EP1_BUFF2_MASK) {
		EpReg = XUsb_ReadReg(InstancePtr->Config.BaseAddress,
		(InstancePtr->EndPointOffset[1] + XUSB_EP_BUF1COUNT_OFFSET));
		InstancePtr->DeviceConfig.Ep[1].Buffer1Count = EpReg;
		InstancePtr->DeviceConfig.Ep[1].Buffer1Ready = 0;
	}

}

/****************************************************************************/
/**
* This function is the Interrupt Service Routine for the GPIO device.
*
* This function will detect the push button on the board has changed state
* and then prepare data to be sent to the host upon receiving the Get
*
* @param	InstancePtr is the GPIO component to operate on. It is a void
*		pointer to meet the interface of an interrupt processing
* 		function.
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void GpioIsr(void *InstancePtr)
{
	XGpio *GpioPtr = (XGpio *)InstancePtr;
	u32 Buttons;
	u32 ButtonsChanged = 0;
	static u32 PreviousButtons;
	u8 Index = 0;
	static u8 State = 0;
	const u8 Position[] = {-4, -4, -4, 0, 4, 4, 4, 0, -4, -4};
	u8 TxBuf[4];


	/*
	 * Disable the interrupt
	 */
	XGpio_InterruptDisable(GpioPtr, BUTTON_INTERRUPT);


	/*
	 * There should not be any other interrupts occurring other than the
	 * the button changes.
	 */
	if ((XGpio_InterruptGetStatus(GpioPtr) & BUTTON_INTERRUPT) !=
		BUTTON_INTERRUPT) {
		return;
	}

	/*
	 * Read state of push buttons and determine which ones changed
	 * states from the previous interrupt. Save a copy of the buttons
	 * for the next interrupt.
	 */
	Buttons = (XGpio_DiscreteRead(GpioPtr, BUTTON_CHANNEL) & 0x1F) ;
	ButtonsChanged = Buttons ^ PreviousButtons;
	PreviousButtons = Buttons;

	/*
	 * Handle all button state changes that occurred since the last
	 * interrupt
	 */
	while (ButtonsChanged != 0) {

		/*
		 * Determine which button changed state and then get
		 * the current state of the associated LED
		 */
		if (ButtonsChanged & 0x1F){

				if (ButtonsChanged & EXIT_BUTTON){
					StopTest = TRUE;
					break;
				}
				TxBuf[1] = Position [State];
				TxBuf[2] = Position [State+2];
				++State;
				for (Index =0; Index < 5; Index++){
					XUsb_EpDataSend(&UsbInstance, 1,
						(unsigned char *)&TxBuf[0], 4);
				}

				if (State > 7)
					State = 0;
		}
		break;
	}

	/*
	 * Clear the interrupt such that it is no longer pending in the GPIO
	 */
	(void)XGpio_InterruptClear(GpioPtr, BUTTON_INTERRUPT);

	/*
	 * Enable the interrupt
	 */
	XGpio_InterruptEnable(GpioPtr, BUTTON_INTERRUPT);

}

/******************************************************************************/
/**
*
* This function sets up the interrupt system such that interrupts can occur
* for the USB and GPIO
*
* @param	Intc is the pointer to the Interrupt controller instance.
* @param	UsbInstancePtr is a pointer to the USB device instance.
* @param	Gpio is pointer to the GPIO instance.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE. if it fails.
*
* @note		None.
*
*******************************************************************************/
static int SetupInterruptSystem(XUsb *UsbInstancePtr, XGpio *Gpio)
{
	int Status;

	/*
	 * Initialize the interrupt controller driver.
	 */
	Status = XIntc_Initialize(&Intc, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS){
		return XST_FAILURE;
	}


	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the USB device occurs.
	 */
	Status = XIntc_Connect(&Intc, USB_INTR,
			    (XInterruptHandler)XUsb_IntrHandler,
			    (void *)UsbInstancePtr);
	if (Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the GPIO device occurs.
	 */
	XIntc_Connect(&Intc, INTC_GPIO_INTERRUPT_ID,
			(XInterruptHandler)GpioIsr,(void *) Gpio);

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the USB can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(&Intc, XIN_REAL_MODE);
	if (Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	/*
	 * Enable the GPIO channel interrupts so that push button can be
	 * detected and enable interrupts for the GPIO device
	 */
	XGpio_InterruptEnable(Gpio, BUTTON_INTERRUPT);
	XGpio_InterruptGlobalEnable(Gpio);

	/*
	 * Enable the interrupt for GPIO
	 */
	XIntc_Enable(&Intc, INTC_GPIO_INTERRUPT_ID);

	/*
	 * Enable the interrupt for the USB.
	 */
	XIntc_Enable(&Intc, USB_INTR);

	/*
	 * Initialize the exception table
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XIntc_InterruptHandler,
				&Intc);

	/*
	 * Enable non-critical exceptions
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

