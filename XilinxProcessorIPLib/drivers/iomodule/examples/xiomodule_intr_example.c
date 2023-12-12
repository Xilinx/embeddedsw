/******************************************************************************
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file xiomodule_intr_example.c
*
* This file contains a design example using the IO Module driver (XIOModule)
* and hardware device using interrupt mode.This example tests the internal
* interrupts in the IO Module.
*
* This file can be used as a standalone example or by the TestAppGen utility
* to include a test of IOModule interrupts.
*
* @note
*
* None
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- --------------------------------------------------------
* 1.00a sa   07/15/11 First release
* 2.4   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 2.16  ml   12/07/23 Make TimerExpired as a static variable.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xiomodule.h"
#include "xil_exception.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place. This definition is not
 * included if the example is generated from the TestAppGen test tool.
 */
#ifndef TESTAPP_GEN
#define IOMODULE_DEVICE_ID XPAR_IOMODULE_0_DEVICE_ID
#endif

#define MAX_INTR_COUNT		3
#define MIN_TIMER_BITS		8
#define RESET_VALUE		((1 << MIN_TIMER_BITS) - 1)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

XStatus IOModuleIntrExample(XIOModule *IOModuleInstancePtr, u16 DeviceId);
XStatus IOModuleInterruptSetup(XIOModule *IOModuleInstancePtr,
			       u16 DeviceId);

void IOModuleHandler(void *CallBackRef, u8 Timer);
void IOModuleSetupIntrSystem(XIOModule *IOModuleInstancePtr);
void IOModuleDisableIntr(XIOModule *IOModuleInstancePtr);

/************************** Variable Definitions *****************************/
#ifndef TESTAPP_GEN
static XIOModule IOModule; /* Instance of the IO Module */
#endif
/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
static volatile int TimerExpired[XTC_DEVICE_TIMER_COUNT];

/*****************************************************************************/
/**
*
* This is the main function for the IO Module example. This function is not
* included if the example is generated from the TestAppGen test tool.
*
* @param    None.
*
* @return   XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note     None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	XStatus Status;

	/*
	 *  Run the example, specify the Device ID generated in xparameters.h
	 */
	Status = IOModuleIntrExample(&IOModule, IOMODULE_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Iomodule interrupt Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Iomodule interrupt Example\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* This function does a minimal test on the IO Module device and driver as a
* design example.  The purpose of this function is to illustrate how to use the
* IO Module component.  It initializes the Programmable Interval Timers and
* then sets it up in compare mode with auto reload such that a periodic
* interrupt is generated.
*
* This function uses interrupt driven mode of the IO Module.
*
* @param	IOModuleInstancePtr is a pointer to the IO Module driver
*		Instance
* @param	DeviceId is the XPAR_<IOModule_instance>_DEVICE_ID value from
*		xparameters.h
*
* @return	XST_SUCCESS if the Test is successful, otherwise XST_FAILURE
*
* @note		This function contains an infinite loop such that if interrupts
*		are not working it may never return.
*
*****************************************************************************/
XStatus IOModuleIntrExample(XIOModule *IOModuleInstancePtr, u16 DeviceId)
{
	int Status;
	u8 Timer;
	XIOModule_Config *CfgPtr = IOModuleInstancePtr->CfgPtr;

	/*
	 * Initialize the IO Module so that it's ready to use, specify the device
	 * ID that is generated in xparameters.h
	 */
	Status = XIOModule_Initialize(IOModuleInstancePtr, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XIOModule_SelfTest(IOModuleInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Initialize and enable interrupts in the processor.
	 */
	IOModuleSetupIntrSystem(IOModuleInstancePtr);

	/*
	 * Setup the handler for the IO Module handler that will be called from
	 * the interrupt context when an interrupt occurs, specify a pointer to
	 * the IO Module driver instance as the callback reference so the
	 * handler is able to access the instance data.
	 */
	XIOModule_SetHandler(IOModuleInstancePtr,
			     IOModuleHandler,
			     IOModuleInstancePtr);

	for (Timer = 0; Timer < XTC_DEVICE_TIMER_COUNT; Timer++) {
		/*
		 * Skip unused timers,timers with prescaler (since they may
		 * have very long expiration times), timers without readable
		 * counters, and timers with small size (since the counter
		 * may not change when sampled).
		 */
		if (!  (CfgPtr->PitUsed[Timer] &&
			CfgPtr->PitPrescaler[Timer] == XTC_PRESCALER_NONE &&
			CfgPtr->PitReadable[Timer] &&
			CfgPtr->PitSize[Timer] > MIN_TIMER_BITS)) {
			TimerExpired[Timer] = MAX_INTR_COUNT;
			continue;
		}

		/*
		 * Use auto reload mode such that the Programmable Interval Timers will
		 * reload automatically and continue repeatedly, without this option
		 * they would expire once only
		 */
		XIOModule_Timer_SetOptions(IOModuleInstancePtr, Timer,
					   XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);

		/*
		 * Set a reset value for the Programmable Interval Timers such that
		 * they will expire earlier than letting them roll over from 0, the
		 * reset value is loaded into the Programmable Interval Timers when
		 * they are started.
		 */
		XIOModule_SetResetValue(IOModuleInstancePtr, Timer, RESET_VALUE);

		/*
		 * Enable the interrupt for the Programmable Interval Timers.
		 */
		XIOModule_Enable(IOModuleInstancePtr,
				 Timer + XIN_IOMODULE_PIT_1_INTERRUPT_INTR);

		/*
		 * Start the Programmable Interval Timers such that they are
		 * decrementing by default, then wait for them to timeout a number of
		 * times.
		 */
		XIOModule_Timer_Start(IOModuleInstancePtr, Timer);
	}

	while (1) {
		int TotalExpiredCount = 0;

		/*
		 * Wait for the Programmable Interval Timers to expire as indicated by
		 * the shared variable which the handler will increment, and stop each
		 * timer when it has reached the expected number of times.
		 */
		for (Timer = 0; Timer < XTC_DEVICE_TIMER_COUNT; Timer++) {
			if (TimerExpired[Timer] >= MAX_INTR_COUNT) {
				XIOModule_Timer_Stop(IOModuleInstancePtr, Timer);
			}
			TotalExpiredCount += TimerExpired[Timer];
		}

		/*
		 * If all timers have expired the expected number of times, then stop
		 * this example.
		 */
		if (TotalExpiredCount == MAX_INTR_COUNT * XTC_DEVICE_TIMER_COUNT) {
			break;
		}
	}

	IOModuleDisableIntr(IOModuleInstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used by the TestAppGen generated application to setup
* the IO Module interrupts.
*
* @param    IOModuleInstancePtr is the reference to the IO Module instance.
* @param    DeviceId is device ID of the IO Module Device , typically
*           XPAR_<IOMODULE_instance>_DEVICE_ID value from xparameters.h
*
* @return   XST_SUCCESS to indicate success, otherwise XST_FAILURE
*
* @note     None.
*
******************************************************************************/
XStatus IOModuleInterruptSetup(XIOModule *IOModuleInstancePtr,
			       u16 DeviceId)
{
	XStatus Status;

	/*
	 * Initialize the IO Module driver so that it is ready to use.
	 */
	Status = XIOModule_Initialize(IOModuleInstancePtr, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XIOModule_SelfTest(IOModuleInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Initialize and enable interrupts in the processor.
	 */
	IOModuleSetupIntrSystem(IOModuleInstancePtr);

	/*
	 * Start the IO Module such that interrupts are enabled for all
	 * internal interrupts.
	 */
	Status = XIOModule_Start(IOModuleInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function is the handler which performs processing for the IO module.
* It is called from an interrupt context such that the amount of processing
* performed should be minimized.  It is called when an interrupt occurs
* if interrupts are enabled.
*
* This handler provides an example of how to handle Programmable Interval
* Timer interrupts but is application specific.
*
* @param	CallBackRef is a pointer to the callback function
* @param	Timer is the number of the interrupt this handler is
*		associated with.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void IOModuleHandler(void *CallBackRef, u8 Timer)
{
	XIOModule *InstancePtr = (XIOModule *)CallBackRef;

	/*
	 * Check if the Programmable Interval Timer has expired, checking is not
	 * necessary since that's the reason this function is executed, this just
	 * shows how the callback reference can be used as a pointer to the
	 * instance of the IO Module that had a timer that expired, increment a
	 * shared variable so the main thread of execution can see the timer
	 * expired.
	 */
	if (XIOModule_IsExpired(InstancePtr, Timer)) {
		TimerExpired[Timer]++;
		if (TimerExpired[Timer] == MAX_INTR_COUNT) {
			XIOModule_Timer_SetOptions(InstancePtr, Timer, 0);
		}
	}
}

/*****************************************************************************/
/**
* This function initializes and enables exception handling for interrupts in
* the processor.
*
* @param	IOModuleInstancePtr is the reference to the IO Module instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void IOModuleSetupIntrSystem(XIOModule *IOModuleInstancePtr)
{
	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the IO module interrupt handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XIOModule_DeviceInterruptHandler,
				     (void *) 0);

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();
}

/******************************************************************************/
/**
*
* This function disables the interrupts for the IO Module.
*
* @param	IOModuleInstancePtr is a reference to the IO Module driver
*		Instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void IOModuleDisableIntr(XIOModule *IOModuleInstancePtr)
{
	u8 Timer;

	/*
	 * Disable the interrupts
	 */
	for (Timer = 0; Timer < XTC_DEVICE_TIMER_COUNT; Timer++) {
		XIOModule_Disable(IOModuleInstancePtr,
				  Timer + XIN_IOMODULE_PIT_1_INTERRUPT_INTR);
	}
}
