/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
* @addtogroup dptxss_v5_1
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
* 2.00 sha 08/07/15 Added new handler types: lane count, link rate,
*                   Pre-emphasis voltage swing adjust and Set MSA.
* 2.00 sha 09/28/15 Added HDCP and Timer Counter interrupt handlers.
* 3.0  sha 02/19/16 Added switch cases for
*                   XDPTXSS_HANDLER_HDCP_RPTR_DWN_STRM_RDY and
*                   XDPTXSS_HANDLER_HDCP_RPTR_EXCHG to register callback
*                   with HDCP.
* 4.1  als 08/08/16 Synchronize with new HDCP APIs.
* 5.0  tu  09/08/17 Added two interrupt handler that addresses driver's
*                   internal callback function of application
*                   DrvHpdEventHandler and DrvHpdPulseHandler
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

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDCP Cipher core.
*
* The application is responsible for connecting this function to the interrupt
* system.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_HdcpIntrHandler(void *InstancePtr)
{
	XDpTxSs *XDpTxSsPtr = (XDpTxSs *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDpTxSsPtr != NULL);
	Xil_AssertVoid(XDpTxSsPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* HDCP Cipher interrupt handler */
	XHdcp1x_CipherIntrHandler(XDpTxSsPtr->Hdcp1xPtr);
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the Timer Counter core.
*
* The application is responsible for connecting this function to the interrupt
* system.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_TmrCtrIntrHandler(void *InstancePtr)
{
	XDpTxSs *XDpTxSsPtr = (XDpTxSs *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDpTxSsPtr != NULL);
	Xil_AssertVoid(XDpTxSsPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Timer Counter interrupt handler */
	XTmrCtr_InterruptHandler(XDpTxSsPtr->TmrCtrPtr);
}
#endif

/*****************************************************************************/
/**
*
* This function is process some DP driver read as per DP spec on HPD EVENT.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_HpdEventProcess(void *InstancePtr)
{
	u32 Status = XST_SUCCESS;
	XDpTxSs *XDpTxSsPtr = (XDpTxSs *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDpTxSsPtr != NULL);
	Xil_AssertVoid(XDpTxSsPtr->IsReady == XIL_COMPONENT_IS_READY);
	XDpTxSs_UsrHpdEventData *UsrHpdEventData = &XDpTxSsPtr->UsrHpdEventData;

	// From here is the requirement per DP spec
	Status = XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_MAX_LINK_RATE, 1,
			&UsrHpdEventData->MaxCapNew);
	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_MAX_LANE_COUNT, 1,
			&UsrHpdEventData->MaxCapLanesNew);

	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_REV, 12,
				UsrHpdEventData->Tmp);
	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_SINK_COUNT, 6,
				UsrHpdEventData->Tmp);


	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_REV, 11,
				&UsrHpdEventData->Dpcd);
	Status |= XDp_TxGetEdidBlock(XDpTxSsPtr->DpPtr,
				     UsrHpdEventData->EdidOrg, 0);
	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_SINK_COUNT, 1,
				&UsrHpdEventData->Rd200);
	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_STATUS_LANE_0_1, 1,
			&UsrHpdEventData->Lane0Sts);
	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_STATUS_LANE_2_3, 1,
			&UsrHpdEventData->Lane2Sts);
	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_MAX_LINK_RATE, 1,
			&UsrHpdEventData->MaxCapNew);
	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_MAX_LANE_COUNT, 1,
			&UsrHpdEventData->MaxCapLanesNew);

	if(Status != XST_SUCCESS){
		xdbg_printf(XDBG_DEBUG_GENERAL, "AUX access had trouble!\r\n");
	}
}

/*****************************************************************************/
/**
*
* This function is process some DP driver read as per DP spec on HPD Pulse.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_HpdPulseProcess(void *InstancePtr)
{
	u32 Status = XST_SUCCESS;
	XDpTxSs *XDpTxSsPtr = (XDpTxSs *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDpTxSsPtr != NULL);
	Xil_AssertVoid(XDpTxSsPtr->IsReady == XIL_COMPONENT_IS_READY);
	XDpTxSs_UsrHpdPulseData *UsrHpdPulseData = &XDpTxSsPtr->UsrHpdPulseData;

	//reading the first block of EDID
	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
			 XDP_DPCD_SINK_COUNT, 6, UsrHpdPulseData->AuxValues);
	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
			 XDP_DPCD_STATUS_LANE_0_1, 1,
			 &UsrHpdPulseData->Lane0Sts);
	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
			 XDP_DPCD_STATUS_LANE_2_3, 1,
			 &UsrHpdPulseData->Lane2Sts);
	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
			 XDP_DPCD_LANE_ALIGN_STATUS_UPDATED, 1,
			 &UsrHpdPulseData->LaneAlignStatus);
	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
			 XDP_DPCD_LANE_COUNT_SET, 1,
			 &UsrHpdPulseData->LaneSet);
	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
			 XDP_DPCD_LINK_BW_SET, 1, &UsrHpdPulseData->BwSet);
	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
			XDP_DPCD_LINK_BW_SET, 1, &UsrHpdPulseData->BwSet);
	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
			XDP_DPCD_LANE_COUNT_SET, 1, &UsrHpdPulseData->LaneSet);
	Status |= XDp_TxGetEdidBlock(XDpTxSsPtr->DpPtr, UsrHpdPulseData->Edid,
		  0);
	if(Status != XST_SUCCESS){
		xdbg_printf(XDBG_DEBUG_GENERAL, "AUX access had trouble!\r\n");
	}
}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType                              Callback Function HandlerType
* ---------------------------------------- ------------------------------------
* (XDPTXSS_HANDLER_DP_HPD_EVENT)           XDP_TX_HANDLER_HPDEVENT
* (XDPTXSS_HANDLER_DP_HPD_PULSE)           XDP_TX_HANDLER_HPDPULSE
* (XDPTXSS_HANDLER_DP_LANE_COUNT_CHG)      XDP_TX_HANDLER_LANECNTCHANGE
* (XDPTXSS_HANDLER_DP_LINK_RATE_CHG)       XDP_TX_HANDLER_LINKRATECHANGE
* (XDPTXSS_HANDLER_DP_PE_VS_ADJUST)        XDP_TX_HANDLER_PEVSADJUST
* (XDPTXSS_HANDLER_HDCP_RPTR_EXCHG)        XHdcp1x_SetCallBack
* (XDPTXSS_HANDLER_DP_SET_MSA)             XDP_TX_HANDLER_SETMSA
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
			XDp_TxSetCallback(InstancePtr->DpPtr,
					XDP_TX_HANDLER_HPDEVENT,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPTXSS_DRV_HANDLER_DP_HPD_EVENT:
			XDp_TxSetCallback(InstancePtr->DpPtr,
				XDP_TX_HANDLER_DRV_HPDEVENT,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPTXSS_HANDLER_DP_HPD_PULSE:
			XDp_TxSetCallback(InstancePtr->DpPtr,
				XDP_TX_HANDLER_HPDPULSE,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPTXSS_DRV_HANDLER_DP_HPD_PULSE:
			XDp_TxSetCallback(InstancePtr->DpPtr,
				XDP_TX_HANDLER_DRV_HPDPULSE,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPTXSS_HANDLER_DP_LANE_COUNT_CHG:
			XDp_TxSetCallback(InstancePtr->DpPtr,
				XDP_TX_HANDLER_LANECNTCHANGE,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPTXSS_HANDLER_DP_LINK_RATE_CHG:
			XDp_TxSetCallback(InstancePtr->DpPtr,
				XDP_TX_HANDLER_LINKRATECHANGE,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPTXSS_HANDLER_DP_PE_VS_ADJUST:
			XDp_TxSetCallback(InstancePtr->DpPtr,
				XDP_TX_HANDLER_PEVSADJUST,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
		case XDPTXSS_HANDLER_HDCP_RPTR_EXCHG:
			XHdcp1x_SetCallBack(InstancePtr->Hdcp1xPtr,
				XHDCP1X_RPTR_HDLR_REPEATER_EXCHANGE,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;
#endif

		case XDPTXSS_HANDLER_DP_SET_MSA:
			XDp_TxSetCallback(InstancePtr->DpPtr,
				XDP_TX_HANDLER_SETMSA,
				CallbackFunc, CallbackRef);
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
