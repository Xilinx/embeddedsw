/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xusb_keyboard.c
 *
 * This file contains an example for USB keyboard.
 *
 * @note
 *		- The example is tested on ML403 and ML507 evaluation boards.
 *		  The push buttons SW3,SW4,SW5 and SW7 on the ML403 evaluation
 *		  board are used in the example for the key action of the USB
 *		  keyboard.The push buttons SW10, SW11, SW12, SW13 and SW14
 *		  are used on the ML507 board.
 *		  The example sends a character from a fixed sequence of
 *		  characters from the device as and when any of the SW3 or SW4
 *		  or SW5 and SW7 push button is pressed. The fixed sequence of
 *		  characters is	XILINX USB KEYBOARD DEMO. The SW6 push button
 *		  switch is used to complete the test. SW14 is used on ML507
 *		  board to complete the test.
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
 *		- The USB keyboard example code has to be compiled along with
 *		  the xusb_cp9.c file. The xusb_cp9.c file contains all the USB
 *		  enumeration related functions. To compile the code for USB
 *		  keyboard example, the constant definitions HID_DEVICES
 *		  and USB_KEYBOARD are to be defined and the definitions the
 *		  constants USB_MOUSE and MASS_STORAGE_DEVICE are to be
 *		  undefined. These definitions can be found in the xusb_types.h
 *		  file.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -----------------------------------------------------------------
 * 1.00a hvm  5/31/07 First release
 * 3.00a hvm  11/18/09 Updated to use HAL processor APIs.
 *		       XUsb_mReadReg is renamed to XUsb_ReadReg and
 *		       XUsb_mWriteReg is renamed to XUsb_WriteReg.
 * 4.00a hvm  08/11/11 Updated the code in gpio isr to increment the index by 4 as
 *			a dummy byte is added in the Message variable in keyboard.h
 *			file to	handle the address alignment issue.
 * 4.02a bss  11/01/11 Modified UsbIfIntrHandler function to unconditionally
 *			reset when USB reset is asserted (CR 627574).
 *
 * </pre>
 *****************************************************************************/
/***************************** Include Files *********************************/

#include "xusb.h"
#include "xintc.h"
#include "xusb_keyboard.h"
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


/************************** Function Prototypes ******************************/

int UsbKbdExample(u16 UsbId, u16 GpioId);
static int SetupInterruptSystem(XUsb *UsbPtr, XGpio *GpioPtr);

/************************** Variable Definitions *****************************/

static XUsb UsbInstance;		/* The instance of the USB device */
static XGpio Gpio; /* The Instance of the GPIO Driver */

XUsb_Config *UsbConfigPtr;	/* Pointer to the USB config structure */
XGpio_Config *GpioConfigPtr;	/* Pointer to the GPIO config structure */

XIntc InterruptController;	/* Instance of the Interrupt Controller */
volatile int StopTest = FALSE;

int MaxMsgLength;

/****************************************************************************/
/**
* This function is the main function of the USB Keyboard example.
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

	Status = UsbKbdExample(USB_DEVICE_ID, GPIO_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
 * This function implements the USB Keyboard example.
 * The Key board action can be seen on the PC as and when any of the push
 * buttons SW3 or SW4 or SW5 or SW7 on the ML403 evaluation board is pressed.
 *
 * @param	UsbId is the USB device id.
 * @param	GpioId is the GPIO device id.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if test fails.
 * @note	None.
 *
 *****************************************************************************/
int UsbKbdExample(u16 UsbId, u16 GpioId)
{
	int Status;



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


	/*
	 * Initialize the USB driver.
	 */
	UsbConfigPtr = XUsb_LookupConfig(UsbId);
	if (UsbConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XUsb_CfgInitialize(&UsbInstance,
					UsbConfigPtr,
					UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the USB instance as required for the keyboard
	 * example.
	 */
	InitUsbInterface(&UsbInstance);

	XGpio_SetDataDirection(&Gpio, BUTTON_CHANNEL, GPIO_ALL_BUTTONS);
	MaxMsgLength = sizeof(Message);

	/*
	 * Set our function address to 0 which is the unenumerated state.
	 */
	Status = XUsb_SetDeviceAddress(&UsbInstance, 0);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/*
	 * Setup the interrupt handlers.
	 */
	XUsb_IntrSetHandler(&UsbInstance, (void *) UsbIfIntrHandler,
			    &UsbInstance);

	XUsb_EpSetHandler(&UsbInstance, 0,
			  (XUsb_EpHandlerFunc *) Ep0IntrHandler, &UsbInstance);

	XUsb_EpSetHandler(&UsbInstance, 1,
			  (XUsb_EpHandlerFunc *) Ep1IntrHandler, &UsbInstance);

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


	while (StopTest == FALSE){
			/*
			 * Stop the test if the Stop key is pressed
			 */
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function initializes the USB device for Keyboard example. The following
 * is the configuration.
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
void InitUsbInterface(XUsb * InstancePtr)
{

	XUsb_DeviceConfig DeviceConfig;

	/*
	 * Setup Endpoint 0.
	 */
	DeviceConfig.Ep[0].RamBase = 0x22;
	DeviceConfig.Ep[0].Size = 0x40;
	DeviceConfig.Ep[0].EpType = 0;
	DeviceConfig.Ep[0].OutIn = XUSB_EP_DIRECTION_OUT;


	/*
	 * Setup Endpoint 1.
	 */
	DeviceConfig.Ep[1].RamBase = 0x1000;
	DeviceConfig.Ep[1].Size = 0x10;
	DeviceConfig.Ep[1].EpType = 0;
	DeviceConfig.Ep[1].OutIn = XUSB_EP_DIRECTION_IN;

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
	InstancePtr->DeviceConfig.Ep[1].RamBase <<= 2;

}

/*****************************************************************************/
/**
 * This function is the interrupt handler for the USB keyboard example
 *
 * @param	CallBackRef is the callback reference passed from the interrupt
 *		handler, which in our case is a pointer to the driver instance.
 * @param	IntrStatus is a bit mask indicating pending interrupts.
 *
 * @return	None.
 *
 * @note	Each interrupt source is disabled upon reception. This is to
 *		avoid the repetitive occurrence of the same event. This is done
 *		because these event conditions exist for few milliseconds.
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

	InstancePtr = (XUsb *) CallBackRef;

	/*
	 * Process the end point zero buffer interrupt.
	 */
	if (IntrStatus & XUSB_BUFFREADY_EP0_BUFF_MASK) {
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
		}
		else if (IntrStatus & XUSB_STATUS_FIFO_BUFF_RDY_MASK) {
			EP0ProcessOutToken(InstancePtr);
		}
		else if (IntrStatus & XUSB_STATUS_FIFO_BUFF_FREE_MASK) {
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

	InstancePtr = (XUsb *) CallBackRef;

	/*
	 * Process the End point 1 interrupts.
	 */
	if (IntrStatus & XUSB_BUFFREADY_EP1_BUFF1_MASK) {
		InstancePtr->DeviceConfig.Ep[1].Buffer0Ready = 0;
	}

	if (IntrStatus & XUSB_BUFFREADY_EP1_BUFF2_MASK) {
		InstancePtr->DeviceConfig.Ep[1].Buffer1Ready = 0;
	}

}

/****************************************************************************/
/**
* This function is the Interrupt Service Routine for the GPIO device.
*
* This function will detect the push button on the board has changed state
* and then prepare data to be sent to the host.
*
* @param	InstancePtr is the GPIO component to operate on. It is a void
*		pointer and in this case will be a pointer to the GPIO
*		instance.
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
	int Status;
	static int Index = 0;
	u8 NoKeyData[3]= {0,0,0};


	/*
	 * Disable the GPIO interrupt.
	 */
	XGpio_InterruptDisable(GpioPtr, BUTTON_INTERRUPT);


	/*
	 * There should not be any other GPIO interrupts occurring other than
	 * the the button changes.
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
	 * interrupt.
	 */
	while (ButtonsChanged != 0) {
		/*
		 * Determine which button changed state and then get
		 * the current state of the associated LED.
		 */
		if (ButtonsChanged & 0x1F){

			if (ButtonsChanged & EXIT_BUTTON){
				StopTest = TRUE;
				break;
			}

			if (Index >= MaxMsgLength){
				Index = 0;
			}
			XUsb_EpDataSend(&UsbInstance, 1,
			(unsigned char *)&Message[Index], 3);

			/*
			 * Send no key status to PC.
			 */
			Status = XUsb_EpDataSend(&UsbInstance, 1,
				(unsigned char *)&NoKeyData[0], 3);
				if (Status == XST_SUCCESS) {
					Index += 4;
				}
		}
		break;
	}

	/*
	 * Clear the interrupt such that it is no longer pending in the GPIO.
	 */
	(void)XGpio_InterruptClear(GpioPtr, BUTTON_INTERRUPT);

	/*
	 * Enable the GPIO interrupt.
	 */
	XGpio_InterruptEnable(GpioPtr, BUTTON_INTERRUPT);

}

/******************************************************************************/
/**
*
* This function sets up the interrupt system such that interrupts can occur
* for the USB and GPIO. This function is application specific since the actual
* system may or may not have an interrupt controller. The USB and GPIO could be
* directly connected to a processor without an interrupt controller.  The
* user should modify this function to fit the application.
*
* @param	UsbPtr is a pointer to the instance of the USB device.
* @param	GpioPtr is a pointer to the instance of the GPIO device.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE. if it fails.
*
* @note		None.
*
*******************************************************************************/
static int SetupInterruptSystem(XUsb *UsbPtr, XGpio *GpioPtr)
{
	int Status;

	/*
	 * Initialize the interrupt controller driver.
	 */
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the USB device occurs.
	 */
	Status = XIntc_Connect(&InterruptController, USB_INTR,
				(XInterruptHandler) XUsb_IntrHandler,
				(void *) UsbPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the GPIO device occurs.
	 */
	XIntc_Connect(&InterruptController, INTC_GPIO_INTERRUPT_ID,
			(XInterruptHandler)GpioIsr,(void *) GpioPtr);


	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the USB and GPIO can cause interrupts through the interrupt
	 * controller.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the GPIO channel interrupts so that push button can be
	 * detected and enable interrupts for the GPIO device.
	 */
	XGpio_InterruptEnable(&Gpio, BUTTON_INTERRUPT);
	XGpio_InterruptGlobalEnable(GpioPtr);

	/*
	 * Enable the interrupt vector at the interrupt controller.
	 */
	XIntc_Enable(&InterruptController, INTC_GPIO_INTERRUPT_ID);

	/*
	 * Enable the interrupt for the USB.
	 */
	XIntc_Enable(&InterruptController, USB_INTR);

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XIntc_InterruptHandler,
				&InterruptController);

	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}


