/******************************************************************************
*
* (c) Copyright 2014 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xcfa_intr.c
* @addtogroup cfa_v7_1
* @{
*
* This file contains interrupt related functions of the CFA core.
* Please see xcfa.h for more details of the core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 7.0   adk    01/07/14 First release.
*                       Implemented the following functions:
*                       XCfa_IntrHandler, XCfa_SetCallBack.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcfa.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the CFA core.
*
* This handler reads the pending interrupt from the STATUS register,
* determines the source of the interrupts and calls the respective
* callbacks for the interrupts that are enabled in IRQ_ENABLE register,
* and finally clears the interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this core is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XCfa_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XCfa instance that just
* 		interrupted.
*
* @return	None.
*
* @note		Interrupt interface should be enabled.
*
******************************************************************************/
void XCfa_IntrHandler(void *InstancePtr)
{
	u32 PendingIntr;
	u32 ErrorStatus;
	XCfa *XCfaPtr = NULL;
	XCfaPtr = (XCfa *)((void *)InstancePtr);

	/* Verify arguments. */
	Xil_AssertVoid(XCfaPtr != NULL);
	Xil_AssertVoid(XCfaPtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(XCfaPtr->Config.HasIntcIf != (u16)0x0);


	/* Get pending interrupts */
	PendingIntr = (u32)XCfa_IntrGetPending(XCfaPtr);

	/* A Slave Error interrupt has happened. */
	if (((PendingIntr) & (XCFA_IXR_SE_MASK)) ==
				((XCFA_IXR_SE_MASK))) {
		ErrorStatus = (PendingIntr) & ((u32)XCFA_IXR_SE_MASK);
		XCfaPtr->ErrCallBack(XCfaPtr->ErrRef, ErrorStatus);
	}

	/* A processing start has happened */
	if (((PendingIntr) & (XCFA_IXR_PROCS_STARTED_MASK)) ==
				(XCFA_IXR_PROCS_STARTED_MASK)) {
		XCfaPtr->ProcStartCallBack(XCfaPtr->ProcStartRef);
	}

	/* A frame done interrupt has happened */
	if (((PendingIntr) & (XCFA_IXR_EOF_MASK)) ==
				(XCFA_IXR_EOF_MASK)) {
		XCfaPtr->FrameDoneCallBack(XCfaPtr->FrameDoneRef);
	}

	/* Clear pending interrupt(s) */
	XCfa_IntrClear(XCfaPtr, PendingIntr);

}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType:
*
*<pre>
* HandlerType              Callback Function Type
* -----------------------  -----------------------------------
* XCFA_HANDLER_FRAMEDONE   FrameDoneCallBack
* XCFA_HANDLER_PROCSTART   ProcStartCallBack
* XCFA_HANDLER_ERROR       ErrCallBack
*
* </pre>
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
* @param	HandlerType specifies which callback is to be attached.
* @param	CallBackFunc is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
* 		callback function when it is invoked.
*
* @return
*		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
* 		installed replaces it with the new handler.
*
******************************************************************************/
int XCfa_SetCallBack(XCfa *InstancePtr, u32 HandlerType,
				void *CallBackFunc, void *CallBackRef)
{
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady ==
					(u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertNonvoid(CallBackFunc != NULL);
	Xil_AssertNonvoid(CallBackRef != NULL);
	Xil_AssertNonvoid((HandlerType >= (u32)(XCFA_HANDLER_PROCSTART)) &&
				(HandlerType <= (u32)(XCFA_HANDLER_ERROR)));

	/* Calls the respective callbacks according to handler type */
	switch (HandlerType) {
		case (XCFA_HANDLER_PROCSTART):
			InstancePtr->ProcStartCallBack =
				(XCfa_CallBack)((void *)CallBackFunc);
			InstancePtr->ProcStartRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		case (XCFA_HANDLER_FRAMEDONE):
			InstancePtr->FrameDoneCallBack =
				(XCfa_CallBack)((void *)CallBackFunc);
			InstancePtr->FrameDoneRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		case (XCFA_HANDLER_ERROR):
			InstancePtr->ErrCallBack =
				(XCfa_ErrorCallBack)((void *)CallBackFunc);
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
