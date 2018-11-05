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
* @file xenhance_intr.c
* @addtogroup enhance_v7_1
* @{
*
* This code contains interrupt related functions of Xilinx Enhance core.
* Please see xenhance.h for more details of the core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 7.0   adk 02/19/14 First release.
*                    Implemented the following functions
*                    XEnhance_IntrHandler
*                    XEnhance_SetCallBack
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xenhance.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the Enhance driver.
*
* This handler reads the pending interrupt from the IER/ISR, determines the
* source of the interrupts, calls according callbacks, and finally clears the
* interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to	handle interrupts and installing the callbacks using
* XEnhance_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XEnhance instance that just
*		interrupted.
*
* @return	None.
*
* @note		Interrupt interface should be enabled.
*
******************************************************************************/
void XEnhance_IntrHandler(void *InstancePtr)
{
	u32 PendingIntr;
	u32 ErrorStatus;
	XEnhance *XEnhancePtr = (XEnhance *)((void *)InstancePtr);

	/* Verify arguments. */
	Xil_AssertVoid(XEnhancePtr != NULL);
	Xil_AssertVoid(XEnhancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(XEnhancePtr->Config.HasIntcIf != (u16)0x0);

	/* Get pending interrupts. */
	PendingIntr = (u32)(XEnhance_IntrGetPending(XEnhancePtr));

	/* A Slave Error interrupt has happened. */
	if (((PendingIntr) & (XENH_IXR_SE_MASK)) ==
				((XENH_IXR_SE_MASK))) {
		ErrorStatus = (PendingIntr) & (XENH_IXR_SE_MASK);
		XEnhancePtr->ErrCallBack(XEnhancePtr->ErrRef, ErrorStatus);
	}

	/* A Processing Start has happened. */
	if (((PendingIntr) & (XENH_IXR_PROCS_STARTED_MASK)) ==
					(XENH_IXR_PROCS_STARTED_MASK)) {
		XEnhancePtr->ProcStartCallBack
					(XEnhancePtr->ProcStartRef);
	}

	/* A Frame Done interrupt has happened */
	if (((PendingIntr) & (XENH_IXR_EOF_MASK)) ==
					(XENH_IXR_EOF_MASK)) {
		XEnhancePtr->FrameDoneCallBack
					(XEnhancePtr->FrameDoneRef);
	}

	/* Clear pending interrupt(s). */
	XEnhance_IntrClear(XEnhancePtr, PendingIntr);

}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType              Callback Function Type
* -----------------------  --------------------------------
* XENH_HANDLER_PROCSTART   ProcStartCallBack
* XENH_HANDLER_FRAMEDONE   FrameDoneCallBack
* XENH_HANDLER_ERROR       ErrCallBack
* </pre>
*
* @param	InstancePtr is a pointer to the XEnhance instance to be worked
* 		on.
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
* 		installed replaces it with the new handler.
*
******************************************************************************/
int XEnhance_SetCallBack(XEnhance *InstancePtr, u32 HandlerType,
				void *CallBackFunc, void *CallBackRef)
{
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady ==
					(u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertNonvoid(CallBackFunc != NULL);
	Xil_AssertNonvoid(CallBackRef != NULL);
	Xil_AssertNonvoid((HandlerType >= (u32)(XENH_HANDLER_PROCSTART)) &&
				(HandlerType <= (u32)(XENH_HANDLER_ERROR)));

	/* Calls the respective callbacks according to handler type. */
	switch (HandlerType) {
		case XENH_HANDLER_PROCSTART:
			InstancePtr->ProcStartCallBack =
				(XEnhance_CallBack)((void *)CallBackFunc);
			InstancePtr->ProcStartRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		case XENH_HANDLER_FRAMEDONE:
			InstancePtr->FrameDoneCallBack =
				(XEnhance_CallBack)((void *)CallBackFunc);
			InstancePtr->FrameDoneRef = CallBackRef;
			Status = (XST_SUCCESS);
			break;

		case XENH_HANDLER_ERROR:
			InstancePtr->ErrCallBack =
			(XEnhance_ErrorCallBack)((void *)CallBackFunc);
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
