/*******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdppsu_intr.c
 *
 * This file contains functions related to XDpPsu interrupt handling.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  01/17/17 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdppsu.h"
#include "xil_assert.h"

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function installs a callback function for when a hot-plug-detect event
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDpPsu_SetHpdEventHandler(XDpPsu *InstancePtr,
			XDpPsu_HpdEventHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->HpdEventHandler = CallbackFunc;
	InstancePtr->HpdEventCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a hot-plug-detect pulse
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDpPsu_SetHpdPulseHandler(XDpPsu *InstancePtr,
			XDpPsu_HpdPulseHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->HpdPulseHandler = CallbackFunc;
	InstancePtr->HpdPulseCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function is the interrupt handler for the XDpPsu driver.
 *
 * When an interrupt happens, it first detects what kind of interrupt happened,
 * then decides which callback function to invoke.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDpPsu_HpdInterruptHandler(XDpPsu *InstancePtr)
{
	u8 HpdEventDetected;
	u8 HpdPulseDetected;
	u32 HpdDuration;
	u32 IntrStatus;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Determine what kind of interrupt occurred. */
	IntrStatus = XDpPsu_ReadReg(InstancePtr->Config.BaseAddr,
							XDPPSU_INTR_STATUS);
	/* Write to clear interrupt status register bits. */
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_INTR_STATUS,
								IntrStatus);

	HpdEventDetected = IntrStatus & XDPPSU_INTR_HPD_EVENT_MASK;
	HpdPulseDetected = IntrStatus & XDPPSU_INTR_HPD_PULSE_DETECTED_MASK;

	if (HpdEventDetected) {
		InstancePtr->HpdEventHandler(InstancePtr->HpdEventCallbackRef);
	}
	if (HpdPulseDetected && XDpPsu_IsConnected(InstancePtr)) {
		/* The source device must debounce the incoming HPD signal by
		 * sampling the value at an interval greater than 0.500 ms. An
		 * HPD pulse should be of width 0.5 ms - 1.0 ms. */
		HpdDuration = XDpPsu_ReadReg(InstancePtr->Config.BaseAddr,
							XDPPSU_HPD_DURATION);
		if (HpdDuration >= 500) {
			InstancePtr->HpdPulseHandler(
					InstancePtr->HpdPulseCallbackRef);
		}
	}
}
