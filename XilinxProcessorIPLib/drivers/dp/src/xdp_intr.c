/*******************************************************************************
 *
 * Copyright (C) 2015 - 2016 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdp_intr.c
 * @addtogroup dp_v7_0
 * @{
 *
 * This file contains functions related to XDp interrupt handling.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial release. TX code merged from the dptx driver.
 * 2.0   als  06/08/15 Added MST interrupt handlers for RX.
 *                     Guard against uninitialized callbacks.
 *                     Added HDCP interrupts.
 *                     Added unplug interrupt.
 * 4.0   als  02/18/16 Removed update of payload table in the driver's interrupt
 *                     handler.
 * 6.0   tu   05/30/17 Removed unused variable in XDp_RxInterruptHandler
 * 6.0   tu   09/08/17 Added two interrupt handler that addresses driver's
 *                     internal callback function of application
 *                     DrvHpdEventHandler and DrvHpdPulseHandler
 * 6.0   tu   09/08/17 Added three interrupt handler that addresses callback
 *                     function of application
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdp.h"

/**************************** Function Prototypes *****************************/

#if XPAR_XDPTXSS_NUM_INSTANCES
static void XDp_TxInterruptHandler(XDp *InstancePtr);
#endif
#if XPAR_XDPRXSS_NUM_INSTANCES
static void XDp_RxInterruptHandler(XDp *InstancePtr);
#endif

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function is the interrupt handler for the XDp driver.
 * When an interrupt happens, this interrupt handler will check which TX/RX mode
 * of operation the core is running in, and will call the appropriate interrupt
 * handler. The called interrupt handler will first detect what kind of
 * interrupt happened, then decides which callback function to invoke.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_InterruptHandler(XDp *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

#if XPAR_XDPTXSS_NUM_INSTANCES
	if (XDp_GetCoreType(InstancePtr) == XDP_TX) {
		XDp_TxInterruptHandler(InstancePtr);
	}  else
#endif
#if XPAR_XDPRXSS_NUM_INSTANCES
	if (XDp_GetCoreType(InstancePtr) == XDP_RX) {
		XDp_RxInterruptHandler(InstancePtr);
	}
#endif
	{
		/* Nothing to be done. */
	}
}

#if XPAR_XDPRXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function generates a pulse on the hot-plug-detect (HPD) line of the
 * specified duration.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	DurationUs is the duration of the HPD pulse, in microseconds.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxGenerateHpdInterrupt(XDp *InstancePtr, u16 DurationUs)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_HPD_INTERRUPT,
			(DurationUs << XDP_RX_HPD_INTERRUPT_LENGTH_US_SHIFT) |
			XDP_RX_HPD_INTERRUPT_ASSERT_MASK);
}

/******************************************************************************/
/**
 * This function enables interrupts associated with the specified mask.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Mask specifies which interrupts should be enabled. Bits set to
 *		1 will enable the corresponding interrupts.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxInterruptEnable(XDp *InstancePtr, u32 Mask)
{
	u32 MaskVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	MaskVal = XDp_ReadReg(InstancePtr->Config.BaseAddr,
				XDP_RX_INTERRUPT_MASK);
	MaskVal &= ~Mask;
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_INTERRUPT_MASK,
						MaskVal);
}

/******************************************************************************/
/**
 * This function disables interrupts associated with the specified mask.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Mask specifies which interrupts should be disabled. Bits set to
 *		1 will disable the corresponding interrupts.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxInterruptDisable(XDp *InstancePtr, u32 Mask)
{
	u32 MaskVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	MaskVal = XDp_ReadReg(InstancePtr->Config.BaseAddr,
				XDP_RX_INTERRUPT_MASK);
	MaskVal |= Mask;
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_INTERRUPT_MASK,
						MaskVal);
}

/******************************************************************************/
/**
 * This function enables interrupts associated with the specified mask1.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Mask specifies which interrupts should be enabled. Bits set to
 *		1 will enable the corresponding interrupts.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxInterruptEnable1(XDp *InstancePtr, u32 Mask)
{
	u32 MaskVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	MaskVal = XDp_ReadReg(InstancePtr->Config.BaseAddr,
			      XDP_RX_INTERRUPT_MASK_1);
	MaskVal &= ~Mask;
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_INTERRUPT_MASK_1,
						MaskVal);
}

/******************************************************************************/
/**
 * This function disables interrupts associated with the specified mask1.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Mask specifies which interrupts should be disabled. Bits set to
 *		1 will disable the corresponding interrupts.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxInterruptDisable1(XDp *InstancePtr, u32 Mask)
{
	u32 MaskVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	MaskVal = XDp_ReadReg(InstancePtr->Config.BaseAddr,
			      XDP_RX_INTERRUPT_MASK_1);
	MaskVal |= Mask;
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_INTERRUPT_MASK_1,
						MaskVal);
}
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

#if XPAR_XDPTXSS_NUM_INSTANCES
/*****************************************************************************/
/**
* This function installs callback functions for the given
* HandlerType:
*
* @param	InstancePtr is a pointer to the DP core instance.
* @param	HandlerType specifies the type of handler.
* @param	CallbackFunc is the address of the callback function.
* @param	CallbackRef is a user data item that will be passed to the
*			callback function when it is invoked.
*
* @return
*		- XST_SUCCESS if callback function installed successfully.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*			installed replaces it with the new handler.
*
******************************************************************************/
int XDp_TxSetCallback(XDp *InstancePtr,	XDp_Tx_HandlerType HandlerType,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(HandlerType < XDP_TX_NUM_HANDLERS);
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	u32 Status;

	switch (HandlerType)
	{
	case XDP_TX_HANDLER_SETMSA:
        InstancePtr->TxInstance.TxSetMsaCallback = CallbackFunc;
        InstancePtr->TxInstance.TxMsaCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_TX_HANDLER_HPDEVENT:
		InstancePtr->TxInstance.HpdEventHandler = CallbackFunc;
		InstancePtr->TxInstance.HpdEventCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_TX_HANDLER_DRV_HPDEVENT:
		InstancePtr->TxInstance.DrvHpdEventHandler = CallbackFunc;
		InstancePtr->TxInstance.DrvHpdEventCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_TX_HANDLER_HPDPULSE:
		InstancePtr->TxInstance.HpdPulseHandler = CallbackFunc;
		InstancePtr->TxInstance.HpdPulseCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_TX_HANDLER_DRV_HPDPULSE:
		InstancePtr->TxInstance.DrvHpdPulseHandler = CallbackFunc;
		InstancePtr->TxInstance.DrvHpdPulseCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_TX_HANDLER_LANECNTCHANGE:
		InstancePtr->TxInstance.LaneCountChangeCallback = CallbackFunc;
		InstancePtr->TxInstance.LaneCountChangeCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_TX_HANDLER_LINKRATECHANGE:
		InstancePtr->TxInstance.LinkRateChangeCallback = CallbackFunc;
		InstancePtr->TxInstance.LinkRateChangeCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_TX_HANDLER_PEVSADJUST:
		InstancePtr->TxInstance.PeVsAdjustCallback = CallbackFunc;
		InstancePtr->TxInstance.PeVsAdjustCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	default:
		Status = XST_INVALID_PARAM;
		break;

	}

	return Status;
}
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

#if XPAR_XDPRXSS_NUM_INSTANCES
/*****************************************************************************/
/**
* This function installs callback functions for the given
* HandlerType:
*
* @param	InstancePtr is a pointer to the DP core instance.
* @param	HandlerType specifies the type of handler.
* @param	CallbackFunc is the address of the callback function.
* @param	CallbackRef is a user data item that will be passed to the
*			callback function when it is invoked.
*
* @return
*		- XST_SUCCESS if callback function installed successfully.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*			installed replaces it with the new handler.
*
******************************************************************************/
int XDp_RxSetCallback(XDp *InstancePtr,	Dp_Rx_HandlerType HandlerType,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertNonvoid(HandlerType < XDP_RX_NUM_HANDLERS);
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	u32 Status;

	switch (HandlerType)
	{
	case XDP_RX_HANDLER_VMCHANGE:
		InstancePtr->RxInstance.IntrVmChangeHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrVmChangeCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_PWRSTATECHANGE:
		InstancePtr->RxInstance.IntrPowerStateHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrPowerStateCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_NOVIDEO:
		InstancePtr->RxInstance.IntrNoVideoHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrNoVideoCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_VBLANK:
		InstancePtr->RxInstance.IntrVBlankHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrVBlankCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_TRAININGLOST:
		InstancePtr->RxInstance.IntrTrainingLostHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrTrainingLostCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_VIDEO:
		InstancePtr->RxInstance.IntrVideoHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrVideoCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_AUD_INFOPKTRECV:
		InstancePtr->RxInstance.IntrInfoPktHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrInfoPktCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_AUD_EXTPKTRECV:
		InstancePtr->RxInstance.IntrExtPktHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrExtPktCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_TRAININGDONE:
		InstancePtr->RxInstance.IntrTrainingDoneHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrTrainingDoneCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_BWCHANGE:
		InstancePtr->RxInstance.IntrBwChangeHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrBwChangeCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_TP1:
		InstancePtr->RxInstance.IntrTp1Handler = CallbackFunc;
		InstancePtr->RxInstance.IntrTp1CallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_TP2:
		InstancePtr->RxInstance.IntrTp2Handler = CallbackFunc;
		InstancePtr->RxInstance.IntrTp2CallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_TP3:
		InstancePtr->RxInstance.IntrTp3Handler = CallbackFunc;
		InstancePtr->RxInstance.IntrTp3CallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

		/* Interrupts for DP 1.4 : set callback start. */
	case XDP_RX_HANDLER_TP4:
		if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
			InstancePtr->RxInstance.IntrTp4Handler = CallbackFunc;
			InstancePtr->RxInstance.IntrTp4CallbackRef = CallbackRef;
			Status = XST_SUCCESS;
		} else {
			Status = XST_FAILURE;
		}
		break;
		/* Interrupts for DP 1.4 : set callback end. */

	case XDP_RX_HANDLER_DOWNREQ:
		InstancePtr->RxInstance.IntrDownReqHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrDownReqCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_DOWNREPLY:
		InstancePtr->RxInstance.IntrDownReplyHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrDownReplyCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_AUD_PKTOVERFLOW:
		InstancePtr->RxInstance.IntrAudioOverHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrAudioOverCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_PAYLOADALLOC:
		InstancePtr->RxInstance.IntrPayloadAllocHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrPayloadAllocCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_ACT_SEQ:
		InstancePtr->RxInstance.IntrActRxHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrActRxCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_CRC_TEST:
		InstancePtr->RxInstance.IntrCrcTestHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrCrcTestCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_HDCP_DEBUG:
		InstancePtr->RxInstance.IntrHdcpDbgWrHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrHdcpDbgWrCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_HDCP_AKSV:
		InstancePtr->RxInstance.IntrHdcpAksvWrHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrHdcpAksvWrCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_HDCP_AN:
		InstancePtr->RxInstance.IntrHdcpAnWrHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrHdcpAnWrCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_HDCP_AINFO:
		InstancePtr->RxInstance.IntrHdcpAinfoWrHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrHdcpAinfoWrCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_HDCP_RO:
		InstancePtr->RxInstance.IntrHdcpRoRdHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrHdcpRoRdCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_HDCP_BINFO:
		InstancePtr->RxInstance.IntrHdcpBinfoRdHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrHdcpBinfoRdCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_UNPLUG:
		InstancePtr->RxInstance.IntrUnplugHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrUnplugCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

		/* Interrupts for DP 1.4 : set callback start. */
	case XDP_RX_HANDLER_ACCESS_LANE_SET:
		if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
			InstancePtr->RxInstance.IntrAccessLaneSetHandler =
					CallbackFunc;
			InstancePtr->RxInstance.IntrAccessLaneSetCallbackRef =
					CallbackRef;
			Status = XST_SUCCESS;
		} else {
			Status = XST_FAILURE;
		}
		break;

	case XDP_RX_HANDLER_ACCESS_LINK_QUAL:
		if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
			InstancePtr->RxInstance.IntrAccessLinkQualHandler =
					CallbackFunc;
			InstancePtr->RxInstance.IntrAccessLinkQualCallbackRef =
					CallbackRef;
			Status = XST_SUCCESS;
		} else {
			Status = XST_FAILURE;
		}
		break;

	case XDP_RX_HANDLER_ACCESS_ERR_COUNTER:
		if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
			InstancePtr->RxInstance.IntrAccessErrorCounterHandler =
					CallbackFunc;
			InstancePtr->RxInstance.IntrAccessErrorCounterCallbackRef =
					CallbackRef;
			Status = XST_SUCCESS;
		} else {
			Status = XST_FAILURE;
		}
		break;
		/* Interrupts for DP 1.4 : set callback end. */

	case XDP_RX_HANDLER_DRV_PWRSTATE:
		InstancePtr->RxInstance.IntrDrvPowerStateHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrDrvPowerStateCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_DRV_NOVIDEO:
		InstancePtr->RxInstance.IntrDrvNoVideoHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrDrvNoVideoCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_DRV_VIDEO:
		InstancePtr->RxInstance.IntrDrvVideoHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrDrvVideoCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	default:
		Status = XST_INVALID_PARAM;
		break;

	}

	return Status;
}

#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

#if XPAR_XDPTXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function is the interrupt handler for the XDp driver operating in TX
 * mode.
 *
 * When an interrupt happens, it first detects what kind of interrupt happened,
 * then decides which callback function to invoke.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void XDp_TxInterruptHandler(XDp *InstancePtr)
{
	u32 IntrStatus;
	u8 HpdEventDetected;
	u8 HpdPulseDetected;
	u32 HpdDuration;

	/* Determine what kind of interrupt occurred.
	 * Note: XDP_TX_INTERRUPT_STATUS is an RC (read-clear) register. */
	IntrStatus = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_TX_INTERRUPT_STATUS);
	IntrStatus &= ~XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_TX_INTERRUPT_MASK);

	HpdEventDetected = IntrStatus & XDP_TX_INTERRUPT_STATUS_HPD_EVENT_MASK;
	HpdPulseDetected = IntrStatus &
				XDP_TX_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK;

	if (HpdEventDetected) {
		if (InstancePtr->TxInstance.DrvHpdEventHandler)
			InstancePtr->TxInstance.DrvHpdEventHandler(
				InstancePtr->TxInstance.DrvHpdEventCallbackRef);
		if (InstancePtr->TxInstance.HpdEventHandler)
			InstancePtr->TxInstance.HpdEventHandler(
				InstancePtr->TxInstance.HpdEventCallbackRef);
	}
	else if (HpdPulseDetected && XDp_TxIsConnected(InstancePtr)) {
		/* The source device must debounce the incoming HPD signal by
		 * sampling the value at an interval greater than 0.500 ms. An
		 * HPD pulse should be of width 0.5 ms - 1.0 ms. */
		HpdDuration = XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_TX_HPD_DURATION);
		if (HpdDuration >= 500) {
			if (InstancePtr->TxInstance.DrvHpdPulseHandler)
				InstancePtr->TxInstance.DrvHpdPulseHandler(
				InstancePtr->TxInstance.DrvHpdPulseCallbackRef);
			if (InstancePtr->TxInstance.HpdPulseHandler)
				InstancePtr->TxInstance.HpdPulseHandler(
				InstancePtr->TxInstance.HpdPulseCallbackRef);
		}
	}
}
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

#if XPAR_XDPRXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function is the interrupt handler for the XDp driver operating in RX
 * mode.
 *
 * When an interrupt happens, it first detects what kind of interrupt happened,
 * then decides which callback function to invoke.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void XDp_RxInterruptHandler(XDp *InstancePtr)
{
	u32 IntrStatus;

	/* Determine what kind of interrupts have occurred.
	 * Note: XDP_RX_INTERRUPT_CAUSE is a RC (read-clear) register. */
	IntrStatus = XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_RX_INTERRUPT_CAUSE);
	/* Mask out required interrupts. */
	IntrStatus &= ~XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_RX_INTERRUPT_MASK);

	/* Training pattern 1 has started. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_TP1_MASK) &&
			InstancePtr->RxInstance.IntrTp1Handler) {
		InstancePtr->RxInstance.IntrTp1Handler(
			InstancePtr->RxInstance.IntrTp1CallbackRef);
	}
	/* Training pattern 2 has started. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_TP2_MASK) &&
			InstancePtr->RxInstance.IntrTp2Handler) {
		InstancePtr->RxInstance.IntrTp2Handler(
			InstancePtr->RxInstance.IntrTp2CallbackRef);
	}
	/* Training pattern 3 has started. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_TP3_MASK) &&
			InstancePtr->RxInstance.IntrTp3Handler) {
		InstancePtr->RxInstance.IntrTp3Handler(
			InstancePtr->RxInstance.IntrTp3CallbackRef);
	}
	/* Training lost - the link has been lost. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_TRAINING_LOST_MASK) &&
			InstancePtr->RxInstance.IntrTrainingLostHandler) {
		InstancePtr->RxInstance.IntrTrainingLostHandler(
			InstancePtr->RxInstance.IntrTrainingLostCallbackRef);
	}
	/* The link has been trained. */
	else if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_TRAINING_DONE_MASK) &&
			InstancePtr->RxInstance.IntrTrainingDoneHandler) {
		InstancePtr->RxInstance.IntrTrainingDoneHandler(
			InstancePtr->RxInstance.IntrTrainingDoneCallbackRef);
	}

	/* A change has been detected in the current video transmitted on the
	 * link as indicated by the main stream attributes (MSA) fields. The
	 * horizontal and vertical resolution parameters are monitored for
	 * changes. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_VM_CHANGE_MASK) &&
			InstancePtr->RxInstance.IntrVmChangeHandler) {
		InstancePtr->RxInstance.IntrVmChangeHandler(
			InstancePtr->RxInstance.IntrVmChangeCallbackRef);
	}
	/* The VerticalBlanking_Flag in the VB-ID field of the received stream
	 * indicates the start of the vertical blanking interval. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_VBLANK_MASK) &&
			InstancePtr->RxInstance.IntrVBlankHandler) {
		InstancePtr->RxInstance.IntrVBlankHandler(
			InstancePtr->RxInstance.IntrVBlankCallbackRef);
	}
	/* The receiver has detected the no-video flags in the VB-ID field after
	 * active video has been received. */
	if (IntrStatus & XDP_RX_INTERRUPT_CAUSE_NO_VIDEO_MASK) {
		if (InstancePtr->RxInstance.IntrDrvNoVideoHandler) {
			InstancePtr->RxInstance.IntrDrvNoVideoHandler(
			InstancePtr->RxInstance.IntrDrvNoVideoCallbackRef);
		}
		if (InstancePtr->RxInstance.IntrNoVideoHandler) {
			InstancePtr->RxInstance.IntrNoVideoHandler(
			InstancePtr->RxInstance.IntrNoVideoCallbackRef);
		}
	}
	/* A valid video frame is detected on the main link. */
	else if (IntrStatus & XDP_RX_INTERRUPT_CAUSE_VIDEO_MASK) {
		if (InstancePtr->RxInstance.IntrDrvVideoHandler) {
			InstancePtr->RxInstance.IntrDrvVideoHandler(
			InstancePtr->RxInstance.IntrDrvVideoCallbackRef);
		}
		if (InstancePtr->RxInstance.IntrVideoHandler) {
			InstancePtr->RxInstance.IntrVideoHandler(
			InstancePtr->RxInstance.IntrVideoCallbackRef);
		}
	}
	/* The transmitter has requested a change in the current power state of
	 * the receiver core. */
	if (IntrStatus & XDP_RX_INTERRUPT_CAUSE_POWER_STATE_MASK) {
		if (InstancePtr->RxInstance.IntrDrvPowerStateHandler) {
			InstancePtr->RxInstance.IntrDrvPowerStateHandler(
			InstancePtr->RxInstance.IntrDrvPowerStateCallbackRef);
		}
		if (InstancePtr->RxInstance.IntrPowerStateHandler) {
			InstancePtr->RxInstance.IntrPowerStateHandler(
			InstancePtr->RxInstance.IntrPowerStateCallbackRef);
		}
	}
	/* A change in the bandwidth has been detected. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_BW_CHANGE_MASK) &&
			InstancePtr->RxInstance.IntrBwChangeHandler) {
		InstancePtr->RxInstance.IntrBwChangeHandler(
			InstancePtr->RxInstance.IntrBwChangeCallbackRef);
	}

	/* An audio info packet has been received. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_INFO_PKT_MASK) &&
			InstancePtr->RxInstance.IntrInfoPktHandler) {
		InstancePtr->RxInstance.IntrInfoPktHandler(
			InstancePtr->RxInstance.IntrInfoPktCallbackRef);
	}
	/* An audio extension packet has been received. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_EXT_PKT_MASK) &&
			InstancePtr->RxInstance.IntrExtPktHandler) {
		InstancePtr->RxInstance.IntrExtPktHandler(
			InstancePtr->RxInstance.IntrExtPktCallbackRef);
	}

	/* The TX has issued a down request; a sideband message is ready. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_DOWN_REQUEST_MASK) &&
			InstancePtr->RxInstance.IntrDownReqHandler) {
		InstancePtr->RxInstance.IntrDownReqHandler(
			InstancePtr->RxInstance.IntrDownReqCallbackRef);
	}

	/* The RX has issued a down reply. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_DOWN_REPLY_MASK) &&
			InstancePtr->RxInstance.IntrDownReplyHandler) {
		InstancePtr->RxInstance.IntrDownReplyHandler(
			InstancePtr->RxInstance.IntrDownReplyCallbackRef);
	}

	/* An audio packet overflow has occurred. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_AUDIO_OVER_MASK) &&
			InstancePtr->RxInstance.IntrAudioOverHandler) {
		InstancePtr->RxInstance.IntrAudioOverHandler(
			InstancePtr->RxInstance.IntrAudioOverCallbackRef);
	}

	/* The RX's DPCD payload allocation registers have been written for
	 * allocation, de-allocation, or partial deletion. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_PAYLOAD_ALLOC_MASK) &&
			InstancePtr->RxInstance.IntrPayloadAllocHandler) {
		InstancePtr->RxInstance.IntrPayloadAllocHandler(
			InstancePtr->RxInstance.IntrPayloadAllocCallbackRef);
	}

	/* The ACT sequence has been received. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_ACT_RX_MASK) &&
			InstancePtr->RxInstance.IntrActRxHandler) {
		InstancePtr->RxInstance.IntrActRxHandler(
			InstancePtr->RxInstance.IntrActRxCallbackRef);
	}

	/* The CRC test has started. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_CRC_TEST_MASK) &&
			InstancePtr->RxInstance.IntrCrcTestHandler) {
		InstancePtr->RxInstance.IntrCrcTestHandler(
			InstancePtr->RxInstance.IntrCrcTestCallbackRef);
	}

	/* A write to one of the HDCP debug registers has been performed. */
	if ((IntrStatus & XDP_RX_INTERRUPT_MASK_HDCP_DEBUG_WRITE_MASK) &&
			InstancePtr->RxInstance.IntrHdcpDbgWrHandler) {
		InstancePtr->RxInstance.IntrHdcpDbgWrHandler(
			InstancePtr->RxInstance.IntrHdcpDbgWrCallbackRef);
	}
	/* A write to the HDCP Aksv MSB register has been performed. */
	if ((IntrStatus & XDP_RX_INTERRUPT_MASK_HDCP_AKSV_WRITE_MASK) &&
			InstancePtr->RxInstance.IntrHdcpAksvWrHandler) {
		InstancePtr->RxInstance.IntrHdcpAksvWrHandler(
			InstancePtr->RxInstance.IntrHdcpAksvWrCallbackRef);
	}
	/* A write to the HDCP An MSB register has been performed. */
	if ((IntrStatus & XDP_RX_INTERRUPT_MASK_HDCP_AN_WRITE_MASK) &&
			InstancePtr->RxInstance.IntrHdcpAnWrHandler) {
		InstancePtr->RxInstance.IntrHdcpAnWrHandler(
			InstancePtr->RxInstance.IntrHdcpAnWrCallbackRef);
	}
	/* A write to the HDCP Ainfo MSB register has been performed. */
	if ((IntrStatus & XDP_RX_INTERRUPT_MASK_HDCP_AINFO_WRITE_MASK) &&
			InstancePtr->RxInstance.IntrHdcpAinfoWrHandler) {
		InstancePtr->RxInstance.IntrHdcpAinfoWrHandler(
			InstancePtr->RxInstance.IntrHdcpAinfoWrCallbackRef);
	}
	/* A read of the HDCP Ro register has been performed. */
	if ((IntrStatus & XDP_RX_INTERRUPT_MASK_HDCP_RO_READ_MASK) &&
			InstancePtr->RxInstance.IntrHdcpRoRdHandler) {
		InstancePtr->RxInstance.IntrHdcpRoRdHandler(
			InstancePtr->RxInstance.IntrHdcpRoRdCallbackRef);
	}
	/* A read of the HDCP Binfo register has been performed. */
	if ((IntrStatus & XDP_RX_INTERRUPT_MASK_HDCP_BINFO_READ_MASK) &&
			InstancePtr->RxInstance.IntrHdcpBinfoRdHandler) {
		InstancePtr->RxInstance.IntrHdcpBinfoRdHandler(
			InstancePtr->RxInstance.IntrHdcpBinfoRdCallbackRef);
	}

	/* An unplug event has occurred. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_UNPLUG_MASK) &&
			InstancePtr->RxInstance.IntrUnplugHandler) {
		InstancePtr->RxInstance.IntrUnplugHandler(
			InstancePtr->RxInstance.IntrUnplugCallbackRef);
	}

	/* DP 1.4 related interrupt handling */
	if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
		u32 IntrStatus1;

		/* Determine what kind of interrupts have occurred.
		 * Note: XDP_RX_INTERRUPT_CAUSE is a RC (read-clear) register. */
		IntrStatus1 = XDp_ReadReg(InstancePtr->Config.BaseAddr,
					  XDP_RX_INTERRUPT_CAUSE_1);
		/* Mask out required interrupts. */
		IntrStatus1 &= ~XDp_ReadReg(InstancePtr->Config.BaseAddr,
					    XDP_RX_INTERRUPT_MASK_1);

		/* Training pattern 4 has started. */
		if ((IntrStatus1 & XDP_RX_INTERRUPT_MASK_TP4_MASK) &&
				InstancePtr->RxInstance.IntrTp4Handler) {
			InstancePtr->RxInstance.IntrTp4Handler(
				InstancePtr->RxInstance.IntrTp4CallbackRef);
		}
		/* Access lane set event. */
		if ((IntrStatus1 & XDP_RX_INTERRUPT_MASK_ACCESS_LANE_SET_MASK) &&
				InstancePtr->RxInstance.IntrAccessLaneSetHandler) {
			InstancePtr->RxInstance.IntrAccessLaneSetHandler(
				InstancePtr->RxInstance.IntrAccessLaneSetCallbackRef);
		}
		/* Access link qual set event. */
		if ((IntrStatus1 & XDP_RX_INTERRUPT_MASK_ACCESS_LINK_QUAL_MASK) &&
				InstancePtr->RxInstance.IntrAccessLinkQualHandler) {
			InstancePtr->RxInstance.IntrAccessLinkQualHandler(
				InstancePtr->RxInstance.IntrAccessLinkQualCallbackRef);
		}
		/* Access error counter read event. */
		if ((IntrStatus1 & XDP_RX_INTERRUPT_MASK_ACCESS_ERROR_COUNTER_MASK) &&
				InstancePtr->RxInstance.IntrAccessErrorCounterHandler) {
			InstancePtr->RxInstance.IntrAccessErrorCounterHandler(
				InstancePtr->RxInstance.IntrAccessErrorCounterCallbackRef);
		}
	}

}
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */
/** @} */
