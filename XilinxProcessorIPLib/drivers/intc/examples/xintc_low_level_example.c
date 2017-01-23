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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/******************************************************************************/
/**
*
* @file xintc_low_level_example.c
*
* This file contains a design example using the low level-0 driver, interface
* of the Interrupt Controller driver.
*
* This example shows the use of the Interrupt Controller both with a PowerPC
* and a MicroBlaze processor.
*
* @note
*		This example can also be used for Cascade mode interrupt
*		controllers by using the interrupt IDs generated in
*		xparameters.h. For Cascade mode, Interrupt IDs are generated
*		in xparameters.h as shown below:
*
*	    Master/Primary INTC
*		 ______
*		|      |-0      Secondary INTC
*		|      |-.         ______
*		|      |-.        |      |-32        Last INTC
*		|      |-.        |      |-.          ______
*		|______|<--31-----|      |-.         |      |-64
*			          |      |-.         |      |-.
*			          |______|<--63------|      |-.
*                                                    |      |-.
*                                                    |______|-95
*
*		All driver functions has to be called using BaseAddress
*		of Primary/Master Controller only. Driver functions takes
*		care of Slave Controllers based on Interrupt ID passed.
*		User must not use Interrupt source/ID  31 of Primary and
*		Secondary controllers to call driver functions.
*
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00c rpm  12/04/03 First release
* 1.00c sv   06/29/05 Minor changes to comply to Doxygen and coding guidelines
* 2.00a ktn  10/20/09 Updated to use HAL Processor APIs and _m is removed from
*		      all the macro names/definitions. Minor changes done as per
*		      coding guidelines.
* 3.6   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xintc_l.h"
#include "xil_exception.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define INTC_BASEADDR		XPAR_INTC_0_BASEADDR
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_INTR_ID	XPAR_INTC_0_UARTLITE_0_VEC_ID
#define INTC_DEVICE_INT_MASK	XPAR_RS232_UART_1_INTERRUPT_MASK


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int IntcLowLevelExample(u32 IntcBaseAddress);

void SetupInterruptSystem();

void DeviceDriverHandler(void *CallbackRef);


/************************** Variable Definitions *****************************/

/*
 * Create a shared variable to be used by the main thread of processing and
 * the interrupt processing
 */
volatile static int InterruptProcessed = FALSE;

/*****************************************************************************/
/**
*
* This is the main function for the Interrupt Controller Low Level example.
*
* @param	None.
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the low level example of Interrupt Controller, specify the Base
	 * Address generated in xparameters.h.
	 */
	Status = IntcLowLevelExample(INTC_BASEADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Intc lowlevel Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Intc lowlevel Example\r\n");
	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* This function is an example of how to use the interrupt controller driver
* component (XIntc) and the hardware device.  This function is designed to
* work without any hardware devices to cause interrupts. It may not return
* if the interrupt controller is not properly connected to the processor in
* either software or hardware.
*
* This function relies on the fact that the interrupt controller hardware
* has come out of the reset state such that it will allow interrupts to be
* simulated by the software.
*
* @param	IntcBaseAddress is Base Address of the the Interrupt Controller
*		Device.
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int IntcLowLevelExample(u32 IntcBaseAddress)
{

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
	XIntc_RegisterHandler(IntcBaseAddress, INTC_DEVICE_INTR_ID,
				(XInterruptHandler)DeviceDriverHandler,
				(void *)0);

	/*
	 * Enable interrupts for all devices that cause interrupts, and enable
	 * the INTC master enable bit.
	 */
	XIntc_EnableIntr(IntcBaseAddress, INTC_DEVICE_INT_MASK);


	/*
	 * Set the master enable bit. Note that we do not enable hardware
	 * interrupts yet since we want to simulate an interrupt from software
	 * down below.
	 */
	XIntc_Out32(IntcBaseAddress + XIN_MER_OFFSET, XIN_INT_MASTER_ENABLE_MASK);

	/*
	 * This step is processor specific, connect the handler for the
	 * interrupt controller to the interrupt source for the processor.
	 */
	SetupInterruptSystem();

	/*
	 * Cause (simulate) an interrupt so the handler will be called. This is
	 * done by writing a 1 to the interrupt status bit for the device
	 * interrupt.
	 */
	XIntc_Out32(IntcBaseAddress + XIN_ISR_OFFSET, INTC_DEVICE_INT_MASK);

	/*
	 * Wait for the interrupt to be processed, if the interrupt does not
	 * occur this loop will wait forever.
	 */
	while (1)
	{
		/*
		 * If the interrupt occurred which is indicated by the global
		 * variable which is set in the device driver handler, then
		 * stop waiting.
		 */
		if (InterruptProcessed) {
			break;
		}
	}


	return XST_SUCCESS;

}
/*****************************************************************************/
/**
*
* This function connects the interrupt handler of the interrupt controller to
* the processor. This function is seperate to allow it to be customized for
* each application. Each processor or RTOS may require unique processing to
* connect the interrupt handler.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void SetupInterruptSystem()
{
	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)XIntc_DeviceInterruptHandler,
			INTC_DEVICE_ID);

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

}



/*****************************************************************************/
/**
*
* This function is designed to look like an interrupt handler in a device
* driver. This is typically a 2nd level handler that is called from the
* interrupt controller interrupt handler.  This handler would typically
* perform device specific processing such as reading and writing the registers
* of the device to clear the interrupt condition and pass any data to an
* application using the device driver.
*
* @param	CallbackRef is passed back to the device driver's interrupt
*		handler by the XIntc driver.  It was given to the XIntc driver
*		in the XIntc_Connect() function call. It is typically a pointer
*		to the device driver instance variable if using the Xilinx Level
*		1 device drivers. In this example, we do not care about the
*		callback reference, so we passed it a 0 when connecting the
*		handler to the XIntc driver and we make no use of it here.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DeviceDriverHandler(void *CallbackRef)
{
	/*
	 * Indicate the interrupt has been processed using a shared variable.
	 */
	InterruptProcessed = TRUE;

}
