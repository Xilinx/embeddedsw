/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdp_intr.c
 * @addtogroup dp_v7_5
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
 * 6.0   jb   02/19/19 Added HDCP22 interrupts.
 * 7.5   rg   08/19/20 Added a interrupt handler that addresses driver's
 *                     internal callback function of application
 *                     ExtPktCallbackHandler
 * 7.5   rg   03/25/21 Add support for adaptive-sync in MST mode
 *
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

	case XDP_TX_HANDLER_EXTPKT_TXD:
		InstancePtr->TxInstance.ExtPktCallbackHandler = CallbackFunc;
		InstancePtr->TxInstance.ExtPktCallbackHandlerRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_TX_HANDLER_DRV_EXTPKT_TXD:
		InstancePtr->TxInstance.DrvExtPktCallbackHandler = CallbackFunc;
		InstancePtr->TxInstance.DrvExtPktCallbackHandlerRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_TX_HANDLER_VSYNC:
		InstancePtr->TxInstance.VsyncCallbackHandler = CallbackFunc;
		InstancePtr->TxInstance.VsyncCallbackHandlerRef = CallbackRef;
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
	u32 Index;

	switch (HandlerType)
	{
	case XDP_RX_HANDLER_VMCHANGE:
		InstancePtr->RxInstance.IntrVmChangeHandler[0] = CallbackFunc;
		InstancePtr->RxInstance.IntrVmChangeCallbackRef[0] = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_VMCHANGE_STREAM_2:
	case XDP_RX_HANDLER_VMCHANGE_STREAM_3:
	case XDP_RX_HANDLER_VMCHANGE_STREAM_4:
		Index = ((HandlerType + 1) -
			XDP_RX_HANDLER_VMCHANGE_STREAM_2);
		InstancePtr->RxInstance.IntrVmChangeHandler[Index] =
								CallbackFunc;
		InstancePtr->RxInstance.IntrVmChangeCallbackRef[Index] =
								CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_PWRSTATECHANGE:
		InstancePtr->RxInstance.IntrPowerStateHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrPowerStateCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_NOVIDEO:
		InstancePtr->RxInstance.IntrNoVideoHandler[0] = CallbackFunc;
		InstancePtr->RxInstance.IntrNoVideoCallbackRef[0] = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_NOVIDEO_STREAM_2:
	case XDP_RX_HANDLER_NOVIDEO_STREAM_3:
	case XDP_RX_HANDLER_NOVIDEO_STREAM_4:
		Index = ((HandlerType + 1) -
			XDP_RX_HANDLER_NOVIDEO_STREAM_2);
		InstancePtr->RxInstance.IntrNoVideoHandler[Index] =
								CallbackFunc;
		InstancePtr->RxInstance.IntrNoVideoCallbackRef[Index] =
								CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_VBLANK:
		InstancePtr->RxInstance.IntrVBlankHandler[0] = CallbackFunc;
		InstancePtr->RxInstance.IntrVBlankCallbackRef[0] = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_VBLANK_STREAM_2:
		InstancePtr->RxInstance.IntrVBlankHandler[1] = CallbackFunc;
		InstancePtr->RxInstance.IntrVBlankCallbackRef[1] = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_VBLANK_STREAM_3:
		InstancePtr->RxInstance.IntrVBlankHandler[2] = CallbackFunc;
		InstancePtr->RxInstance.IntrVBlankCallbackRef[2] = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_VBLANK_STREAM_4:
		InstancePtr->RxInstance.IntrVBlankHandler[3] = CallbackFunc;
		InstancePtr->RxInstance.IntrVBlankCallbackRef[3] = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_TRAININGLOST:
		InstancePtr->RxInstance.IntrTrainingLostHandler = CallbackFunc;
		InstancePtr->RxInstance.IntrTrainingLostCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_VIDEO:
		InstancePtr->RxInstance.IntrVideoHandler[0] = CallbackFunc;
		InstancePtr->RxInstance.IntrVideoCallbackRef[0] = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_VIDEO_STREAM_2:
	case XDP_RX_HANDLER_VIDEO_STREAM_3:
	case XDP_RX_HANDLER_VIDEO_STREAM_4:
		Index = ((HandlerType + 1) -
			XDP_RX_HANDLER_VIDEO_STREAM_2);
		InstancePtr->RxInstance.IntrVideoHandler[Index] = CallbackFunc;
		InstancePtr->RxInstance.IntrVideoCallbackRef[Index] = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_AUD_INFOPKTRECV:
		InstancePtr->RxInstance.IntrInfoPktHandler[0] = CallbackFunc;
		InstancePtr->RxInstance.IntrInfoPktCallbackRef[0] = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_AUD_INFOPKTRECV_STREAM_2:
	case XDP_RX_HANDLER_AUD_INFOPKTRECV_STREAM_3:
	case XDP_RX_HANDLER_AUD_INFOPKTRECV_STREAM_4:
		Index = ((HandlerType + 1) -
			XDP_RX_HANDLER_AUD_INFOPKTRECV_STREAM_2);
		InstancePtr->RxInstance.IntrInfoPktHandler[Index] =
								CallbackFunc;
		InstancePtr->RxInstance.IntrInfoPktCallbackRef[Index] =
								CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_AUD_EXTPKTRECV:
		InstancePtr->RxInstance.IntrExtPktHandler[0] = CallbackFunc;
		InstancePtr->RxInstance.IntrExtPktCallbackRef[0] = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_AUD_EXTPKTRECV_STREAM_2:
	case XDP_RX_HANDLER_AUD_EXTPKTRECV_STREAM_3:
	case XDP_RX_HANDLER_AUD_EXTPKTRECV_STREAM_4:
		Index = ((HandlerType + 1) -
			XDP_RX_HANDLER_AUD_EXTPKTRECV_STREAM_2);
		InstancePtr->RxInstance.IntrExtPktHandler[Index] =
								CallbackFunc;
		InstancePtr->RxInstance.IntrExtPktCallbackRef[Index] =
								CallbackRef;
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

	case XDP_RX_HANDLER_ADAPTIVE_SYNC_SDP:
		if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
			InstancePtr->RxInstance.IntrAdapatveSyncSdpHandler[0] = CallbackFunc;
			InstancePtr->RxInstance.IntrAdapatveSyncSdpCallbackRef[0] = CallbackRef;
			Status = XST_SUCCESS;
		} else {
			Status = XST_FAILURE;
		}
		break;

	case XDP_RX_HANDLER_ADAPTIVE_SYNC_SDP_STREAM_2:
	case XDP_RX_HANDLER_ADAPTIVE_SYNC_SDP_STREAM_3:
	case XDP_RX_HANDLER_ADAPTIVE_SYNC_SDP_STREAM_4:
		Index = ((HandlerType + 1) -
			XDP_RX_HANDLER_ADAPTIVE_SYNC_SDP_STREAM_2);
			InstancePtr->RxInstance.
			IntrAdapatveSyncSdpHandler[Index] = CallbackFunc;
			InstancePtr->RxInstance.
			IntrAdapatveSyncSdpCallbackRef[Index] = CallbackRef;
			Status = XST_SUCCESS;
		break;

	case XDP_RX_HANDLER_ADAPTIVE_SYNC_VBLANK:
		if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
			InstancePtr->RxInstance.IntrAdaptiveSyncVbHandler[0] = CallbackFunc;
			InstancePtr->RxInstance.IntrAdaptiveSyncVbCallbackRef[0] = CallbackRef;
			Status = XST_SUCCESS;
		} else {
			Status = XST_FAILURE;
		}
		break;

	case XDP_RX_HANDLER_ADAPTIVE_SYNC_VBLANK_STREAM_2:
	case XDP_RX_HANDLER_ADAPTIVE_SYNC_VBLANK_STREAM_3:
	case XDP_RX_HANDLER_ADAPTIVE_SYNC_VBLANK_STREAM_4:
		Index = ((HandlerType + 1) -
			XDP_RX_HANDLER_ADAPTIVE_SYNC_VBLANK_STREAM_2);
			InstancePtr->RxInstance.
			IntrAdaptiveSyncVbHandler[Index] = CallbackFunc;
			InstancePtr->RxInstance.
			IntrAdaptiveSyncVbCallbackRef[Index] = CallbackRef;
			Status = XST_SUCCESS;
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

#if (XPAR_XHDCP22_RX_NUM_INSTANCES > 0)
	case XDP_RX_HANDLER_HDCP22_AKE_INIT:
		InstancePtr->RxInstance.IntrHdcp22AkeInitWrHandler =
			CallbackFunc;
		InstancePtr->RxInstance.IntrHdcp22AkeInitWrCallbackRef =
			CallbackRef;
		Status = XST_SUCCESS;
		break;
	case XDP_RX_HANDLER_HDCP22_AKE_NO_STORED_KM:
		InstancePtr->RxInstance.IntrHdcp22AkeNoStoredKmWrHandler =
			CallbackFunc;
		InstancePtr->RxInstance.IntrHdcp22AkeNoStoredKmWrCallbackRef =
			CallbackRef;
		Status = XST_SUCCESS;
		break;
	case XDP_RX_HANDLER_HDCP22_AKE_STORED_KM:
		InstancePtr->RxInstance.IntrHdcp22AkeStoredWrHandler =
			CallbackFunc;
		InstancePtr->RxInstance.IntrHdcp22AkeStoredWrCallbackRef =
			CallbackRef;
		Status = XST_SUCCESS;
		break;
	case XDP_RX_HANDLER_HDCP22_LC_INIT:
		InstancePtr->RxInstance.IntrHdcp22LcInitWrHandler =
			CallbackFunc;
		InstancePtr->RxInstance.IntrHdcp22LcInitWrCallbackRef =
			CallbackRef;
		Status = XST_SUCCESS;
		break;
	case XDP_RX_HANDLER_HDCP22_SKE_SEND_EKS:
		InstancePtr->RxInstance.IntrHdcp22SkeSendEksWrHandler =
			CallbackFunc;
		InstancePtr->RxInstance.IntrHdcp22SkeSendEksWrCallbackRef =
			CallbackRef;
		Status = XST_SUCCESS;
		break;
	case XDP_RX_HANDLER_HDCP22_HPRIME_READ_DONE:
		InstancePtr->RxInstance.IntrHdcp22HprimeReadDoneHandler =
			CallbackFunc;
		InstancePtr->RxInstance.IntrHdcp22HprimeReadDoneCallbackRef =
			CallbackRef;
		Status = XST_SUCCESS;
		break;
	case XDP_RX_HANDLER_HDCP22_PAIRING_READ_DONE:
		InstancePtr->RxInstance.IntrHdcp22PairingReadDoneHandler =
			CallbackFunc;
		InstancePtr->RxInstance.IntrHdcp22PairingReadDoneCallbackRef =
			CallbackRef;
		Status = XST_SUCCESS;
		break;
	case XDP_RX_HANDLER_HDCP22_STREAM_TYPE:
		InstancePtr->RxInstance.IntrHdcp22StreamTypeWrHandler =
			CallbackFunc;
		InstancePtr->RxInstance.IntrHdcp22StreamTypeWrCallbackRef =
			CallbackRef;
		Status = XST_SUCCESS;
		break;
	case XDP_RX_HANDLER_HDCP22_REPEAT_AUTH_RCVID_LST_DONE:
		InstancePtr->RxInstance.IntrHdcp22RepeatAuthRcvIdLstAckWrHandler =
			CallbackFunc;
		InstancePtr->RxInstance.IntrHdcp22RepeatAuthRcvIdLstAckWrCallbackRef =
			CallbackRef;
		Status = XST_SUCCESS;
		break;
	case XDP_RX_HANDLER_HDCP22_REPEAT_AUTH_STREAM_MANAGE_DONE:
		InstancePtr->RxInstance.IntrHdcp22RepeatAuthStreamMangWrHandler =
			CallbackFunc;
		InstancePtr->RxInstance.IntrHdcp22RepeatAuthStreamMangWrCallbackRef =
			CallbackRef;
		Status = XST_SUCCESS;
		break;
#endif

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

	if(IntrStatus & XDP_TX_INTERRUPT_STATUS_EXT_PKT_TXD_MASK){
		if (InstancePtr->TxInstance.ExtPktCallbackHandler)
			InstancePtr->TxInstance.ExtPktCallbackHandler(
				InstancePtr->TxInstance.ExtPktCallbackHandlerRef);
		if (InstancePtr->TxInstance.DrvExtPktCallbackHandler)
			InstancePtr->TxInstance.DrvExtPktCallbackHandler(
				InstancePtr->TxInstance.DrvExtPktCallbackHandlerRef);
	}

	if(IntrStatus & XDP_TX_INTERRUPT_STATUS_VBLANK_STREAM1_MASK){
		if (InstancePtr->TxInstance.VsyncCallbackHandler)
			InstancePtr->TxInstance.VsyncCallbackHandler(
				InstancePtr->TxInstance.VsyncCallbackHandlerRef);
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
	u32 IntrStatus1;
	u32 IntrStatus2;

	/* Determine what kind of interrupts have occurred.
	 * Note: XDP_RX_INTERRUPT_CAUSE is a RC (read-clear) register. */
	IntrStatus = XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_RX_INTERRUPT_CAUSE);
	/* Mask out required interrupts. */
	IntrStatus &= ~XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_RX_INTERRUPT_MASK);

	IntrStatus1 = XDp_ReadReg(InstancePtr->Config.BaseAddr,
				  XDP_RX_INTERRUPT_CAUSE_1);
	/* Mask out required interrupts. */
	IntrStatus1 &= ~XDp_ReadReg(InstancePtr->Config.BaseAddr,
				    XDP_RX_INTERRUPT_MASK_1);

	/* Determine what kind of interrupts have occurred.
	 * Note: XDP_RX_INTERRUPT_CAUSE_2 is a RC (read-clear) register. */
	IntrStatus2 = XDp_ReadReg(InstancePtr->Config.BaseAddr,
			XDP_RX_INTERRUPT_CAUSE_2);

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
		InstancePtr->RxInstance.IntrVmChangeHandler[0]) {
			InstancePtr->RxInstance.IntrVmChangeHandler[0](
			InstancePtr->RxInstance.IntrVmChangeCallbackRef[0]);
	}

	/* A change has been detected in the stream 2 transmitted on the
	 * link as indicated by the main stream attributes (MSA) fields. The
	 * horizontal and vertical resolution parameters are monitored for
	 * changes. */
	if ((IntrStatus1 &
		XDP_RX_INTERRUPT_MASK_1_VM_CHANGE_STREAM234_MASK(
		XDP_RX_STREAM_ID2)) &&
		InstancePtr->RxInstance.IntrVmChangeHandler[1]) {
			InstancePtr->RxInstance.IntrVmChangeHandler[1](
			InstancePtr->RxInstance.
			IntrVmChangeCallbackRef[1]);
	}

	/* A change has been detected in the stream 3 transmitted on the
	 * link as indicated by the main stream attributes (MSA) fields. The
	 * horizontal and vertical resolution parameters are monitored for
	 * changes. */
	if ((IntrStatus1 &
		XDP_RX_INTERRUPT_MASK_1_VM_CHANGE_STREAM234_MASK(
		XDP_RX_STREAM_ID3)) &&
		InstancePtr->RxInstance.IntrVmChangeHandler[2]) {
			InstancePtr->RxInstance.IntrVmChangeHandler[2](
			InstancePtr->RxInstance.
			IntrVmChangeCallbackRef[2]);
	}

	/* A change has been detected in the stream 4 transmitted on the
	 * link as indicated by the main stream attributes (MSA) fields. The
	 * horizontal and vertical resolution parameters are monitored for
	 * changes. */
	if ((IntrStatus1 &
		XDP_RX_INTERRUPT_MASK_1_VM_CHANGE_STREAM234_MASK(
		XDP_RX_STREAM_ID4)) &&
		InstancePtr->RxInstance.IntrVmChangeHandler[3]) {
			InstancePtr->RxInstance.IntrVmChangeHandler[3](
			InstancePtr->RxInstance.
			IntrVmChangeCallbackRef[3]);
	}

	/* The VerticalBlanking_Flag in the VB-ID field of the received stream
	 * indicates the start of the vertical blanking interval. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_VBLANK_MASK) &&
			InstancePtr->RxInstance.IntrVBlankHandler[0]) {
		InstancePtr->RxInstance.IntrVBlankHandler[0](
			InstancePtr->RxInstance.IntrVBlankCallbackRef[0]);
	}
	/* The receiver has detected the no-video flags in the VB-ID field after
	 * active video has been received. */
	if (IntrStatus & XDP_RX_INTERRUPT_CAUSE_NO_VIDEO_MASK) {
		if (InstancePtr->RxInstance.IntrDrvNoVideoHandler) {
			InstancePtr->RxInstance.IntrDrvNoVideoHandler(
			InstancePtr->RxInstance.IntrDrvNoVideoCallbackRef);
		}
		if (InstancePtr->RxInstance.IntrNoVideoHandler[0]) {
			InstancePtr->RxInstance.IntrNoVideoHandler[0](
			InstancePtr->RxInstance.IntrNoVideoCallbackRef[0]);
		}
	}
	/* The receiver has detected the no-video flags in the VB-ID field after
	 * active video has been received stream 2. */
	if ((IntrStatus1 &
		XDP_RX_INTERRUPT_MASK_1_NO_VIDEO_STREAM234_MASK(
		XDP_RX_STREAM_ID2)) &&
		InstancePtr->RxInstance.IntrNoVideoHandler[1]) {
			InstancePtr->RxInstance.IntrNoVideoHandler[1](
			InstancePtr->RxInstance.
			IntrNoVideoCallbackRef[1]);
	}

	/* The receiver has detected the no-video flags in the VB-ID field after
	 * active video has been received stream 3. */
	if ((IntrStatus1 &
		XDP_RX_INTERRUPT_MASK_1_NO_VIDEO_STREAM234_MASK(
		XDP_RX_STREAM_ID3)) &&
		InstancePtr->RxInstance.IntrNoVideoHandler[2]) {
			InstancePtr->RxInstance.IntrNoVideoHandler[2](
			InstancePtr->RxInstance.
			IntrNoVideoCallbackRef[2]);
	}

	/* The receiver has detected the no-video flags in the VB-ID field after
	 * active video has been received stream 4. */
	if ((IntrStatus1 &
		XDP_RX_INTERRUPT_MASK_1_NO_VIDEO_STREAM234_MASK(
		XDP_RX_STREAM_ID4)) &&
		InstancePtr->RxInstance.IntrNoVideoHandler[3]) {
			InstancePtr->RxInstance.IntrNoVideoHandler[3](
			InstancePtr->RxInstance.
			IntrNoVideoCallbackRef[3]);
	}
	/* A valid video frame is detected on the main link. */
	else if (IntrStatus & XDP_RX_INTERRUPT_CAUSE_VIDEO_MASK) {
		if (InstancePtr->RxInstance.IntrDrvVideoHandler) {
			InstancePtr->RxInstance.IntrDrvVideoHandler(
			InstancePtr->RxInstance.IntrDrvVideoCallbackRef);
		}
		if (InstancePtr->RxInstance.IntrVideoHandler[0]) {
			InstancePtr->RxInstance.IntrVideoHandler[0](
			InstancePtr->RxInstance.IntrVideoCallbackRef[0]);
		}
	}
	/* A valid video frame is detected on the main link. */
	if ((IntrStatus1 &
		XDP_RX_INTERRUPT_MASK_1_VIDEO_STREAM234_MASK(
		XDP_RX_STREAM_ID2)) &&
		InstancePtr->RxInstance.IntrVideoHandler[1]) {
			InstancePtr->RxInstance.IntrVideoHandler[1](
			InstancePtr->RxInstance.
			IntrVideoCallbackRef[1]);
	}

	/* A valid video frame is detected on the main link. */
	if ((IntrStatus1 &
		XDP_RX_INTERRUPT_MASK_1_VIDEO_STREAM234_MASK(
		XDP_RX_STREAM_ID3)) &&
		InstancePtr->RxInstance.IntrVideoHandler[2]) {
			InstancePtr->RxInstance.IntrVideoHandler[2](
			InstancePtr->RxInstance.
			IntrVideoCallbackRef[2]);
	}

	/* A valid video frame is detected on the main link. */
	if ((IntrStatus1 &
		XDP_RX_INTERRUPT_MASK_1_VIDEO_STREAM234_MASK(
		XDP_RX_STREAM_ID4)) &&
		InstancePtr->RxInstance.IntrVideoHandler[3]) {
			InstancePtr->RxInstance.IntrVideoHandler[3](
			InstancePtr->RxInstance.
			IntrVideoCallbackRef[3]);
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
		InstancePtr->RxInstance.IntrInfoPktHandler[0]) {
		InstancePtr->RxInstance.IntrInfoPktHandler[0](
			InstancePtr->RxInstance.IntrInfoPktCallbackRef[0]);
	}
	/* An audio info packet has been received at stream 2. */
	if ((IntrStatus1 &
		XDP_RX_INTERRUPT_MASK_1_INFO_PKT_STREAM234_MASK(
		XDP_RX_STREAM_ID2)) &&
		InstancePtr->RxInstance.IntrInfoPktHandler[1]) {
			InstancePtr->RxInstance.IntrInfoPktHandler[1](
			InstancePtr->RxInstance.
			IntrInfoPktCallbackRef[1]);
	}

	/* An audio info packet has been received at stream 3. */
	if ((IntrStatus1 &
		XDP_RX_INTERRUPT_MASK_1_INFO_PKT_STREAM234_MASK(
		XDP_RX_STREAM_ID3)) &&
		InstancePtr->RxInstance.IntrInfoPktHandler[2]) {
			InstancePtr->RxInstance.IntrInfoPktHandler[2](
			InstancePtr->RxInstance.
			IntrInfoPktCallbackRef[2]);
	}

	/* An audio info packet has been received at stream 4. */
	if ((IntrStatus1 &
		XDP_RX_INTERRUPT_MASK_1_INFO_PKT_STREAM234_MASK(
		XDP_RX_STREAM_ID4)) &&
		InstancePtr->RxInstance.IntrInfoPktHandler[3]) {
			InstancePtr->RxInstance.IntrInfoPktHandler[3](
			InstancePtr->RxInstance.
			IntrInfoPktCallbackRef[3]);
	}

	/* An audio extension packet has been received. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_EXT_PKT_MASK) &&
		InstancePtr->RxInstance.IntrExtPktHandler[0]) {
		InstancePtr->RxInstance.IntrExtPktHandler[0](
			InstancePtr->RxInstance.IntrExtPktCallbackRef[0]);
	}

	/* An audio extension packet has been received at stream 2 */
	if ((IntrStatus1 &
		XDP_RX_INTERRUPT_MASK_1_EXT_PKT_STREAM234_MASK(
		XDP_RX_STREAM_ID2)) &&
		InstancePtr->RxInstance.IntrExtPktHandler[1]) {
			InstancePtr->RxInstance.IntrExtPktHandler[1](
			InstancePtr->RxInstance.
			IntrExtPktCallbackRef[1]);
	}

	/* An audio extension packet has been received at stream 3 */
	if ((IntrStatus1 &
		XDP_RX_INTERRUPT_MASK_1_EXT_PKT_STREAM234_MASK(
		XDP_RX_STREAM_ID3)) &&
		InstancePtr->RxInstance.IntrExtPktHandler[2]) {
			InstancePtr->RxInstance.IntrExtPktHandler[2](
			InstancePtr->RxInstance.
			IntrExtPktCallbackRef[2]);
	}

	/* An audio extension packet has been received at stream 4 */
	if ((IntrStatus1 &
		XDP_RX_INTERRUPT_MASK_1_EXT_PKT_STREAM234_MASK(
		XDP_RX_STREAM_ID4)) &&
		InstancePtr->RxInstance.IntrExtPktHandler[3]) {
			InstancePtr->RxInstance.IntrExtPktHandler[3](
			InstancePtr->RxInstance.
			IntrExtPktCallbackRef[3]);
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

#if (XPAR_XHDCP22_RX_NUM_INSTANCES > 0)
	/* A write to the HDCP22 Ake_Init MSB register has been performed. */
	if ((IntrStatus2 & XDP_RX_INTERRUPT_MASK_HDCP22_AKE_INIT_MASK) &&
			InstancePtr->RxInstance.IntrHdcp22AkeInitWrHandler) {
		InstancePtr->RxInstance.IntrHdcp22AkeInitWrHandler(
				InstancePtr->RxInstance.
				IntrHdcp22AkeInitWrCallbackRef);
	}

	/* A write to the HDCP22 Ake_No_Stored_Km
	 * MSB register has been performed.*/
	if ((IntrStatus2 &
			XDP_RX_INTERRUPT_MASK_HDCP22_AKE_NO_STORED_KM_MASK)
			&& InstancePtr->RxInstance.
			IntrHdcp22AkeNoStoredKmWrHandler) {
		InstancePtr->RxInstance.IntrHdcp22AkeNoStoredKmWrHandler(
			InstancePtr->
			RxInstance.IntrHdcp22AkeNoStoredKmWrCallbackRef);
	}

	/* A write to the HDCP22 Ake_Stored_Km
	 * MSB register has been performed. */
	if ((IntrStatus2 &
			XDP_RX_INTERRUPT_MASK_HDCP22_AKE_STORED_KM_MASK) &&
			InstancePtr->RxInstance.IntrHdcp22AkeStoredWrHandler) {
		InstancePtr->RxInstance.IntrHdcp22AkeStoredWrHandler(
				InstancePtr->RxInstance.
				IntrHdcp22AkeStoredWrCallbackRef);
	}

	/* A write to the HDCP22 LC_Init
	 * MSB register has been performed. */
	if ((IntrStatus2 &
			XDP_RX_INTERRUPT_MASK_HDCP22_LC_INIT_MASK) &&
			InstancePtr->RxInstance.IntrHdcp22LcInitWrHandler) {
		InstancePtr->RxInstance.IntrHdcp22LcInitWrHandler(
				InstancePtr->RxInstance.
				IntrHdcp22LcInitWrCallbackRef);
	}

	/* A write to the HDCP22 Ske_Send_Eks
	 * MSB register has been performed. */
	if ((IntrStatus2 &
			XDP_RX_INTERRUPT_MASK_HDCP22_SKE_SEND_EKS_MASK) &&
			InstancePtr->RxInstance.IntrHdcp22SkeSendEksWrHandler) {
		InstancePtr->RxInstance.IntrHdcp22SkeSendEksWrHandler(
				InstancePtr->RxInstance.
				IntrHdcp22SkeSendEksWrCallbackRef);
	}

	/* HDCP22 H' read done*/
	if ((IntrStatus2 &
			XDP_RX_INTERRUPT_MASK_HDCP22_HPRIME_READ_MASK) &&
			InstancePtr->RxInstance.
			IntrHdcp22HprimeReadDoneHandler) {
		InstancePtr->RxInstance.IntrHdcp22HprimeReadDoneHandler(
				InstancePtr->RxInstance.
				IntrHdcp22HprimeReadDoneCallbackRef);
	}

	/* HDCP22 Pairing Info read Done*/
	if ((IntrStatus2 &
			XDP_RX_INTERRUPT_MASK_HDCP22_PAIRING_INFO_READ_MASK)
			&& InstancePtr->RxInstance.
			IntrHdcp22PairingReadDoneHandler) {
		InstancePtr->RxInstance.IntrHdcp22PairingReadDoneHandler(
				InstancePtr->RxInstance.
				IntrHdcp22PairingReadDoneCallbackRef);
	}

	/* A write to the HDCP22 TYPE register has been performed. */
	if ((IntrStatus2 &
			XDP_RX_INTERRUPT_MASK_HDCP22_STREAM_TYPE_MASK)
			&& InstancePtr->RxInstance.
			IntrHdcp22StreamTypeWrHandler) {
		InstancePtr->RxInstance.IntrHdcp22StreamTypeWrHandler(
				InstancePtr->RxInstance.
				IntrHdcp22StreamTypeWrCallbackRef);
	}

	/* A write to the HDCP22 rcvID Lst Ack register has been performed. */
	if ((IntrStatus2 &
			XDP_RX_INTERRUPT_MASK_HDCP22_RPTR_RCVID_LST_ACK_MASK)
			&& InstancePtr->RxInstance.
			IntrHdcp22RepeatAuthRcvIdLstAckWrHandler) {
		InstancePtr->RxInstance.IntrHdcp22RepeatAuthRcvIdLstAckWrHandler(
				InstancePtr->RxInstance.
				IntrHdcp22RepeatAuthRcvIdLstAckWrCallbackRef);
	}

	/* A write to the HDCP22 Stream manage register has been performed. */
	if ((IntrStatus2 &
			XDP_RX_INTERRUPT_MASK_HDCP22_RPTR_STREAM_MANAGE_MASK)
			&& InstancePtr->RxInstance.
			IntrHdcp22RepeatAuthStreamMangWrHandler) {
		InstancePtr->RxInstance.IntrHdcp22RepeatAuthStreamMangWrHandler(
				InstancePtr->RxInstance.
				IntrHdcp22RepeatAuthStreamMangWrCallbackRef);
	}
#endif /*XPAR_XHDCP22_RX_NUM_INSTANCES*/

	/* An unplug event has occurred. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_UNPLUG_MASK) &&
			InstancePtr->RxInstance.IntrUnplugHandler) {
		InstancePtr->RxInstance.IntrUnplugHandler(
			InstancePtr->RxInstance.IntrUnplugCallbackRef);
	}

	/* DP 1.4 related interrupt handling */
	if (InstancePtr->Config.DpProtocol == XDP_PROTOCOL_DP_1_4) {
		/* The VerticalBlanking_Flag in the VB-ID field of the received
		 * stream 2 indicates the start of the vertical blanking
		 * interval. */
		if ((IntrStatus1 &
			XDP_RX_INTERRUPT_MASK_1_VBLANK_STREAM234_MASK(
				XDP_RX_STREAM_ID2)) &&
			InstancePtr->RxInstance.IntrVBlankHandler[1]) {
			InstancePtr->RxInstance.IntrVBlankHandler[1](
					InstancePtr->RxInstance.
					IntrVBlankCallbackRef[1]);
		}

		/* The VerticalBlanking_Flag in the VB-ID field of the received
		 * stream 3 indicates the start of the vertical blanking
		 * interval. */
		if ((IntrStatus1 &
			XDP_RX_INTERRUPT_MASK_1_VBLANK_STREAM234_MASK(
				XDP_RX_STREAM_ID3)) &&
			InstancePtr->RxInstance.IntrVBlankHandler[2]) {
			InstancePtr->RxInstance.IntrVBlankHandler[2](
					InstancePtr->RxInstance.
					IntrVBlankCallbackRef[2]);
		}

		/* The VerticalBlanking_Flag in the VB-ID field of the received
		 * stream 4 indicates the start of the vertical blanking
		 * interval. */
		if ((IntrStatus1 &
			XDP_RX_INTERRUPT_MASK_1_VBLANK_STREAM234_MASK(
				XDP_RX_STREAM_ID4)) &&
			InstancePtr->RxInstance.IntrVBlankHandler[3]) {
			InstancePtr->RxInstance.IntrVBlankHandler[3](
					InstancePtr->RxInstance.
					IntrVBlankCallbackRef[3]);
		}

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
		/* Adaptive-Sync SDP packet event received for stream 1 */
		if ((IntrStatus2 &
			XDP_RX_INTERRUPT_ADAPTIVE_SYNC_SDP_STREAMX_MASK(
			XDP_RX_STREAM_ID1)) &&
			InstancePtr->RxInstance.IntrAdapatveSyncSdpHandler[0]) {
			InstancePtr->RxInstance.IntrAdapatveSyncSdpHandler[0](
				InstancePtr->RxInstance.
				IntrAdapatveSyncSdpCallbackRef[0]);
		}
		/* Adaptive-Sync SDP packet event received for stream 2 */
		if ((IntrStatus2 &
			XDP_RX_INTERRUPT_ADAPTIVE_SYNC_SDP_STREAMX_MASK(
			XDP_RX_STREAM_ID2)) &&
			InstancePtr->RxInstance.IntrAdapatveSyncSdpHandler[1]) {
			InstancePtr->RxInstance.IntrAdapatveSyncSdpHandler[1](
				InstancePtr->RxInstance.
				IntrAdapatveSyncSdpCallbackRef[1]);
		}
		/* Adaptive-Sync SDP packet event received for stream 3 */
		if ((IntrStatus2 &
			XDP_RX_INTERRUPT_ADAPTIVE_SYNC_SDP_STREAMX_MASK(
			XDP_RX_STREAM_ID3)) &&
			InstancePtr->RxInstance.IntrAdapatveSyncSdpHandler[2]) {
			InstancePtr->RxInstance.IntrAdapatveSyncSdpHandler[2](
				InstancePtr->RxInstance.
				IntrAdapatveSyncSdpCallbackRef[2]);
		}
		/* Adaptive-Sync SDP packet event received for stream 4 */
		if ((IntrStatus2 &
			XDP_RX_INTERRUPT_ADAPTIVE_SYNC_SDP_STREAMX_MASK(
			XDP_RX_STREAM_ID4)) &&
			InstancePtr->RxInstance.IntrAdapatveSyncSdpHandler[3]) {
			InstancePtr->RxInstance.IntrAdapatveSyncSdpHandler[3](
				InstancePtr->RxInstance.
				IntrAdapatveSyncSdpCallbackRef[3]);
		}
		/* vblank difference event detected for stream 1 */
		if ((IntrStatus2 &
			XDP_RX_INTERRUPT_ADAPTIVE_SYNC_VB_STREAMX_MASK(
			XDP_RX_STREAM_ID1)) &&
			InstancePtr->RxInstance.IntrAdaptiveSyncVbHandler[0]) {
			InstancePtr->RxInstance.IntrAdaptiveSyncVbHandler[0](
				InstancePtr->RxInstance.
				IntrAdaptiveSyncVbCallbackRef[0]);
		}
		/* vblank difference event detected for stream 2 */
		if ((IntrStatus2 &
			XDP_RX_INTERRUPT_ADAPTIVE_SYNC_VB_STREAMX_MASK(
			XDP_RX_STREAM_ID2)) &&
			InstancePtr->RxInstance.IntrAdaptiveSyncVbHandler[1]) {
			InstancePtr->RxInstance.IntrAdaptiveSyncVbHandler[1](
				InstancePtr->RxInstance.
				IntrAdaptiveSyncVbCallbackRef[1]);
		}
		/* vblank difference event detected for stream 3 */
		if ((IntrStatus2 &
			XDP_RX_INTERRUPT_ADAPTIVE_SYNC_VB_STREAMX_MASK(
			XDP_RX_STREAM_ID3)) &&
			InstancePtr->RxInstance.IntrAdaptiveSyncVbHandler[2]) {
			InstancePtr->RxInstance.IntrAdaptiveSyncVbHandler[2](
				InstancePtr->RxInstance.
				IntrAdaptiveSyncVbCallbackRef[2]);
		}
		/* vblank difference event detected for stream 4 */
		if ((IntrStatus2 &
			XDP_RX_INTERRUPT_ADAPTIVE_SYNC_VB_STREAMX_MASK(
			XDP_RX_STREAM_ID4)) &&
			InstancePtr->RxInstance.IntrAdaptiveSyncVbHandler[3]) {
			InstancePtr->RxInstance.IntrAdaptiveSyncVbHandler[3](
				InstancePtr->RxInstance.
				IntrAdaptiveSyncVbCallbackRef[3]);
		}


	}

}
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */
/** @} */
