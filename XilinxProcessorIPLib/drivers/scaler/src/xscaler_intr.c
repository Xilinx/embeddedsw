/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/**
*
* @file xscaler_intr.c
*
* This code contains interrupt related functions of Xilinx MVI Video Scaler
* device driver. The Scaler device converts a specified rectangular area of an
* input digital video image from one original sampling grid to a desired target
* sampling grid. Please see xscaler.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a xd   02/10/09 First release
* 2.00a xd   12/14/09 Updated doxygen document tags
* </pre>
*
******************************************************************************/

#include "xscaler.h"

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the Scaler driver.
*
* This handler reads the pending interrupt from the IER/ISR, determines the
* source of the interrupts, calls according callbacks, and finally clears the
* interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to	handle interrupts and installing the callbacks using
* XScaler_SetCallBack() during initialization phase. An example delivered with
* this driver demonstrates how this could be done.
*
* @param   InstancePtr is a pointer to the XScaler instance that just
*	   interrupted.
* @return  None.
* @note	   None.
*
******************************************************************************/
void XScaler_IntrHandler(void *InstancePtr)
{
	XScaler *XScalerPtr;
	u32 PendingIntr;
	u32 ErrorMask;
	u32 EventMask;

	XScalerPtr = (XScaler *)InstancePtr;

	/* Validate parameters */
	Xil_AssertVoid(XScalerPtr != NULL);
	Xil_AssertVoid(XScalerPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Get pending interrupts */
	PendingIntr = XScaler_IntrGetPending(XScalerPtr);

	/* Clear pending interrupt(s) */
	XScaler_IntrClear(XScalerPtr, PendingIntr);

	/* Error interrupt is occurring or spurious interrupt */
	if ((0 == (PendingIntr & XSCL_IXR_ALLINTR_MASK)) ||
		(PendingIntr & XSCL_IXR_ERROR_MASK)) {
		ErrorMask = PendingIntr & XSCL_IXR_ERROR_MASK;
		XScalerPtr->ErrorCallBack(XScalerPtr->ErrorRef, ErrorMask);

		/* The Error Interrupt Callback should reset the Scaler device
		 * and so there is no need to handle other pending interrupts
		 */
		 return;
	}

	/* A normal event just happened */
	if ((PendingIntr & XSCL_IXR_EVENT_MASK)) {
		EventMask = PendingIntr & XSCL_IXR_EVENT_MASK;
		XScalerPtr->EventCallBack(XScalerPtr->EventRef, EventMask);
	}

	return;
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType		   Callback Function Type
* -----------------------  ---------------------------
* XSCL_HANDLER_EVENT	   XScaler_CallBack
* XSCL_HANDLER_ERROR	   XScaler_CallBack
*
* HandlerType		   Invoked by this driver when:
* -----------------------  --------------------------------------------------
* XSCL_HANDLER_EVENT	   A normal event just happened. Normal events include
*			   frame done, Coefficient FIFO Ready and Shadow
*			   Register updated.
*
* XSCL_HANDLER_ERROR	   An error just happened.
*
* </pre>
*
* @param	InstancePtr is a pointer to the XScaler instance to be worked
*		on.
* @param	HandlerType specifies which callback is to be attached.
* @param	CallBackFunc is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return
*		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note
* Invoking this function for a handler that already has been installed replaces
* it with the new handler.
*
******************************************************************************/
int XScaler_SetCallBack(XScaler *InstancePtr, u32 HandlerType,
				void *CallBackFunc, void *CallBackRef)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(CallBackFunc != NULL);
	Xil_AssertNonvoid(CallBackRef != NULL);

	switch (HandlerType) {
	case XSCL_HANDLER_EVENT:
		InstancePtr->EventCallBack = (XScaler_CallBack)CallBackFunc;
		InstancePtr->EventRef = CallBackRef;
		break;

	case XSCL_HANDLER_ERROR:
		InstancePtr->ErrorCallBack = (XScaler_CallBack)CallBackFunc;
		InstancePtr->ErrorRef = CallBackRef;
		break;

	default:
		return XST_INVALID_PARAM;
	}
	return XST_SUCCESS;
}
