/******************************************************************************
*
* Copyright (C) 2011 - 2015 Xilinx, Inc.  All rights reserved.
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
*
* @file xiomodule_selftest.c
* @addtogroup iomodule_v2_5
* @{
*
* Contains diagnostic self-test functions for the XIOModule component. This
* file requires other files of the component to be linked in also.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sa   07/15/11 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xiomodule.h"
#include "xiomodule_i.h"
#include "xil_assert.h"

#include "xiomodule_io.h"
#include <stdio.h>

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* Run a self-test on the interrupt controller driver/device. This is a
* destructive test.
*
* This involves forcing interrupts into the controller (if possible, given
* the IO Module configuration) and verifying that they are recognized and can
* be acknowledged.
*
* @param	InstancePtr is a pointer to the XIOModule instance to be
* 		worked on.
*
* @return
* 		- XST_SUCCESS if self-test is successful.
* 		- XST_INTC_FAIL_SELFTEST if the Interrupt controller
*                 fails the self-test. It will fail the self test if the
*                 device has previously been started in real mode.
*
* @note		None.
*
******************************************************************************/
int XIOModule_Intc_SelfTest(XIOModule * InstancePtr)
{
	u32 CurrentISR;
	u32 Temp;

	/*
	 * Assert the arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Acknowledge all pending interrupts by reading the interrupt status
	 * register and writing the value to the acknowledge register
	 */
	Temp = XIomodule_In32(InstancePtr->BaseAddress + XIN_ISR_OFFSET);

	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IAR_OFFSET, Temp);

	/*
	 * Verify that there are no interrupts by reading the interrupt status
	 */
	CurrentISR = XIomodule_In32(InstancePtr->BaseAddress + XIN_ISR_OFFSET);

	/*
	 * ISR for internal interrupts should be zero after all interrupts
	 * are acknowledged. Skip checking external interrupts, since they may
	 * occur at any time.
	 */
	if ((CurrentISR & 0xffff) != 0) {
		return XST_INTC_FAIL_SELFTEST;
	}

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* Runs a self-test on the timer driver/device. This test verifies that the
* specified programmable interval timer of the device can be enabled and
* increments.
*
* @param	InstancePtr is a pointer to the XIOMOdule instance.
* @param	TimerNumber is the timer of the device to operate on.
*		Each device may contain multiple timers. The timer
*		number is a zero based number with a range of
*		0 to (XTC_DEVICE_TIMER_COUNT - 1).
*
* @return
* 		- XST_SUCCESS if self-test was successful
*		- XST_FAILURE if the timer is not incrementing.
*
* @note
*
* This is a destructive test using the provided timer. The current settings
* of the timer are returned to the initialized values and all settings at the
* time this function is called are overwritten.
*
******************************************************************************/
int XIOModule_Timer_SelfTest(XIOModule * InstancePtr, u8 TimerNumber)
{
	u32 TimerCount1 = 0;
	u32 TimerCount2 = 0;
	u16 Count = 0;

	/*
	 * Assert the arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(TimerNumber < XTC_DEVICE_TIMER_COUNT);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Set the Load register most significant bit to 1.
	 */
	XIOModule_SetResetValue(InstancePtr, TimerNumber,
		   1 << (InstancePtr->CfgPtr->PitSize[TimerNumber] - 1));

	/*
	 * Reset the timer and the interrupt
	 */
	XIOModule_Timer_SetOptions(InstancePtr, TimerNumber, 0);

	/*
	 * Set the control/status register to enable timer
	 */
	XIOModule_Timer_Start(InstancePtr, TimerNumber);

	/*
	 * Read the timer
	 */
	TimerCount1 = XIOModule_GetValue(InstancePtr, TimerNumber);

	/*
	 * Make sure timer is decrementing if the Count rolls under zero
	 * and the timer still has not decremented an error is returned
	 */
	do {
		TimerCount2 = XIOModule_GetValue(InstancePtr, TimerNumber);
		Count++;
	}
	while ((TimerCount1 == TimerCount2) && (Count != 0));

	/*
	 * Set the control/status register to 0 to complete initialization
	 * this disables the timer completely and allows it to be used again
	 */
	XIOModule_Timer_Stop(InstancePtr, TimerNumber);

	if (TimerCount1 == TimerCount2) {
		return XST_FAILURE;
	}
	else {
		return XST_SUCCESS;
	}
}

int XIOModule_SelfTest(XIOModule * InstancePtr)
{
	XStatus Status;
	u8 Timer;
	XIOModule_Config *CfgPtr;

	/*
	 * Assert the arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Perform the interrupt controller self test.
	 */
	Status = XIOModule_Intc_SelfTest(InstancePtr);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	/*
	 * Perform the Programmable Interval Timer (PIT) self test.
	 *
	 * Skip unused timers,timers with prescaler (since they may
	 * have very long expiration times), timers without readable
	 * counters, and timers with small size (since the counter
	 * may not change when sampled).
	 */
	CfgPtr = InstancePtr->CfgPtr;
	for (Timer = 0; Timer < XTC_DEVICE_TIMER_COUNT; Timer++) {
		if (CfgPtr->PitUsed[Timer] &&
		    CfgPtr->PitPrescaler[Timer] == XTC_PRESCALER_NONE &&
		    CfgPtr->PitReadable[Timer] &&
		    CfgPtr->PitSize[Timer] > 8) {
			Status = XIOModule_Timer_SelfTest(InstancePtr, Timer);
			if (Status != XST_SUCCESS)
			{
				return XST_FAILURE;
			}
		}
	}

	return XST_SUCCESS;
}
/** @} */
