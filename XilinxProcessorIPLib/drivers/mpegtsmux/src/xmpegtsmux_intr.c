/******************************************************************************
 *
 * Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xmpegtsmux_intr.c
 * @addtogroup mpegtsmux_v1_0
 * @{
 *
 * The functions in this file provides interrupt handler and associated
 * functions.
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xmpegtsmux_hw.h"
#include "xmpegtsmux.h"

#define XMPEG_TS_MUX_ISR_DONE_BIT_MASK 0x1

/*****************************************************************************/
/**
 **
 ** This function installs an asynchronous callback function:
 **
 ** @param    InstancePtr is a pointer to the MPEG TS MUX IP instance.
 ** @param    CallbackFunc is the address of the callback function.
 ** @param    CallbackRef is a user data item that will be passed to the
 **       callback function when it is invoked.
 **
 ** @return      None.
 **
 ** @note     Invoking this function for a handler that already has been
 **       installed replaces it with the new handler.
 **
 ******************************************************************************/
void XMpegTsMux_SetCallback(XMpegtsmux *InstancePtr, void *CallbackFunc,
	void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->Callback = (XMpegTsMux_Callback)CallbackFunc;
	InstancePtr->CallbackRef = CallbackRef;
}

/*****************************************************************************/
/**
 **
 ** This function is the interrupt handler for the MPEG TS MUX core driver.
 **
 ** This handler clears the pending interrupt and determines if the source is
 ** frame done signal. If yes, calls the registered callback function.
 **
 ** The application is responsible for connecting this function to the interrupt
 ** system.
 **
 ** @param    InstancePtr is a pointer to the core instance that just
 **           interrupted.
 **
 ** @return   None.
 **
 ** @note     None.
 **
 *****************************************************************************/
void *XMpegTsMuxIntrHandler(void *InstancePtr)
{
	XMpegtsmux *Ptr = (XMpegtsmux *) InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(Ptr != NULL);
	Xil_AssertVoid(Ptr->IsReady == XIL_COMPONENT_IS_READY);

	if (XMpegtsmux_InterruptGetStatus(Ptr)) {
		XMpegtsmux_InterruptClear(Ptr,
			XMPEG_TS_MUX_ISR_DONE_BIT_MASK);
		if (Ptr->Callback)
			Ptr->Callback(Ptr);
	}
}
/** @} */
