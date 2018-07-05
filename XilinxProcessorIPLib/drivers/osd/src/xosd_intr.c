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
* @file xosd_intr.c
* @addtogroup osd_v4_0
* @{
*
* This code contains interrupt related functions of Xilinx Video
* On-Screen-Display core. Please see xosd.h for more details of
* the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 1.00a xd     08/18/08 First release.
* 2.00a cjm    12/18/12 Converted from xio.h to xil_io.h, translating basic
*                       types, MB cache functions, exceptions and assertions
*                       to xil_io format.
* 4.0   adk    02/18/14 Renamed the following functions:
*                       XOSD_IntrHandler - > XOsd_IntrHandler.
*                       XOSD_SetCallBack -> XOsd_SetCallBack.
*                       Removed the following handlers:
*                       XOSD_HANDLER_VBISTART, XOSD_HANDLER_VBIEND.
*                       Added new handler XOSD_HANDLER_PROCSTART.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xosd.h"

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

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
* callbacks to handle interrupts and installing the callbacks using
* XOsd_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XOsd instance that just
*		interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_IntrHandler(void *InstancePtr)
{
	u32 PendingIntr;
	u32 ErrorStatus;
	XOsd *XOsdPtr = NULL;

	XOsdPtr = (XOsd *)((void *)InstancePtr);

	/* Validate arguments. */
	Xil_AssertVoid(XOsdPtr != NULL);
	Xil_AssertVoid(XOsdPtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));

	/* Get pending interrupts. */
	PendingIntr = (u32)(XOsd_IntrGetPending(XOsdPtr));

	/* Error interrupt is occurring or spurious interrupt. */
	if (((PendingIntr) & (XOSD_IXR_ALLERR_MASK)) != (u32)0x00) {
		ErrorStatus = (PendingIntr) & (XOSD_IXR_ALLERR_MASK);
		XOsdPtr->ErrCallBack(XOsdPtr->ErrRef, ErrorStatus);
	}

	/* A Processing start interrupt has occurred. */
	if (((PendingIntr) & (XOSD_IXR_PROC_STARTED_MASK)) ==
				(XOSD_IXR_PROC_STARTED_MASK)) {
		XOsdPtr->ProcStartCallBack(XOsdPtr->ProcStartRef);
	}

	/* A frame done interrupt has occurred. */
	if (((PendingIntr) & (XOSD_IXR_EOF_MASK)) ==
				(XOSD_IXR_EOF_MASK)) {
		XOsdPtr->FrameDoneCallBack(XOsdPtr->FrameDoneRef);
	}

	/* Clear pending interrupts. */
	XOsd_IntrClear(XOsdPtr, PendingIntr);

}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType              Callback Function Type
* -----------------------  -------------------------------
* XOSD_HANDLER_PROCSTART   StubCallBack
* XOSD_HANDLER_FRAMEDONE   StubCallBack
* XOSD_HANDLER_ERROR       StubErrCallBack
*
* </pre>
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	HandlerType specifies which callback is to be attached.
* @param	CallBackFunc is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return	- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
******************************************************************************/
int XOsd_SetCallBack(XOsd *InstancePtr, u32 HandlerType,
			void *CallBackFunc, void *CallBackRef)
{
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady ==
				(u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertNonvoid(CallBackFunc != NULL);
	Xil_AssertNonvoid(CallBackRef != NULL);
	Xil_AssertNonvoid((HandlerType >= (u32)(XOSD_HANDLER_PROCSTART)) &&
				(HandlerType <= (u32)(XOSD_HANDLER_ERROR)));

	/* Setting the HandlerType. */
	switch (HandlerType) {
		case XOSD_HANDLER_PROCSTART:
			InstancePtr->ProcStartCallBack =
				(XOsd_CallBack)((void *)CallBackFunc);
			InstancePtr->ProcStartRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		case XOSD_HANDLER_FRAMEDONE:
			InstancePtr->FrameDoneCallBack =
				(XOsd_CallBack)((void *)CallBackFunc);
			InstancePtr->FrameDoneRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		case XOSD_HANDLER_ERROR:
			InstancePtr->ErrCallBack =
				(XOsd_ErrorCallBack)((void *)CallBackFunc);
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
