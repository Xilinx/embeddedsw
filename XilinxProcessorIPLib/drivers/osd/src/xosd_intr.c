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
/*****************************************************************************/
/**
*
* @file xosd_intr.c
*
* This code contains interrupt related functions of Xilinx MVI Video
* On-Screen-Display device driver. Please see xosd.h for more details of
* the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a xd   08/18/08 First release
* 2.00a cjm  12/18/12 Converted from xio.h to xil_io.h, translating
*                     basic types, MB cache functions, exceptions and
*                     assertions to xil_io format. 
* </pre>
*
******************************************************************************/

#include "xosd.h"

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the On-Screen-Display driver.
*
* This handler reads the pending interrupt from the IER/ISR, determines the
* source of the interrupts, calls according callbacks, and finally clears the
* interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to	handle interrupts and installing the callbacks using
* XOSD_SetCallBack() during initialization phase. An example delivered with
* this driver demonstrates how this could be done.
*
* @param   InstancePtr is a pointer to the XOSD instance that just interrupted.
* @return  None.
* @note	   None.
*
******************************************************************************/
void XOSD_IntrHandler(void *InstancePtr)
{
	u32 PendingIntr;
	u32 ErrorStatus;
	XOSD *XOSDPtr;

	XOSDPtr = (XOSD *)InstancePtr;

	/* Validate parameters */
	Xil_AssertVoid(XOSDPtr != NULL);
	Xil_AssertVoid(XOSDPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Get pending interrupts */
	PendingIntr = XOSD_IntrGetPending(XOSDPtr);

	/* Clear pending interrupts */
	XOSD_IntrClear(XOSDPtr, PendingIntr);

	/* Error interrupt is occurring or spurious interrupt */
	if ((0 == (PendingIntr & XOSD_IXR_ALLINTR_MASK)) ||
		(PendingIntr & XOSD_IXR_ALLIERR_MASK)) {

		ErrorStatus = PendingIntr & XOSD_IXR_ALLIERR_MASK;
		XOSDPtr->ErrCallBack(XOSDPtr->ErrRef, ErrorStatus);

		/* The Error CallBack should reset the device and so
		 * there is no need to handle other interrupts
		 */
		return;
	}

	/* A VBI Start has happened */
	if ((PendingIntr & XOSD_IXR_VBIS_MASK))
		XOSDPtr->VbiStartCallBack(XOSDPtr->VbiStartRef);

	/* A VBI End has happened */
	if ((PendingIntr & XOSD_IXR_VBIE_MASK))
		XOSDPtr->VbiEndCallBack(XOSDPtr->VbiEndRef);

	/* A Frame Done interrupt has happened */
	if ((PendingIntr & XOSD_IXR_FD_MASK))
		XOSDPtr->FrameDoneCallBack(XOSDPtr->FrameDoneRef);

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
* XOSD_HANDLER_VBISTART	   XOSD_CallBack
* XOSD_HANDLER_VBIEND	   XOSD_CallBack
* XOSD_HANDLER_FRAMEDONE   XOSD_CallBack
* XOSD_HANDLER_ERROR	   XOSD_ErrCallBack
*
* HandlerType		   Invoked by this driver when:
* -----------------------  --------------------------------------------------
* XOSD_HANDLER_VBISTART	   A Vertical Blank Interval Start Interrupt happens
* XOSD_HANDLER_VBIEND	   A Vertical Blank Interval End Interrupt happens
* XOSD_HANDLER_FRAMEDONE   A Frame Done Interrupt happens
* XOSD_HANDLER_ERROR	   An error condition happens
*
* </pre>
*
* @param	InstancePtr is a pointer to the XOSD instance to be worked on.
* @param	HandlerType specifies which callback is to be attached.
* @param	CallbackFunc is the address of the callback function.
* @param	CallbackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return
*  - XST_SUCCESS when handler is installed.
*  - XST_INVALID_PARAM when HandlerType is invalid.
*
* @note
* Invoking this function for a handler that already has been installed replaces
* it with the new handler.
*
******************************************************************************/
int XOSD_SetCallBack(XOSD *InstancePtr, u32 HandlerType,
			 void *CallBackFunc, void *CallBackRef)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(CallBackFunc != NULL);
	Xil_AssertNonvoid(CallBackRef != NULL);

	switch (HandlerType) {
	case XOSD_HANDLER_VBISTART:
		InstancePtr->VbiStartCallBack = (XOSD_CallBack)CallBackFunc;
		InstancePtr->VbiStartRef = CallBackRef;
		break;

	case XOSD_HANDLER_VBIEND:
		InstancePtr->VbiEndCallBack = (XOSD_CallBack)CallBackFunc;
		InstancePtr->VbiEndRef = CallBackRef;
		break;

	case XOSD_HANDLER_FRAMEDONE:
		InstancePtr->FrameDoneCallBack = (XOSD_CallBack)CallBackFunc;
		InstancePtr->FrameDoneRef = CallBackRef;
		break;

	case XOSD_HANDLER_ERROR:
		InstancePtr->ErrCallBack = (XOSD_ErrorCallBack)CallBackFunc;
		InstancePtr->ErrRef = CallBackRef;
		break;

	default:
		return XST_INVALID_PARAM;

	}
	return XST_SUCCESS;
}
