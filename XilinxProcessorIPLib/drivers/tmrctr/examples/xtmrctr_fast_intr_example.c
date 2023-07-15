/******************************************************************************
* Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file  xtmrctr_fast_intr_example.c
*
* This file contains a design example using the timer counter driver
* (XTmCtr) and hardware device using fast interrupt mode.This example assumes
* that the interrupt controller is also present as a part of the system
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a bss  07/31/12 First release
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

#include "xtmrctr.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#ifdef SDT
#include "xinterrupt_wrap.h"

#define XTMRCTR_BASEADDRESS     XPAR_XTMRCTR_0_BASEADDR
#else
#include "xintc.h"
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define TMRCTR_DEVICE_ID	XPAR_TMRCTR_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define TMRCTR_INTERRUPT_ID	XPAR_INTC_0_TMRCTR_0_VEC_ID
#endif

/*
 * The following constant determines which timer counter of the device that is
 * used for this example, there are currently 2 timer counters in a device
 * and this example uses the first one, 0, the timer numbers are 0 based
 */
#define TIMER_CNTR_0	 0


/*
 * The following constant is used to set the reset value of the timer counter,
 * making this number larger reduces the amount of time this example consumes
 * because it is the value the timer counter is loaded with when it is started
 */
#define RESET_VALUE	 0xF0000000

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#ifndef SDT
int TmrCtrFastIntrExample(XIntc *IntcInstancePtr,
			  XTmrCtr *InstancePtr,
			  u16 DeviceId,
			  u16 IntrId,
			  u8 TmrCtrNumber);

static int TmrCtrSetupIntrSystem(XIntc *IntcInstancePtr,
				 XTmrCtr *InstancePtr,
				 u16 DeviceId,
				 u16 IntrId,
				 u8 TmrCtrNumber);


static void TmrCtrDisableIntr(XIntc *IntcInstancePtr, u16 IntrId);
#else
int TmrCtrFastIntrExample(XTmrCtr *InstancePtr,
			  UINTPTR BaseAddr,
			  u8 TmrCtrNumber);

static void TmrCtrDisableIntr(XTmrCtr *InstancePtr);
#endif

static void TmrCtr_FastHandler(void) __attribute__ ((fast_interrupt));
static void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber);

/************************** Variable Definitions *****************************/
#ifndef SDT
XIntc InterruptController;  /* The instance of the Interrupt Controller */
#endif

XTmrCtr TimerCounterInst;   /* The instance of the Timer Counter */

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile int TimerExpired;


/*****************************************************************************/
/**
* This is the main function of the Tmrctr example using Fast Interrupt feature
* in MicroBlaze and Intc controller.
*
* @param	None.
*
* @return	- XST_SUCCESS to indicate success.
*		- XST_FAILURE to indicate a failure.
*
* @note		None.
*
******************************************************************************/

int main(void)
{

	int Status;

	/*
	 * Run the Timer Counter Fast Interrupt example.
	 */
#ifndef SDT
	Status = TmrCtrFastIntrExample(&InterruptController,
				       &TimerCounterInst,
				       TMRCTR_DEVICE_ID,
				       TMRCTR_INTERRUPT_ID,
				       TIMER_CNTR_0);
#else
	Status = TmrCtrFastIntrExample(&TimerCounterInst,
				       XTMRCTR_BASEADDRESS,
				       TIMER_CNTR_0);
#endif

	if (Status != XST_SUCCESS) {
		xil_printf("Tmrctr fast interrupt Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Tmrctr fast interrupt Example\r\n");
	return XST_SUCCESS;

}


/*****************************************************************************/
/**
* This function does a minimal test on the timer counter device and driver as a
* design example.  The purpose of this function is to illustrate how to use the
* XTmrCtr component.  It initializes a timer counter and then sets it up in
* compare mode with auto reload such that a periodic interrupt is generated.
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
* @param	TmrCtrNumber is the number of the timer to which this
*		handler is associated with.
*
* @return
*		- XST_SUCCESS if the Test is successful
*		- XST_FAILURE if the Test is not successful
*
* @note		This function contains an infinite loop such that if interrupts
*		are not working it may never return.
*
*****************************************************************************/
#ifndef SDT
int TmrCtrFastIntrExample(XIntc *IntcInstancePtr,
			  XTmrCtr *TmrCtrInstancePtr,
			  u16 DeviceId,
			  u16 IntrId,
			  u8 TmrCtrNumber)
#else
int TmrCtrFastIntrExample(XTmrCtr *TmrCtrInstancePtr,
			  UINTPTR BaseAddr,
			  u8 TmrCtrNumber)
#endif
{
	int Status;
	int LastTimerExpired = 0;

	/*
	 * Initialize the timer counter so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
#ifndef SDT
	Status = XTmrCtr_Initialize(TmrCtrInstancePtr, DeviceId);
#else
	Status = XTmrCtr_Initialize(TmrCtrInstancePtr, BaseAddr);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly, use the 1st timer in the device (0)
	 */
	Status = XTmrCtr_SelfTest(TmrCtrInstancePtr, TmrCtrNumber);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the timer counter to the interrupt subsystem such that
	 * interrupts can occur.  This function is application specific.
	 */
#ifndef SDT
	Status = TmrCtrSetupIntrSystem(IntcInstancePtr,
				       TmrCtrInstancePtr,
				       DeviceId,
				       IntrId,
				       TmrCtrNumber);
#else
	Status = XSetupInterruptSystem(TmrCtrInstancePtr, TmrCtr_FastHandler, \
				       TmrCtrInstancePtr->Config.IntrId, TmrCtrInstancePtr->Config.IntrParent, \
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the timer counter that will be called from the
	 * interrupt context when the timer expires, specify a pointer to the
	 * timer counter driver instance as the callback reference so the
	 * handler is able to access the instance data
	 */
	XTmrCtr_SetHandler(TmrCtrInstancePtr, TimerCounterHandler,
			   TmrCtrInstancePtr);

	/*
	 * Enable the interrupt of the timer counter so interrupts will occur
	 * and use auto reload mode such that the timer counter will reload
	 * itself automatically and continue repeatedly, without this option
	 * it would expire once only
	 */
	XTmrCtr_SetOptions(TmrCtrInstancePtr, TmrCtrNumber,
			   XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);

	/*
	 * Set a reset value for the timer counter such that it will expire
	 * eariler than letting it roll over from 0, the reset value is loaded
	 * into the timer counter when it is started
	 */
	XTmrCtr_SetResetValue(TmrCtrInstancePtr, TmrCtrNumber, RESET_VALUE);

	/*
	 * Start the timer counter such that it's incrementing by default,
	 * then wait for it to timeout a number of times
	 */
	XTmrCtr_Start(TmrCtrInstancePtr, TmrCtrNumber);

	while (1) {
		/*
		 * Wait for the first timer counter to expire as indicated by
		 * the shared variable which the handler will increment
		 */
		while (TimerExpired == LastTimerExpired) {
		}
		LastTimerExpired = TimerExpired;

		/*
		 * If it has expired a number of times, then stop the timer
		 * counter and stop this example
		 */
		if (TimerExpired == 3) {

			XTmrCtr_Stop(TmrCtrInstancePtr, TmrCtrNumber);
			break;
		}
	}
#ifndef SDT
	TmrCtrDisableIntr(IntcInstancePtr, IntrId);
#else
	TmrCtrDisableIntr(TmrCtrInstancePtr);
#endif
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
		if (TimerExpired == 3) {
			XTmrCtr_SetOptions(InstancePtr, TmrCtrNumber, 0);
		}
	}
}

#ifndef SDT
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
* @param	TmrCtrNumber is the number of the timer to which this
*		handler is associated with.
*
* @return
*		- XST_SUCCESS if the initialization is successful
*		- XST_FAILURE if the initialization is not successful
*
* @note		None
*
*
******************************************************************************/
static int TmrCtrSetupIntrSystem(XIntc *IntcInstancePtr,
				 XTmrCtr *TmrCtrInstancePtr,
				 u16 DeviceId,
				 u16 IntrId,
				 u8 TmrCtrNumber)
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
	 * Connect a Fast handler that will be called when an interrupt
	 * for the device occurs.
	 */
	Status = XIntc_ConnectFastHandler(IntcInstancePtr, IntrId,
					  (XFastInterruptHandler)TmrCtr_FastHandler);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the timer counter can cause interrupts through the
	 * interrupt controller.
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
#endif

/*****************************************************************************/
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
#ifndef SDT
void TmrCtrDisableIntr(XIntc *IntcInstancePtr, u16 IntrId)
#else
void TmrCtrDisableIntr(XTmrCtr *TmrCtrInstancePtr)
#endif

{
#ifndef SDT
	/*
	 * Disable the interrupt for the timer counter
	 */
	XIntc_Disable(IntcInstancePtr, IntrId);
#else
	XDisableIntrId(TmrCtrInstancePtr->Config.IntrId, TmrCtrInstancePtr->Config.IntrParent);
#endif
}


/*****************************************************************************/
/**
*
* This is the Fast Interrupt Handler for the Timer.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void TmrCtr_FastHandler(void)
{

	/* Call the TmrCtr Interrupt handler */
	XTmrCtr_InterruptHandler(&TimerCounterInst);
}
