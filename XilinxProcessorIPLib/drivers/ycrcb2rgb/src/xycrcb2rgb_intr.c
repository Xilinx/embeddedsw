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
* @file xycrcb2rgb_intr.c
* @addtogroup ycrcb2rgb_v7_1
* @{
*
* This file contains interrupt related functions for the YCRCB2RGB core.
* Please see xycrcb2rgb.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 7.0   adk    01/31/14 First Release.
*                       Implemented the following functions:
*                       XYCrCb2Rgb_IntrHandler
*                       XYCrCb2Rgb_SetCallBack
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xycrcb2rgb.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the YCRCB2RGB core.
*
* This handler reads the pending interrupt from the IER(IRQ_ENABLE register)
* or ISR (STATUS register), determines the source of the interrupts,
* calls according callbacks, and finally clears the interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to	handle interrupts and installing the callbacks using
* XYCrCb2Rgb_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance that just
*		interrupted.
*
* @return	None.
*
* @note		Interrupt interface (HasIntcIf) should be enabled.
*
******************************************************************************/
void XYCrCb2Rgb_IntrHandler(void *InstancePtr)
{
	u32 PendingIntr;
	u32 ErrorStatus;

	XYCrCb2Rgb *XYCrCb2RgbPtr = (XYCrCb2Rgb *)((void *)InstancePtr);

	/* Verify arguments. */
	Xil_AssertVoid(XYCrCb2RgbPtr != NULL);
	Xil_AssertVoid(XYCrCb2RgbPtr->IsReady ==
				(u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(XYCrCb2RgbPtr->Config.HasIntcIf != (u16)0x0);

	/* Get pending interrupts */
	PendingIntr = (u32)XYCrCb2Rgb_IntrGetPending(XYCrCb2RgbPtr);

	/* Slave  error interrupt has occurred */
	if ((PendingIntr & (XYCC_IXR_SE_MASK)) == (XYCC_IXR_SE_MASK)) {
		ErrorStatus = PendingIntr & (XYCC_IXR_SE_MASK);
		XYCrCb2RgbPtr->ErrCallBack(XYCrCb2RgbPtr->ErrRef, ErrorStatus);
	}

	/* A Processing Start has occurred */
	if ((PendingIntr & (XYCC_IXR_PROCS_MASK)) == (XYCC_IXR_PROCS_MASK)) {
		XYCrCb2RgbPtr->ProcStartCallBack( XYCrCb2RgbPtr->ProcStartRef);
	}

	/* A Frame Done interrupt has occurred */
	if ((PendingIntr & (XYCC_IXR_EOF_MASK)) == (XYCC_IXR_EOF_MASK)) {
		XYCrCb2RgbPtr->FrameDoneCallBack(XYCrCb2RgbPtr->FrameDoneRef);
	}

	/* Clear pending interrupt(s) */
	XYCrCb2Rgb_IntrClear(XYCrCb2RgbPtr, PendingIntr);
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerTypes.
*
* <pre>
* HandlerType              Callback Function Type
* -----------------------  ---------------------------
* XYCC_HANDLER_PROCSTART   ProcStartCallBack.
* XYCC_HANDLER_FRAMEDONE   FrameDoneCallBack.
* XYCC_HANDLER_ERROR       ErrCallBack.
* </pre>
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance to be
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
int XYCrCb2Rgb_SetCallBack(XYCrCb2Rgb *InstancePtr, u32 HandlerType,
				void *CallBackFunc, void *CallBackRef)
{
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady ==
					(u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertNonvoid(CallBackFunc != NULL);
	Xil_AssertNonvoid(CallBackRef != NULL);
	Xil_AssertNonvoid((HandlerType >= (u32)(XYCC_HANDLER_PROCSTART)) &&
				(HandlerType <= (u32)(XYCC_HANDLER_ERROR)));

	/* Setting function and reference based on HandlerType */
	switch (HandlerType) {
		case XYCC_HANDLER_PROCSTART:
			InstancePtr->ProcStartCallBack =
				(XYCrCb2Rgb_CallBack)((void *)CallBackFunc);
			InstancePtr->ProcStartRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		case XYCC_HANDLER_FRAMEDONE:
			InstancePtr->FrameDoneCallBack =
				(XYCrCb2Rgb_CallBack)((void *)CallBackFunc);
			InstancePtr->FrameDoneRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		case XYCC_HANDLER_ERROR:
			InstancePtr->ErrCallBack =
			(XYCrCb2Rgb_ErrorCallBack)((void *)CallBackFunc);
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
