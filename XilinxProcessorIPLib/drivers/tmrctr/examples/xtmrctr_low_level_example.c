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
* @file  xtmrctr_low_level_example.c
*
* This file contains a design example using the Timer Counter (XTmrCtr)
* low level driver and hardware device in a polled mode.
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  02/13/02 First release
* 1.00b sv   04/26/05 Minor changes to comply to Doxygen and coding guidelines
* 2.00a ktn  10/30/09 Updated the example as the macros in the driver are
*                     renamed by removing _m in the definition.
*                     Minor changes as per coding guidelines are done.
* 4.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xtmrctr_l.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define TMRCTR_BASEADDR		XPAR_TMRCTR_0_BASEADDR

/*
 * This example only uses the 1st of the 2 timer counters contained in a
 * single timer counter hardware device
 */
#define TIMER_COUNTER_0	 0

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int TmrCtrLowLevelExample(u32 TmrCtrBaseAddress, u8 TimerCounter);

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
* This function is the main function of the Tmrctr low level example.
*
* @param	None.
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE to indicate
*		a Failure.
*
* @note		None.
*
*
*****************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the Timer Counter - Low Level example .
	 */
	Status = TmrCtrLowLevelExample(TMRCTR_BASEADDR, TIMER_COUNTER_0);
	if (Status != XST_SUCCESS) {
		xil_printf("Tmrctr lowlevel Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Tmrctr lowlevel Example\r\n");
	return XST_SUCCESS;

}

/*****************************************************************************/
/**
* This function does a minimal test on the timer counter device and
* the low level driver as a design example.  The purpose of this function is
* to illustrate how to use the XTmrCtr low level driver.
*
*
* @param	TmrCtrBaseAddress is the base address of the device.
* @param	TmrCtrNumber is the timer counter of the device to operate on.
*		Each device may contain multiple timer counters.
*		The timer number is a zero based number with a range of
*		0 - (XTC_DEVICE_TIMER_COUNT - 1).
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE to indicate
*		a Failure.
*
* @note
*
* This function contains a loop which waits for the value of a timer counter
* to change.  If the hardware is not working correctly, this function may not
* return.
*
****************************************************************************/
int TmrCtrLowLevelExample(u32 TmrCtrBaseAddress, u8 TmrCtrNumber)
{
	u32 Value1;
	u32 Value2;
	u32 ControlStatus;

	/*
	 * Clear the Control Status Register
	 */
	XTmrCtr_SetControlStatusReg(TmrCtrBaseAddress, TmrCtrNumber,0x0);

	/*
	 * Set the value that is loaded into the timer counter and cause it to
	 * be loaded into the timer counter
	 */
	XTmrCtr_SetLoadReg(TmrCtrBaseAddress, TmrCtrNumber, 0xDEADBEEF);
	XTmrCtr_LoadTimerCounterReg(TmrCtrBaseAddress, TmrCtrNumber);

	/*
	 * Clear the Load Timer bit in the Control Status Register
	 */
	ControlStatus = XTmrCtr_GetControlStatusReg(TmrCtrBaseAddress,
						 TmrCtrNumber);
	XTmrCtr_SetControlStatusReg(TmrCtrBaseAddress, TmrCtrNumber,
				 ControlStatus & (~XTC_CSR_LOAD_MASK));

	/*
	 * Get a snapshot of the timer counter value before it's started
	 * to compare against later
	 */
	Value1 = XTmrCtr_GetTimerCounterReg(TmrCtrBaseAddress, TmrCtrNumber);

	/*
	 * Start the timer counter such that it's incrementing by default
	 */
	XTmrCtr_Enable(TmrCtrBaseAddress, TmrCtrNumber);

	/*
	 * Read the value of the timer counter and wait for it to change,
	 * since it's incrementing it should change, if the hardware is not
	 * working for some reason, this loop could be infinite such that the
	 * function does not return
	 */
	while (1) {
		Value2 = XTmrCtr_GetTimerCounterReg(TmrCtrBaseAddress, TmrCtrNumber);

		if (Value1 != Value2) {
			break;
		}
	}

	/*
	 * Disable the timer counter such that it stops incrementing
	 */
	XTmrCtr_Disable(TmrCtrBaseAddress, TmrCtrNumber);

	return XST_SUCCESS;
}

