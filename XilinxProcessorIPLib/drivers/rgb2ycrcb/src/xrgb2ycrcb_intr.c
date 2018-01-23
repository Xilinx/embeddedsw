/******************************************************************************
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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xrgb2ycrcb_intr.c
* @addtogroup rgb2ycrcb_v7_1
* @{
*
* This file contains interrupt related functions for the RGB2YCRCB core.
* Please see xrgb2crcb.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who   Date     Changes
* ----- ----- -------- ---------------------------------------------------
* 7.0   adk   01/28/14 First release.
*                      Implemented the following functions:
*                      XRgb2YCrCb_IntrHandler,
*                      XRgb2YCrCb_SetCallBack.
* </pre>
*
******************************************************************************/

#include "xrgb2ycrcb.h"

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the RGB2YCRCB driver.
*
* This handler reads the pending interrupt from the IER(IRQ_ENABLE register)
* or ISR (STATUS register), determines the source of the interrupts,
* calls according callbacks, and finally clears the interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XRgb2YCrCb_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance that just
*		interrupted.
*
* @return	None.
*
* @note		Interrupt interface (HasIntcIf) should be enabled.
*
******************************************************************************/
void XRgb2YCrCb_IntrHandler(void *InstancePtr)
{
	u32 PendingIntr;
	u32 ErrorStatus;
	XRgb2YCrCb *XRgb2YCrCbPtr = NULL;

	XRgb2YCrCbPtr = (XRgb2YCrCb *)((void *)InstancePtr);

	/* Verify arguments. */
	Xil_AssertVoid(XRgb2YCrCbPtr != NULL);
	Xil_AssertVoid(XRgb2YCrCbPtr->IsReady ==
					(u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(XRgb2YCrCbPtr->Config.HasIntcIf != (u16)0x0);

	/* Get pending interrupts */
	PendingIntr = (u32)XRgb2YCrCb_IntrGetPending(XRgb2YCrCbPtr);

	/* Slave error interrupt has occurred */
	if (((PendingIntr) & (XRGB_IXR_SE_MASK)) == (XRGB_IXR_SE_MASK)) {

		ErrorStatus = (PendingIntr) & (XRGB_IXR_SE_MASK);
		XRgb2YCrCbPtr->ErrCallBack(XRgb2YCrCbPtr->ErrRef, ErrorStatus);
	}

	/* A processing start interrupt has occurred */
	if (((PendingIntr) & (XRGB_IXR_PROC_STARTED_MASK)) ==
					(XRGB_IXR_PROC_STARTED_MASK)) {
		XRgb2YCrCbPtr->ProcStartCallBack(XRgb2YCrCbPtr->ProcStartRef);
	}

	/* A frame done interrupt has occurred */
	if (((PendingIntr) & (XRGB_IXR_EOF_MASK)) == (XRGB_IXR_EOF_MASK)) {
		XRgb2YCrCbPtr->FrameDoneCallBack(XRgb2YCrCbPtr->FrameDoneRef);
	}

	/* Clear pending interrupt(s) */
	XRgb2YCrCb_IntrClear(XRgb2YCrCbPtr, PendingIntr);
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType              Callback Function Type
* -----------------------  --------------------------------------------------
* XRGB_HANDLER_PROCSTART   ProcStartCallBack
* XRGB_HANDLER_FRAMEDONE   FrameDoneCallBack
* XRGB_HANDLER_ERROR       ErrCallBack
*</pre>
*
* @param	InstancePtr is a pointer to the XRgb2YCrCb instance to be
*		worked on.
* @param	HandlerType specifies which callback is to be attached.
* @param	CallBackFunc is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return
*		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
******************************************************************************/
int XRgb2YCrCb_SetCallBack(XRgb2YCrCb *InstancePtr, u32 HandlerType,
				void *CallBackFunc, void *CallBackRef)
{
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady ==
					(u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertNonvoid(CallBackFunc != NULL);
	Xil_AssertNonvoid(CallBackRef != NULL);
	Xil_AssertNonvoid((HandlerType >= (u32)(XRGB_HANDLER_PROCSTART)) &&
				(HandlerType <= (u32)(XRGB_HANDLER_ERROR)));

	/* Sets the callback according to HandlerType */
	switch (HandlerType) {
		case XRGB_HANDLER_PROCSTART:
			InstancePtr->ProcStartCallBack =
				(XRgb2YCrCb_CallBack)((void *)CallBackFunc);
			InstancePtr->ProcStartRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		case XRGB_HANDLER_FRAMEDONE:
			InstancePtr->FrameDoneCallBack =
				(XRgb2YCrCb_CallBack)((void *)CallBackFunc);
			InstancePtr->FrameDoneRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		case XRGB_HANDLER_ERROR:
			InstancePtr->ErrCallBack =
				(XRgb2YCrCb_ErrorCallBack)((void *)CallBackFunc);
			InstancePtr->ErrRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		default:
			Status = (XST_INVALID_PARAM);
			break;
	}

	return Status;
}
/** @} */
