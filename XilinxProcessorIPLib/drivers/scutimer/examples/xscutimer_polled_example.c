/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file  xscutimer_polled_example.c
*
* This file contains a design example using the Scu Private Timer driver
* (XScuTimer) and hardware timer device. This test assumes Auto Reload mode is
* not enabled.
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 1.00a nm   03/10/10 First release
* 2.5   dp   07/11/23 Add Support for system device tree flow
* 2.5   dp   09/08/23 Update example to stop timer at the end of the test
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xscutimer.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define TIMER_DEVICE_ID		XPAR_XSCUTIMER_0_DEVICE_ID
#endif
#define TIMER_LOAD_VALUE	0xFF

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifndef SDT
int ScuTimerPolledExample(u16 DeviceId);
#else
int ScuTimerPolledExample(XScuTimer *TimerInstancePtr, UINTPTR BaseAddress);
#endif
/************************** Variable Definitions *****************************/

XScuTimer Timer;		/* Cortex A9 SCU Private Timer Instance */

/*****************************************************************************/
/**
*
* Main function to call the Scu Private Timer polled mode example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	xil_printf("SCU Timer Polled Mode Example Test \r\n");

	/*
	 * Call the polled example, specify the device ID that is generated in
	 * xparameters.h.
	 */
#ifndef SDT
   Status = ScuTimerPolledExample(TIMER_DEVICE_ID);
#else
	Status = ScuTimerPolledExample(&Timer, XPAR_SCUTIMER_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("SCU Timer Polled Mode Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SCU Timer Polled Mode Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function does a minimal test on the SCU Private timer device and driver.
* The purpose of this function is to illustrate how to use the XScuTimer driver.
*
* @param	DeviceId is the unique device id of the device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
#ifndef SDT
int ScuTimerPolledExample(u16 DeviceId)
#else
int ScuTimerPolledExample(XScuTimer *TimerInstancePtr, UINTPTR BaseAddress)
#endif
{
	int Status;
	volatile u32 CntValue1 = 0;
	volatile u32 CntValue2 = 0;
	XScuTimer_Config *ConfigPtr;
#ifndef SDT
	XScuTimer *TimerInstancePtr = &Timer;
#endif

	/*
	 * Initialize the Scu Private Timer so that it is ready to use.
	 */
#ifndef SDT
	ConfigPtr = XScuTimer_LookupConfig(DeviceId);
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
	 * Load the timer counter register.
	 */
	XScuTimer_LoadTimer(TimerInstancePtr, TIMER_LOAD_VALUE);

	/*
	 * Get a snapshot of the timer counter value before it's started
	 * to compare against later.
	 */
	CntValue1 = XScuTimer_GetCounterValue(TimerInstancePtr);

	/*
	 * Start the Scu Private Timer device.
	 */
	XScuTimer_Start(TimerInstancePtr);

	/*
	 * Read the value of the timer counter and wait for it to change,
	 * since it's decrementing it should change, if the hardware is not
	 * working for some reason, this loop could be infinite such that the
	 * function does not return.
	 */
	while (1) {
		CntValue2 = XScuTimer_GetCounterValue(TimerInstancePtr);
		if (CntValue1 != CntValue2) {
			break;
		}
	}

   XScuTimer_Stop(TimerInstancePtr);

	return XST_SUCCESS;
}
