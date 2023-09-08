/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xscutimer_intr_example.c
*
* This file contains a design example using the Cortex A9 Scu Private
* Timer and the driver (XScuTimer) using interrupts.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 1.00a nm   03/10/10 First release
* 2.5   dp   07/11/23 Add Support for system device tree flow
* 2.5   dp   09/08/23 Update example to stop timer at the end of the test
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xscutimer.h"
#ifndef SDT
#include "xscugic.h"
#else
#include "xinterrupt_wrap.h"
#endif
#include "xil_exception.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#ifndef TESTAPP_GEN
#define TIMER_DEVICE_ID		XPAR_XSCUTIMER_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define TIMER_IRPT_INTR		XPAR_SCUTIMER_INTR
#endif
#endif

#define TIMER_LOAD_VALUE	0xFFFF

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifndef SDT
int ScuTimerIntrExample(XScuGic *IntcInstancePtr, XScuTimer *TimerInstancePtr,
			u16 TimerDeviceId, u16 TimerIntrId);
#else
int ScuTimerIntrExample(XScuTimer *TimerInstancePtr, UINTPTR BaseAddress);
#endif

static void TimerIntrHandler(void *CallBackRef);

#ifndef SDT
static int TimerSetupIntrSystem(XScuGic *IntcInstancePtr,
				XScuTimer *TimerInstancePtr, u16 TimerIntrId);

static void TimerDisableIntrSystem(XScuGic *IntcInstancePtr, u16 TimerIntrId);
#endif

/************************** Variable Definitions *****************************/

#ifndef TESTAPP_GEN
XScuTimer TimerInstance;	/* Cortex A9 Scu Private Timer Instance */
#ifndef SDT
XScuGic IntcInstance;		/* Interrupt Controller Instance */
#endif
#endif

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile int TimerExpired;

/*****************************************************************************/
/**
* Main function to call the Cortex A9 Scu Private Timer interrupt example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	xil_printf("SCU Timer Interrupt Example Test \r\n");

	/*
	 * Call the interrupt example, specify the parameters generated in
	 * xparameters.h
	 */
#ifndef SDT
	Status = ScuTimerIntrExample(&IntcInstance, &TimerInstance,
				TIMER_DEVICE_ID, TIMER_IRPT_INTR);
#else
	Status = ScuTimerIntrExample(&TimerInstance, XPAR_SCUTIMER_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("SCU Timer Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SCU Timer Interrupt Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function tests the functioning of the Cortex A9 Scu Private Timer driver
* and hardware using interrupts.
*
* @param	IntcInstancePtr is a pointer to the instance of XScuGic driver.
* @param	TimerInstancePtr is a pointer to the instance of XScuTimer
*		driver.
* @param	TimerDeviceId is the Device ID of the XScuTimer device.
* @param	TimerIntrId is the Interrupt Id of the XScuTimer device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
int ScuTimerIntrExample(XScuGic *IntcInstancePtr, XScuTimer * TimerInstancePtr,
			u16 TimerDeviceId, u16 TimerIntrId)
#else
int ScuTimerIntrExample(XScuTimer * TimerInstancePtr,	UINTPTR BaseAddress)
#endif
{
	int Status;
	int LastTimerExpired = 0;
	XScuTimer_Config *ConfigPtr;

	/*
	 * Initialize the Scu Private Timer driver.
	 */
#ifndef SDT
	ConfigPtr = XScuTimer_LookupConfig(TimerDeviceId);
#else
	ConfigPtr = XScuTimer_LookupConfig(BaseAddress);
#endif
	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XScuTimer_CfgInitialize(TimerInstancePtr, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XScuTimer_SelfTest(TimerInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the device to interrupt subsystem so that interrupts
	 * can occur.
	 */
#ifndef SDT
	Status = TimerSetupIntrSystem(IntcInstancePtr,
					TimerInstancePtr, TimerIntrId);
#else
   Status = XSetupInterruptSystem(TimerInstancePtr, &TimerIntrHandler,
                                    TimerInstancePtr->Config.IntrId,
                                    TimerInstancePtr->Config.IntrParent,
                                    XINTERRUPT_DEFAULT_PRIORITY);
	/*
	 * Enable the timer interrupts for timer mode.
	 */
	XScuTimer_EnableInterrupt(TimerInstancePtr);

#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable Auto reload mode.
	 */
	XScuTimer_EnableAutoReload(TimerInstancePtr);

	/*
	 * Load the timer counter register.
	 */
	XScuTimer_LoadTimer(TimerInstancePtr, TIMER_LOAD_VALUE);

	/*
	 * Start the timer counter and then wait for it
	 * to timeout a number of times.
	 */
	XScuTimer_Start(TimerInstancePtr);

	while (1) {
		/*
		 * Wait for the first timer counter to expire as indicated by
		 * the shared variable which the handler will increment.
		 */
		while (TimerExpired == LastTimerExpired) {
		}

		LastTimerExpired = TimerExpired;
		/*
		 * If it has expired a number of times, then stop the timer
		 * counter and stop this example.
		 */
		if (TimerExpired == 3) {
			XScuTimer_Stop(TimerInstancePtr);
			break;
		}
	}
	/*
	 * Disable and disconnect the interrupt system.
	 */
#ifndef SDT
	TimerDisableIntrSystem(IntcInstancePtr, TimerIntrId);
#else
   XDisconnectInterruptCntrl(TimerInstancePtr->Config.IntrId, TimerInstancePtr->Config.IntrParent);
#endif

   XScuTimer_Stop(TimerInstancePtr);

	return XST_SUCCESS;
}

#ifndef SDT
/*****************************************************************************/
/**
*
* This function sets up the interrupt system such that interrupts can occur
* for the device.
*
* @param	IntcInstancePtr is a pointer to the instance of XScuGic driver.
* @param	TimerInstancePtr is a pointer to the instance of XScuTimer
*		driver.
* @param	TimerIntrId is the Interrupt Id of the XScuTimer device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int TimerSetupIntrSystem(XScuGic *IntcInstancePtr,
			      XScuTimer *TimerInstancePtr, u16 TimerIntrId)
{
	int Status;

#ifndef TESTAPP_GEN
	XScuGic_Config *IntcConfig;

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


	Xil_ExceptionInit();



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
	Status = XScuGic_Connect(IntcInstancePtr, TimerIntrId,
				(Xil_ExceptionHandler)TimerIntrHandler,
				(void *)TimerInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the device.
	 */
	XScuGic_Enable(IntcInstancePtr, TimerIntrId);

	/*
	 * Enable the timer interrupts for timer mode.
	 */
	XScuTimer_EnableInterrupt(TimerInstancePtr);

#ifndef TESTAPP_GEN
	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();
#endif

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function is the Interrupt handler for the Timer interrupt of the
* Timer device. It is called on the expiration of the timer counter in
* interrupt context.
*
* @param	CallBackRef is a pointer to the callback function.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void TimerIntrHandler(void *CallBackRef)
{
	XScuTimer *TimerInstancePtr = (XScuTimer *) CallBackRef;

	/*
	 * Check if the timer counter has expired, checking is not necessary
	 * since that's the reason this function is executed, this just shows
	 * how the callback reference can be used as a pointer to the instance
	 * of the timer counter that expired, increment a shared variable so
	 * the main thread of execution can see the timer expired.
	 */
	if (XScuTimer_IsExpired(TimerInstancePtr)) {
		XScuTimer_ClearInterruptStatus(TimerInstancePtr);
		TimerExpired++;
		if (TimerExpired == 3) {
			XScuTimer_DisableAutoReload(TimerInstancePtr);
		}
	}
}

#ifndef SDT
/*****************************************************************************/
/**
*
* This function disables the interrupts that occur for the device.
*
* @param	IntcInstancePtr is the pointer to the instance of XScuGic
*		driver.
* @param	TimerIntrId is the Interrupt Id for the device.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void TimerDisableIntrSystem(XScuGic *IntcInstancePtr, u16 TimerIntrId)
{
	/*
	 * Disconnect and disable the interrupt for the Timer.
	 */
	XScuGic_Disconnect(IntcInstancePtr, TimerIntrId);
}
#endif
