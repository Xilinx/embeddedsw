/*******************************************************************************
 *
 * Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdptx_intr.c
 *
 * This file contains functions related to XDptx interrupt handling.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  05/17/14 Initial release.
 * 3.0   als  12/16/14 Increased debounce duration for HPD to 0.500ms.
 *                     Added masking of interrupts during servicing.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdptx.h"

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function installs a callback function for when a hot-plug-detect event
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDptx_SetHpdEventHandler(XDptx *InstancePtr,
			XDptx_HpdEventHandler CallbackFunc, void *CallbackRef)
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
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDptx_SetHpdPulseHandler(XDptx *InstancePtr,
			XDptx_HpdPulseHandler CallbackFunc, void *CallbackRef)
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
 * This function is the interrupt handler for the XDptx driver.
 *
 * When an interrupt happens, it first detects what kind of interrupt happened,
 * then decides which callback function to invoke.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDptx_HpdInterruptHandler(XDptx *InstancePtr)
{
	u32 IntrStatus;
	u8 HpdEventDetected;
	u8 HpdPulseDetected;
	u32 HpdDuration;
	u32 IntrMask;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Determine what kind of interrupt occurred.
	 * Note: XDPTX_INTERRUPT_STATUS is an RC (read-clear) register. */
	IntrStatus = XDptx_ReadReg(InstancePtr->Config.BaseAddr,
							XDPTX_INTERRUPT_STATUS);
	IntrStatus &= ~XDptx_ReadReg(InstancePtr->Config.BaseAddr,
							XDPTX_INTERRUPT_MASK);
	IntrMask = XDptx_ReadReg(InstancePtr->Config.BaseAddr,
							XDPTX_INTERRUPT_MASK);

	HpdEventDetected = IntrStatus & XDPTX_INTERRUPT_STATUS_HPD_EVENT_MASK;
	HpdPulseDetected = IntrStatus &
				XDPTX_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK;

	if (HpdEventDetected) {
		/* Mask interrupts while event handling is taking place. API
		 * will error out in case of a disconnection event anyway. */
		XDptx_WriteReg(InstancePtr->Config.BaseAddr,
			XDPTX_INTERRUPT_MASK, IntrMask |
			XDPTX_INTERRUPT_MASK_HPD_EVENT_MASK);

		InstancePtr->HpdEventHandler(InstancePtr->HpdEventCallbackRef);
	}
	else if (HpdPulseDetected && XDptx_IsConnected(InstancePtr)) {
		/* Mask interrupts while event handling is taking place. */
		XDptx_WriteReg(InstancePtr->Config.BaseAddr,
			XDPTX_INTERRUPT_MASK, IntrMask |
			XDPTX_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK);

		/* The source device must debounce the incoming HPD signal by
		 * sampling the value at an interval greater than 0.500 ms. An
		 * HPD pulse should be of width 0.5 ms - 1.0 ms. */
		HpdDuration = XDptx_ReadReg(InstancePtr->Config.BaseAddr,
							XDPTX_HPD_DURATION);
		if (HpdDuration >= 500) {
			InstancePtr->HpdPulseHandler(
					InstancePtr->HpdPulseCallbackRef);
		}
	}

	/* Unmask previously masked interrupts once handling is done. */
	XDptx_WriteReg(InstancePtr->Config.BaseAddr, XDPTX_INTERRUPT_MASK,
								IntrMask);
}
