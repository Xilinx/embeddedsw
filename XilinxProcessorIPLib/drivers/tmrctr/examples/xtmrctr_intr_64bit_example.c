/******************************************************************************
*
* Copyright (C) 2011 - 2018 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
* @file  xtmrctr_intr_64bit_example.c
*
* This file contains a design example using the timer counter driver
* (XTmCtr) and hardware device using interrupt mode with the counters configured
* in  cascasde mode for a 64 bit operation. Both the timers should enabled in
* HW configuration for the cascade mode of operation.
*
* The cascade mode of operation is present in the new versions of the
* axi_timer IP. Please check the HW Datasheet to see whether this feature
* is present in the version of the IP that you are using.
*
* This example assumes that the interrupt controller is also present
*  as a part of the system.
*
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 2.04a sdm  07/15/11 Created based on the xtmrctr_intr_example
* 4.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 4.5   mus  07/05/18 Updated example to call TmrCtrDisableIntr function
*                     with correct arguments. Presently device id is
*                     being passed instead of interrupt id. It fixes
*                     CR#1006251.
* 4.5   mus  07/05/18 Fixed checkpatch errors and warnings.
*
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xtmrctr.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define TMRCTR_DEVICE_ID	XPAR_TMRCTR_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define TMRCTR_INTERRUPT_ID	XPAR_INTC_0_TMRCTR_0_VEC_ID

/*
 * The following constant determines which timer counter of the device that is
 * used for this example, there are currently 2 timer counters in a device
 * and this example uses the first one, 0, the timer numbers are 0 based
 */
#define TIMER_CNTR_0	 0
#define TIMER_CNTR_1	 1

/*
 * The following constants are used to set the reset value of the timer counter,
 * making this number larger reduces the amount of time this example consumes
 * because it is the value the timer counter is loaded with when it is started
 */
#define RESET_VALUE_CNTR_0	 0xF0000000
#define RESET_VALUE_CNTR_1	 0xFFFFFF00



/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int TmrCtrCascadeIntrExample(XIntc *IntcInstancePtr,
			XTmrCtr *InstancePtr,
			u16 DeviceId,
			u16 IntrId);

static int TmrCtrSetupIntrSystem(XIntc *IntcInstancePtr,
				XTmrCtr *InstancePtr,
				u16 DeviceId,
				u16 IntrId);

static void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber);

static void TmrCtrDisableIntr(XIntc *IntcInstancePtr, u16 IntrId);

/************************** Variable Definitions *****************************/

XIntc InterruptController;  /* The instance of the Interrupt Controller */
XTmrCtr TimerCounterInst;   /* The instance of the Timer Counter */

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile int TimerExpired;


/*****************************************************************************/
/**
* This function is the main function of the Tmrctr 64 bit example using
* Interrupts.
*
* @param	None.
*
* @return
*
*		- XST_SUCCESS to indicate success
*		- XST_FAILURE to indicate a failure.
*
* @note		None.
*
******************************************************************************/
int main(void)
{

	int Status;

	/*
	 * Run the Timer Counter Cascade mode - Interrupt example.
	 */
	Status = TmrCtrCascadeIntrExample(&InterruptController,
				  &TimerCounterInst,
				  TMRCTR_DEVICE_ID,
				  TMRCTR_INTERRUPT_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Tmrctr interrupt 64bit Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Tmrctr interrupt 64bit Example\r\n");
	return XST_SUCCESS;

}

/*****************************************************************************/
/**
* This function does a minimal test on the timer counter device and driver as a
* design example.  The purpose of this function is to illustrate how to use the
* XTmrCtr driver in cascade mode of operation.  It initializes a timer counter
* and then sets it up in compare mode with auto reload such that a periodic
* interrupt is generated.
*
* This function uses interrupt driven mode of the timer counter.
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance
* @param	TmrCtrInstancePtr is a pointer to the XTmrCtr driver Instance
* @param	DeviceId is the XPAR_<TmrCtr_instance>_DEVICE_ID value from
*		xparameters.h
* @param	IntrId is XPAR_<INTC_instance>_<TmrCtr_instance>_VEC_ID
*		value from xparameters.h
*
* @return
*		- XST_SUCCESS if the Test is successful.
*		- XST_FAILURE if the Test is Not Successful.
*
* @note		This function contains an infinite loop such that if interrupts
*		are not working it may never return.
*
*****************************************************************************/
int TmrCtrCascadeIntrExample(XIntc *IntcInstancePtr,
				XTmrCtr *TmrCtrInstancePtr,
				u16 DeviceId,
				u16 IntrId)
{
	int Status;
	int LastTimerExpired = 0;

	/*
	 * Initialize the timer counter so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	Status = XTmrCtr_Initialize(TmrCtrInstancePtr, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly, use the 1st timer in the device (0)
	 */
	Status = XTmrCtr_SelfTest(TmrCtrInstancePtr, TIMER_CNTR_0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly, use the 2nd timer in the device (0)
	 */
	Status = XTmrCtr_SelfTest(TmrCtrInstancePtr, TIMER_CNTR_1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the timer counter to the interrupt subsystem such that
	 * interrupts can occur.  This function is application specific.
	 * In the cascade mode only the interrupt from Timer Zero is valid.
	 */
	Status = TmrCtrSetupIntrSystem(IntcInstancePtr,
					TmrCtrInstancePtr,
					DeviceId,
					IntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the timer counter that will be called from the
	 * interrupt context when the timer expires, specify a pointer to the
	 * timer counter driver instance as the callback reference so
	 * the handler is able to access the instance data
	 */
	XTmrCtr_SetHandler(TmrCtrInstancePtr, TimerCounterHandler,
					   TmrCtrInstancePtr);


	/*
	 * Set a reset value for the timer counter such that it will expire
	 * eariler than letting it roll over from 0, the reset value is loaded
	 * into the timer counter when it is started
	 */
	XTmrCtr_SetResetValue(TmrCtrInstancePtr, TIMER_CNTR_0,
				RESET_VALUE_CNTR_0);
	XTmrCtr_SetResetValue(TmrCtrInstancePtr, TIMER_CNTR_1,
				RESET_VALUE_CNTR_1);


	/*
	 * Enable the interrupt of the timer counter so interrupts will occur
	 * and use auto reload mode such that the timer counter will reload
	 * itself automatically and continue repeatedly, without this option
	 * it would expire once only and set the Cascade mode.
	 */
	XTmrCtr_SetOptions(TmrCtrInstancePtr, TIMER_CNTR_0,
				XTC_INT_MODE_OPTION |
				XTC_AUTO_RELOAD_OPTION |
				XTC_CASCADE_MODE_OPTION);

	/*
	 * Reset the timer counters such that it's incrementing by default
	 */
	 XTmrCtr_Reset(TmrCtrInstancePtr, TIMER_CNTR_0);
	 XTmrCtr_Reset(TmrCtrInstancePtr, TIMER_CNTR_1);

	/*
	 * Start the timer counter 0 such that it's incrementing by default,
	 * then wait for it to timeout a number of times.
	 */
	XTmrCtr_Start(TmrCtrInstancePtr, TIMER_CNTR_0);


	while (1) {
		/*
		 * Wait for the first timer counter to expire as indicated
		 * by the shared variable which the handler will increment
		 */
		while (TimerExpired == LastTimerExpired) {
		}
		LastTimerExpired = TimerExpired;

		/*
		 * If it has expired a number of times, then stop the timer
		 * counter and stop this example
		 */
		if (TimerExpired == 3) {

			XTmrCtr_Stop(TmrCtrInstancePtr, TIMER_CNTR_0);
			break;
		}
	}

	TmrCtrDisableIntr(IntcInstancePtr, IntrId);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function is the handler which performs processing for the timer counter.
* It is called from an interrupt context such that the amount of processing
* performed should be minimized.  It is called when the timer counter expires
* if interrupts are enabled.
*
* This handler provides an example of how to handle timer counter interrupts
* but is application specific.
*
* @param	CallBackRef is a pointer to the callback function
* @param	TmrCtrNumber is the number of the timer to which this
*		handler is associated with.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber)
{
	XTmrCtr *InstancePtr = (XTmrCtr *)CallBackRef;

	/*
	 * Check if the timer counter has expired, checking is not necessary
	 * since that's the reason this function is executed, this just shows
	 * how the callback reference can be used as a pointer to the instance
	 * of the timer counter that expired, increment a shared variable so
	 * the main thread of execution can see the timer expired
	 */
	if (XTmrCtr_IsExpired(InstancePtr, TmrCtrNumber)) {
		TimerExpired++;
		if(TimerExpired == 3) {
			XTmrCtr_SetOptions(InstancePtr, TmrCtrNumber, 0);
		}
	}
}

/*****************************************************************************/
/**
* This function setups the interrupt system such that interrupts can occur
* for the timer counter. This function is application specific since the actual
* system may or may not have an interrupt controller.  The timer counter could
* be directly connected to a processor without an interrupt controller.  The
* user should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance.
* @param	TmrCtrInstancePtr is a pointer to the XTmrCtr driver Instance.
* @param	DeviceId is the XPAR_<TmrCtr_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	IntrId is XPAR_<INTC_instance>_<TmrCtr_instance>_VEC_ID
*		value from xparameters.h.
*
* @return
*		- XST_SUCCESS if the Test is successful
*		- XST_FAILURE if the Test is not successful
*
* @note		This function contains an infinite loop such that if interrupts
*		are not working it may never return.
*
******************************************************************************/
static int TmrCtrSetupIntrSystem(XIntc *IntcInstancePtr,
				 XTmrCtr *TmrCtrInstancePtr,
				 u16 DeviceId,
				 u16 IntrId)
{
	 int Status;


	/*
	 * Initialize the interrupt controller driver so that
	 * it's ready to use, specify the device ID that is generated in
	 * xparameters.h
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device
	 */
	Status = XIntc_Connect(IntcInstancePtr, IntrId,
				(XInterruptHandler)XTmrCtr_InterruptHandler,
				(void *)TmrCtrInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the timer counter can cause interrupts thru the interrupt controller.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Enable the interrupt for the timer counter
	 */
	XIntc_Enable(IntcInstancePtr, IntrId);


	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
					(Xil_ExceptionHandler)
					XIntc_InterruptHandler,
					IntcInstancePtr);

	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();


	return XST_SUCCESS;
}


/******************************************************************************/
/**
*
* This function disables the interrupts for the Timer.
*
* @param	IntcInstancePtr is a reference to the Interrupt Controller
*		driver Instance.
* @param	IntrId is XPAR_<INTC_instance>_<Timer_instance>_VEC_ID
*		value from xparameters.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void TmrCtrDisableIntr(XIntc *IntcInstancePtr, u16 IntrId)
{
	/*
	 * Disable the interrupt for the timer counter
	 */
	XIntc_Disable(IntcInstancePtr, IntrId);
}

