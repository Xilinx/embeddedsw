/******************************************************************************
*
* Copyright (C) 2002 - 2014 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
/*****************************************************************************/
/**
* @file xgpio_intr_example.c
*
* This file contains a design example using the GPIO driver (XGpio) in an
* interrupt driven mode of operation. This example does assume that there is
* an interrupt controller in the hardware system and the GPIO device is
* connected to the interrupt controller.
*
* This example is designed to work on the Xilinx ML300 board using the PowerPC
* 405 processor present in the VirtexIIPro device. The example uses the
* interrupt capability of the GPIO to detect push button presses and control the
* LEDs on the board. When a button is pressed it will turn on and LED located
* closest to it. When the button is released it will turn off the LED.
* This examples uses two channels of a GPIO such that it is necessary to have
* dual channel capabilities.
*
* The buttons and LEDs are on 2 seperate channels of the GPIO so that interrupts
* are not caused when the LEDs are turned on and off.
*
* At the start of execution all LEDs will be turned on, then each one by itself,
* and then all on again followed by all turned off. After this sequence, button
* presses are processed by interrupts.
*
* The following snippet from the UCF file of the hardware build indicates the
* way the GPIO channels are connected to the ML300 for the LEDs and buttons.
*
*<pre>
* Net LEDs_Push_Buttons_GPIO_IO<0> LOC=C4;
* Net LEDs_Push_Buttons_GPIO_IO<1> LOC=L8;
* Net LEDs_Push_Buttons_GPIO_IO<2> LOC=F8;
* Net LEDs_Push_Buttons_GPIO_IO<3> LOC=J7;
* Net LEDs_Push_Buttons_GPIO_IO<4> LOC=K7;
* Net LEDs_Push_Buttons_GPIO_IO<5> LOC=E7;
* Net LEDs_Push_Buttons_GPIO_IO<6> LOC=D3;
* Net LEDs_Push_Buttons_GPIO_IO<7> LOC=C6;
* Net LEDs_Push_Buttons_GPIO_IO<8> LOC=E8;
* Net LEDs_Push_Buttons_GPIO_IO<9> LOC=B3;
* Net LEDs_Push_Buttons_GPIO_IO<10> LOC=E9;
* Net LEDs_Push_Buttons_GPIO_IO<11> LOC=G9;
* Net LEDs_Push_Buttons_GPIO_IO<12> LOC=A3;
* Net LEDs_Push_Buttons_GPIO_IO<13> LOC=F9;
* Net LEDs_Push_Buttons_GPIO_IO<14> LOC=D6;
* Net LEDs_Push_Buttons_GPIO_IO<15> LOC=G10;
* Net LEDs_Push_Buttons_GPIO2_IO<0> LOC=G6;
* Net LEDs_Push_Buttons_GPIO2_IO<1> LOC=L7;
* Net LEDs_Push_Buttons_GPIO2_IO<2> LOC=G5;
* Net LEDs_Push_Buttons_GPIO2_IO<3> LOC=M8;
* Net LEDs_Push_Buttons_GPIO2_IO<4> LOC=H6;
* Net LEDs_Push_Buttons_GPIO2_IO<5> LOC=M7;
* Net LEDs_Push_Buttons_GPIO2_IO<6> LOC=H5;
* Net LEDs_Push_Buttons_GPIO2_IO<7> LOC=N8;
* Net LEDs_Push_Buttons_GPIO2_IO<8> LOC=J6;
* Net LEDs_Push_Buttons_GPIO2_IO<9> LOC=M5;
* Net LEDs_Push_Buttons_GPIO2_IO<10> LOC=J5;
* Net LEDs_Push_Buttons_GPIO2_IO<11> LOC=M2;
* Net LEDs_Push_Buttons_GPIO2_IO<12> LOC=K6;
* Net LEDs_Push_Buttons_GPIO2_IO<13> LOC=M1;
* Net LEDs_Push_Buttons_GPIO2_IO<14> LOC=K5;
* Net LEDs_Push_Buttons_GPIO2_IO<15> LOC=P6;
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 2.00a jhl  12/01/03 First release
* 2.00a sv   04/15/05 Minor changes to comply to Doxygen and coding guidelines
* 3.00a ktn  11/21/09 Updated to use HAL Processor APIs and minior changes
*		      as per coding guidelines.
* 3.00a sdm  02/16/11 Updated to support ARM Generic Interrupt Controller
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xgpio.h"
#include "xil_exception.h"

#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the names of the hardware instances that
 * were created in the EDK XPS system.  They are only defined here such that
 * a user can easily change all the needed device IDs in one place.
 */
#define GPIO_DEVICE_ID		XPAR_PUSH_BUTTONS_4BITS_DEVICE_ID
#define INTC_GPIO_INTERRUPT_ID	XPAR_INTC_0_GPIO_2_VEC_ID

#ifdef XPAR_INTC_0_DEVICE_ID
 #define INTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID
 #define INTC		XIntc
 #define INTC_HANDLER	XIntc_InterruptHandler
#else
 #define INTC_DEVICE_ID	XPAR_SCUGIC_SINGLE_DEVICE_ID
 #define INTC		XScuGic
 #define INTC_HANDLER	XScuGic_InterruptHandler
#endif

/*
 * The following constants define the positions of the buttons and LEDs each
 * channel of the GPIO
 */
#define GPIO_ALL_LEDS		0xFFFF
#define GPIO_ALL_BUTTONS	0xFFFF

/*
 * The following constants define the GPIO channel that is used for the buttons
 * and the LEDs. They allow the channels to be reversed easily.
 */
#define BUTTON_CHANNEL	 1	/* Channel 1 of the GPIO Device */
#define LED_CHANNEL	 2	/* Channel 2 of the GPIO Device */
#define BUTTON_INTERRUPT XGPIO_IR_CH1_MASK  /* Channel 1 Interrupt Mask */


/*
 * The following constant is used to wait after an LED is turned on to make
 * sure that it is visible to the human eye.  This constant might need to be
 * tuned for faster or slower processor speeds.
 */
#define LED_DELAY	 1000000

/**************************** Type Definitions *******************************/

typedef struct
{
	u32 ButtonMask;	 /* The bit corresponding to the button */
	u32 LedMask;	 /* The bit corresponding to the LED */
} MapButtonTable;

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int MapButton2Led(u32 Buttons, u32 *ButtonFoundPtr);

void SequenceLeds();

void GpioIsr(void *InstancePtr);

int SetupInterruptSystem();

/************************** Variable Definitions *****************************/

/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger
 */
static XGpio Gpio; /* The Instance of the GPIO Driver */

static INTC Intc; /* The Instance of the Interrupt Controller Driver */

volatile int InterruptCount; /* Count of interrupts that have occured */


/*
 * The following table contains the masks for the buttons and LEDS
 * that are connected on the board. It's purpose is to map a button
 * to a specific LED.
 */
 MapButtonTable Button2LedTable[] =
	{ { 0x1, 0x1 },
	  { 0x2, 0x2 },
	  { 0x4, 0x4 },
	  { 0x8, 0x8 },
	  { 0x10, 0x8 },
	  { 0x20, 0x8 },
	  { 0x40, 0x8 },
	  { 0x100, 0x10 },
	  { 0x200, 0x20 },
	  { 0x400, 0x40 },
	  { 0x800, 0x80 },
	  { 0x1000, 0x80 },
	  { 0x2000, 0x80 },
	  { 0x4000, 0x80 } };

/****************************************************************************/
/**
* This function is the main function of the GPIO example.  It is responsible
* for initializing the GPIO device, setting up interrupts and providing a
* foreground loop such that interrupt can occur in the background.
*
* @param	None.
*
* @return
*		- XST_SUCCESS to indicate success.
*		- XST_FAILURE to indicate Failure.
*
* @note		None.
*
*
*****************************************************************************/
int main(void)
{
	int Status;

	/* Initialize the GPIO driver. If an error occurs then exit */

	Status = XGpio_Initialize(&Gpio, GPIO_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test on the GPIO.  This is a minimal test and only
	 * verifies that there is not any bus error when reading the data
	 * register
	 */
	XGpio_SelfTest(&Gpio);

	/*
	 * Setup direction register so the switch is an input and the LED is
	 * an output of the GPIO
	 */
	XGpio_SetDataDirection(&Gpio, BUTTON_CHANNEL, GPIO_ALL_BUTTONS);
	XGpio_SetDataDirection(&Gpio, LED_CHANNEL, ~GPIO_ALL_LEDS);

	/* Sequence the LEDs to show this example is starting to run */

	SequenceLeds();

	/*
	 * Setup the interrupts such that interrupt processing can occur. If
	 * an error occurs then exit
	 */
	Status = SetupInterruptSystem();
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Loop forever while the button changes are handled by the interrupt
	 * level processing
	 */
	while (1) {
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* This function sequences the LEDs by turning them all on, the turning each
* one on individually, then turning them all on, and finally off.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void SequenceLeds()
{
	u32 Mask = 0x8000;
	int Led;
	volatile int Delay;

	/* Turn on all the LEDS to show starting the sequence */

	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, GPIO_ALL_LEDS);
	for (Delay = 0; Delay < LED_DELAY; Delay++);

	/* Sequence thru turning each LED on one at a time */

	for (Led = 1; Led <= 16; Led++) {
		XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, Mask);
		Mask >>= 1;

		/* Wait a small amount of time so the LED is visible */

		for (Delay = 0; Delay < LED_DELAY; Delay++);
	}

	/* Turn on all LEDS to show stopping the sequence */

	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, GPIO_ALL_LEDS);
	for (Delay = 0; Delay < LED_DELAY; Delay++);

	/* Turn off all the LEDs */

	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0);

	XGpio_InterruptClear(&Gpio, XGPIO_IR_MASK);
}

/****************************************************************************/
/**
* This function maps each button on the board to an LED.
*
* @param	Buttons contains the buttons that have changed.
* @param	ButtonFoundPtr is a pointer to allow this function to indicate
*		the button that was associated with the returned LED. This
*		input is needed to allow muliple buttons to change
*		simulataneously.
*
* @return
*
* The Led that is associated with the first button that was found to be
* changed.  A value of zero indicates no LED was found.
*
* @note		None.
*
*****************************************************************************/
int MapButton2Led(u32 Buttons, u32 *ButtonFoundPtr)
{
	int Index;

	/* Look thru the table to map the button to an LED */

	for (Index = 0; Index < sizeof(Button2LedTable)/ sizeof(MapButtonTable);
		Index++) {

		/*
		 * Determine which LED corresponds to the button being careful
		 * because more than one button could have changed
		 */
		if (Button2LedTable[Index].ButtonMask ==
			(Buttons & Button2LedTable[Index].ButtonMask)) {
			/*
			 * If the button was found then return the
			 * associated LED
			 */
			*ButtonFoundPtr = Button2LedTable[Index].ButtonMask;
			return Button2LedTable[Index].LedMask;
		}
	}

	/* If no button was found in the table, then indicate no LED */

	return 0;
}

/****************************************************************************/
/**
* This function is the Interrupt Service Routine for the GPIO device.  It
* will be called by the processor whenever an interrupt is asserted by the
* device.
*
* This function will detect the push button on the board has changed state
* and then turn on or off the LED.
*
* @param	InstancePtr is the GPIO instance pointer to operate on.
*		It is a void pointer to meet the interface of an interrupt
*		processing function.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void GpioIsr(void *InstancePtr)
{
	XGpio *GpioPtr = (XGpio *)InstancePtr;
	u32 Led;
	u32 LedState;
	u32 Buttons;
	u32 ButtonFound;
	u32 ButtonsChanged = 0;
	static u32 PreviousButtons;

	/*
	 * Disable the interrupt
	 */
	XGpio_InterruptDisable(GpioPtr, BUTTON_INTERRUPT);

	/* Keep track of the number of interrupts that occur */

	InterruptCount++;

	/*
	 * There should not be any other interrupts occuring other than the
	 * the button changes
	 */
	if ((XGpio_InterruptGetStatus(GpioPtr) & BUTTON_INTERRUPT) !=
		BUTTON_INTERRUPT) {
		return;
	}


	/*
	 * Read state of push buttons and determine which ones changed
	 * states from the previous interrupt. Save a copy of the buttons
	 * for the next interrupt
	 */
	Buttons = XGpio_DiscreteRead(GpioPtr, BUTTON_CHANNEL);
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
		 Led = MapButton2Led(ButtonsChanged, &ButtonFound);
		 LedState = XGpio_DiscreteRead(GpioPtr, LED_CHANNEL) & Led;

		 /*
		  * Clear the button that is being processed so that it is
		  * done and others can be handled also
		  */
		 ButtonsChanged &= ~ButtonFound;

		 /* Toggle the state of the LED */
		 if (LedState) {
			 XGpio_DiscreteClear(GpioPtr, LED_CHANNEL, Led);
		 } else {
			 XGpio_DiscreteSet(GpioPtr, LED_CHANNEL, Led);
		 }
	 }

	 /* Clear the interrupt such that it is no longer pending in the GPIO */

	 (void)XGpio_InterruptClear(GpioPtr, BUTTON_INTERRUPT);

	 /*
	  * Enable the interrupt
	  */
	 XGpio_InterruptEnable(GpioPtr, BUTTON_INTERRUPT);

}

/****************************************************************************/
/**
* This function sets up the interrupt system for the example.  The processing
* contained in this funtion assumes the hardware system was built with
* and interrupt controller.
*
* @param	None.
*
* @return	A status indicating XST_SUCCESS or a value that is contained in
*		xstatus.h.
*
* @note		None.
*
*****************************************************************************/
int SetupInterruptSystem()
{
	int Result;
	INTC *IntcInstancePtr = &Intc;

#ifdef XPAR_INTC_0_DEVICE_ID
	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 * specify the device ID that was generated in xparameters.h
	 */
	Result = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Result != XST_SUCCESS) {
		return Result;
	}

	/* Hook up interrupt service routine */
	XIntc_Connect(IntcInstancePtr, INTC_GPIO_INTERRUPT_ID,
		      (Xil_ExceptionHandler)GpioIsr, &Gpio);

	/* Enable the interrupt vector at the interrupt controller */

	XIntc_Enable(IntcInstancePtr, INTC_GPIO_INTERRUPT_ID);

	/*
	 * Start the interrupt controller such that interrupts are recognized
	 * and handled by the processor
	 */
	Result = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Result != XST_SUCCESS) {
		return Result;
	}

#else
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Result = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, INTC_GPIO_INTERRUPT_ID,
					0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Result = XScuGic_Connect(IntcInstancePtr, INTC_GPIO_INTERRUPT_ID,
				 (Xil_ExceptionHandler)GpioIsr, &Gpio);
	if (Result != XST_SUCCESS) {
		return Result;
	}

	/*
	 * Enable the interrupt for the GPIO device.
	 */
	XScuGic_Enable(IntcInstancePtr, INTC_GPIO_INTERRUPT_ID);
#endif

	/*
	 * Enable the GPIO channel interrupts so that push button can be
	 * detected and enable interrupts for the GPIO device
	 */
	XGpio_InterruptEnable(&Gpio, BUTTON_INTERRUPT);
	XGpio_InterruptGlobalEnable(&Gpio);

	/*
	 * Initialize the exception table and register the interrupt
	 * controller handler with the exception table
	 */
	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 (Xil_ExceptionHandler)INTC_HANDLER, IntcInstancePtr);

	/* Enable non-critical exceptions */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
