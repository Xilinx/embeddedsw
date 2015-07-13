/******************************************************************************
*
* Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
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
* @file xdualsplitter_intr.c
* @addtogroup dual_splitter_v1_0
* @{
*
* This file contains interrupt related functions for Xilinx Dual Splitter
* core. Please see xdualsplitter.h for more details of the core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- --------------------------------------------------
* 1.00  sha 07/21/14 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdualsplitter.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the Dual Splitter core.
*
* This handler reads the pending interrupt from the GENR_ERROR register,
* determines the source of the interrupts and calls the respective
* callbacks for the interrupts that are enabled in IRQ_ENABLE register,
* and finally clears the interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this core is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XDualSplitter_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
*
* @return	None.
*
* @note		Interrupt should be enabled to execute interrupt handler.
*
******************************************************************************/
void XDualSplitter_IntrHandler(void *InstancePtr)
{
	u32 PendingIntr;
	u32 ErrorStatus;

	XDualSplitter *XDualSpltrPtr = (XDualSplitter *)((void *)InstancePtr);

	/* Verify arguments. */
	Xil_AssertVoid(XDualSpltrPtr != NULL);
	Xil_AssertVoid(XDualSpltrPtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDualSpltrPtr->Config.HasIntrReq == 0x1);

	/* Get pending interrupts */
	PendingIntr = XDualSplitter_IntrGetPending(XDualSpltrPtr);

	/* Interrupt has happened due to error(s) */
	if (PendingIntr & XDUSP_ALL_ERR_MASK) {
		ErrorStatus = PendingIntr & (u32)XDUSP_ALL_ERR_MASK;
		XDualSpltrPtr->ErrCallback(XDualSpltrPtr->ErrRef, ErrorStatus);
	}

	/* Clear pending interrupt(s) */
	XDualSplitter_IntrClear(XDualSpltrPtr, PendingIntr);
}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
* @param	CallbackFunc is the address of the callback function.
* @param	CallbackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return	None.
*
* @note		Invoking this function for a callback that already has been
*		installed replaces it with the new callback.
*
******************************************************************************/
void XDualSplitter_SetCallback(XDualSplitter *InstancePtr,
				void *CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	/* sets user provided callback and callback reference */
	InstancePtr->ErrCallback =
			(XDualSplitter_ErrCallback)CallbackFunc;
	InstancePtr->ErrRef = CallbackRef;
}
/** @} */
