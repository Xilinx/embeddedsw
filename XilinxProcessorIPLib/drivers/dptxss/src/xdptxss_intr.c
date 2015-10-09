/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xdptxss_intr.c
* @addtogroup dptxss_v1_0
* @{
*
* This file contains interrupt related functions of Xilinx DisplayPort TX
* Subsystem core. Please see xdptxss.h for more details of the core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- ---------------------------------------------------------
* 1.00 sha 01/29/15 Initial release.
* 1.00 sha 08/07/15 Added new handler types: lane count, link rate,
*                   Pre-emphasis voltage swing adjust and Set MSA.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdptxss.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the DisplayPort TX core operating
* in TX mode.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XDpTxSs_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_DpIntrHandler(void *InstancePtr)
{
	XDpTxSs *XDpTxSsPtr = (XDpTxSs *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDpTxSsPtr != NULL);
	Xil_AssertVoid(XDpTxSsPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* DisplayPort TX interrupt handler */
	XDp_InterruptHandler(XDpTxSsPtr->DpPtr);
}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType                         Callback Function Type
* ----------------------------------- -------------------------------------
* (XDPTXSS_HANDLER_DP_HPD_EVENT)      XDp_TxSetHpdEventHandler
* (XDPTXSS_HANDLER_DP_HPD_PULSE)      XDp_TxSetHpdPulseHandler
* (XDPTXSS_HANDLER_DP_LANE_COUNT_CHG) XDp_TxSetLaneCountChangeCallback
* (XDPTXSS_HANDLER_DP_LINK_RATE_CHG)  XDp_TxSetLinkRateChangeCallback
* (XDPTXSS_HANDLER_DP_PE_VS_ADJUST)   XDp_TxSetPeVsAdjustCallback
* (XDPTXSS_HANDLER_DP_SET_MSA)        XDp_TxSetMsaHandler
* </pre>
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	HandlerType specifies the type of handler.
* @param	CallbackFunc is the address of the callback function.
* @param	CallbackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return
*		- XST_SUCCESS if callback function installed successfully.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
******************************************************************************/
u32 XDpTxSs_SetCallBack(XDpTxSs *InstancePtr, u32 HandlerType,
			void *CallbackFunc, void *CallbackRef)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(HandlerType >= XDPTXSS_HANDLER_DP_HPD_EVENT);
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	/* Assign callback based on handler type */
	switch (HandlerType) {
		case XDPTXSS_HANDLER_DP_HPD_EVENT:
			XDp_TxSetHpdEventHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPTXSS_HANDLER_DP_HPD_PULSE:
			XDp_TxSetHpdPulseHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPTXSS_HANDLER_DP_LANE_COUNT_CHG:
			XDp_TxSetLaneCountChangeCallback(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPTXSS_HANDLER_DP_LINK_RATE_CHG:
			XDp_TxSetLinkRateChangeCallback(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPTXSS_HANDLER_DP_PE_VS_ADJUST:
			XDp_TxSetPeVsAdjustCallback(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPTXSS_HANDLER_DP_SET_MSA:
			XDp_TxSetMsaHandler(InstancePtr->DpPtr, CallbackFunc,
				CallbackRef);
			Status = XST_SUCCESS;
			break;

		default:
			Status = XST_INVALID_PARAM;
			break;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function installs a custom delay/sleep function to be used by the
* DisplayPort TX Subsystem.
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
* @param	CallbackFunc is the address to the callback function.
* @param	CallbackRef is the user data item (microseconds to delay) that
*		will be passed to the custom sleep/delay function when it is
*		invoked.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_SetUserTimerHandler(XDpTxSs *InstancePtr,
		XDpTxSs_TimerHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	/* Set custom timer wait handler */
	XDp_SetUserTimerHandler(InstancePtr->DpPtr, CallbackFunc, CallbackRef);
}
/** @} */
