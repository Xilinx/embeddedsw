/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file  xtmrctr_polled_example.c
*
* This file contains a design example using the Timer Counter driver (XTmrCtr)
* and hardware device in a polled mode.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  02/13/02 First release
* 1.00b sv   04/26/05 Minor changes to comply to Doxygen and coding guidelines.
* 2.00a ktn  11/26/09 Minor changes as per coding guidelines.
* 4.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xtmrctr.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define TMRCTR_DEVICE_ID	XPAR_TMRCTR_0_DEVICE_ID
#else
#define XTMRCTR_BASEADDRESS	XPAR_XTMRCTR_0_BASEADDR
#endif


/*
 * This example only uses the 1st of the 2 timer counters contained in a
 * single timer counter hardware device
 */
#define TIMER_COUNTER_0	 0


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#ifndef SDT
int TmrCtrPolledExample(u16 DeviceId, u8 TmrCtrNumber);
#else
int TmrCtrPolledExample(UINTPTR BaseAddr, u8 TmrCtrNumber);
#endif
/************************** Variable Definitions *****************************/

XTmrCtr TimerCounter; /* The instance of the Tmrctr Device */


/*****************************************************************************/
/**
* Main function to call the polled example.
*
* @param	None.
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE to indicate
*		a Failure.
*
* @note		None.
*
******************************************************************************/
int main(void)
{

	int Status;

	/*
	 * Run the Timer Counter - Polled Example
	 */
#ifndef SDT
	Status = TmrCtrPolledExample(TMRCTR_DEVICE_ID, TIMER_COUNTER_0);
#else
	Status = TmrCtrPolledExample(XTMRCTR_BASEADDRESS, TIMER_COUNTER_0);
#endif

	if (Status != XST_SUCCESS) {
		xil_printf("Tmrctr polled Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Tmrctr polled Example\r\n");
	return XST_SUCCESS;

}


/*****************************************************************************/
/**
* This function does a minimal test on the timer counter device and
* driver as a design example. The purpose of this function is to illustrate
* how to use the XTmrCtr component in a polled mode.
*
*
* @param	DeviceId is the XPAR_<TMRCTR_instance>_DEVICE_ID value from
*		xparameters.h
* @param	TmrCtrNumber is the timer counter of the device to operate on.
*		 Each device may contain multiple timer counters.
*		The timer number is a zero based number with a range of
*		0 - (XTC_DEVICE_TIMER_COUNT - 1).
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE to indicate
*		a Failure.
*
* @note
*
* This function contains a loop which waits for the value of a timer counter
* to change. If the hardware is not working correctly, this function may not
* return.
*
****************************************************************************/
#ifndef SDT
int TmrCtrPolledExample(u16 DeviceId, u8 TmrCtrNumber)
#else
int TmrCtrPolledExample(UINTPTR BaseAddr, u8 TmrCtrNumber)
#endif
{
	int Status;
	u32 Value1;
	u32 Value2;
	XTmrCtr *TmrCtrInstancePtr = &TimerCounter;

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
	 * Enable the Autoreload mode of the timer counters.
	 */
	XTmrCtr_SetOptions(TmrCtrInstancePtr, TmrCtrNumber,
			   XTC_AUTO_RELOAD_OPTION);

	/*
	 * Get a snapshot of the timer counter value before it's started
	 * to compare against later
	 */
	Value1 = XTmrCtr_GetValue(TmrCtrInstancePtr, TmrCtrNumber);

	/*
	 * Start the timer counter such that it's incrementing by default
	 */
	XTmrCtr_Start(TmrCtrInstancePtr, TmrCtrNumber);

	/*
	 * Read the value of the timer counter and wait for it to change,
	 * since it's incrementing it should change, if the hardware is not
	 * working for some reason, this loop could be infinite such that the
	 * function does not return
	 */
	while (1) {
		Value2 = XTmrCtr_GetValue(TmrCtrInstancePtr, TmrCtrNumber);
		if (Value1 != Value2) {
			break;
		}
	}

	/*
	 * Disable the Autoreload mode of the timer counters.
	 */
	XTmrCtr_SetOptions(TmrCtrInstancePtr, TmrCtrNumber, 0);

	return XST_SUCCESS;
}

