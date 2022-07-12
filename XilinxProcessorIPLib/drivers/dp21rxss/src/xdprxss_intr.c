/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdprxss_intr.c
* @addtogroup dprxss_v8_0
* @{
*
* This file contains interrupt related functions of Xilinx DisplayPort RX
* Subsystem core. Please see xdprxss.h for more details of the core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------
* 1.00 sha 05/18/15 Initial release.
* 2.00 sha 10/05/15 Removed setting HDCP callbacks.
*                   Added HDCP and Timer Counter interrupt handler.
* 3.0  sha 02/19/16 Added switch case for XDPRXSS_HANDLER_HDCP_RPTR_TDSA_EVENT
*                   to register callback with HDCP.
* 4.0  aad 12/01/16 Added HDCP Authentication interrupt handler
* 4.1  tu  09/06/17 Added three driver side interrupt handler for Video,
*                   NoVideo and PowerChange event
* 4.1  jb  02/19/19 Added support for HDCP22.
* 6.2  rg  03/25/21 Added support for adaptive-sync in MST
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdprxss.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the DisplayPort RX core operating
* in RX mode.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XDpRxSs_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpRxSs_DpIntrHandler(void *InstancePtr)
{
	XDpRxSs *XDpRxSsPtr = (XDpRxSs *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDpRxSsPtr != NULL);
	Xil_AssertVoid(XDpRxSsPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* DisplayPort RX interrupt handler */
	XDp_InterruptHandler(XDpRxSsPtr->DpPtr);
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDCP Cipher core.
*
* The application is responsible for connecting this function to the interrupt
* system.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpRxSs_HdcpIntrHandler(void *InstancePtr)
{
	XDpRxSs *XDpRxSsPtr = (XDpRxSs *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDpRxSsPtr != NULL);
	Xil_AssertVoid(XDpRxSsPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* HDCP Cipher interrupt handler */
	XHdcp1x_CipherIntrHandler(XDpRxSsPtr->Hdcp1xPtr);
}

/*****************************************************************************/
/**
 * This function is the interrupt handler for HDCP22 LIC failure.
 *
 * The application is responsible for connecting this function to the interrupt
 * system.
 *
 * @param	InstancePtr is a pointer to the XDpRxSs core instance that
 * 			just interrupted.
 *
 * @return None.
 *
 * @note   None.
 ******************************************************************************/
void XDpRxSs_Hdcp22LicFailHandler(void *InstancePtr)
{
	XDpRxSs *XDpRxSsPtr = (XDpRxSs *)InstancePtr;

	Xil_AssertVoid(InstancePtr);

	if (XDpRxSsPtr->Hdcp22Ptr) {
		if (XDpRxSsPtr->HdcpProtocol == XDPRXSS_HDCP_22) {
			XHdcp22Rx_Dp_SetLinkError(XDpRxSsPtr->Hdcp22Ptr);
		}
	}
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the Timer Counter core.
*
* The application is responsible for connecting this function to the interrupt
* system.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpRxSs_TmrCtrIntrHandler(void *InstancePtr)
{
	XDpRxSs *XDpRxSsPtr = (XDpRxSs *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDpRxSsPtr != NULL);
	Xil_AssertVoid(XDpRxSsPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Timer Counter interrupt handler */
	XTmrCtr_InterruptHandler(XDpRxSsPtr->TmrCtrPtr);
}

/*****************************************************************************/
/**
 *
 * This function is the interrupt handler for No Video
 *
 * @param       InstancePtr is a pointer to the XDpRxSs core instance that
 *              just interrupted.
 *
 * @return      None.
 *
 * @note        None.
 *
 ******************************************************************************/
void XDpRxSs_DrvNoVideoHandler(void *InstancePtr)
{
	XDpRxSs *XDpRxSsPtr = (XDpRxSs *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDpRxSsPtr != NULL);
	Xil_AssertVoid(XDpRxSsPtr->IsReady == XIL_COMPONENT_IS_READY);

	XDp_WriteReg(XDpRxSsPtr->DpPtr->Config.BaseAddr,
			XDP_RX_VIDEO_UNSUPPORTED, 0x1);
	XDp_WriteReg(XDpRxSsPtr->DpPtr->Config.BaseAddr,
			XDP_RX_AUDIO_UNSUPPORTED, 0x1);

}

/*****************************************************************************/
/**
 *
 * This function is for the video interrupt handler
 *
 * @param      InstancePtr is a pointer to the XDpRxSs core instance that
 *             just interrupted.
 *
 * @return     None.
 *
 * @note       None.
 *
 ******************************************************************************/
void XDpRxSs_DrvVideoHandler(void *InstancePtr)
{
	XDpRxSs *XDpRxSsPtr = (XDpRxSs *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDpRxSsPtr != NULL);
	Xil_AssertVoid(XDpRxSsPtr->IsReady == XIL_COMPONENT_IS_READY);

	XDp_WriteReg(XDpRxSsPtr->DpPtr->Config.BaseAddr,
			XDP_RX_VIDEO_UNSUPPORTED, 0x0);
	XDp_WriteReg(XDpRxSsPtr->DpPtr->Config.BaseAddr,
			XDP_RX_AUDIO_UNSUPPORTED, 0x0);
}

/*****************************************************************************/
/**
*
* This function is for the power change interrupt handler
*
* @param       InstancePtr is a pointer to the XDpRxSs core instance that
*              just interrupted.
*
* @return      None.
*
* @note        None.
*
******************************************************************************/
void XDpRxSs_DrvPowerChangeHandler(void *InstancePtr)
{
       XDpRxSs *XDpRxSsPtr = (XDpRxSs *)InstancePtr;
       u32 Rdata;
       const u32 PowerDownMode = 0x2;

       /* Verify arguments. */
       Xil_AssertVoid(XDpRxSsPtr != NULL);
       Xil_AssertVoid(XDpRxSsPtr->IsReady == XIL_COMPONENT_IS_READY);

       Rdata = XDp_ReadReg(XDpRxSsPtr->DpPtr->Config.BaseAddr,
                       XDP_RX_DPCD_SET_POWER_STATE);

       if (Rdata == PowerDownMode) {
               XDp_RxInterruptDisable(XDpRxSsPtr->DpPtr,
                               XDP_RX_INTERRUPT_MASK_UNPLUG_MASK);
       }
}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType                              Callback Function Type
* ---------------------------------------- -----------------------------------
* XDPRXSS_HANDLER_DP_VM_CHG_EVENT          XDp_RxSetIntrVmChangeHandler
* XDPRXSS_HANDLER_DP_PWR_CHG_EVENT         XDp_RxSetIntrPowerStateHandler
* XDPRXSS_HANDLER_DP_NO_VID_EVENT          XDp_RxSetIntrNoVideoHandler
* XDPRXSS_HANDLER_DP_VBLANK_EVENT          XDp_RxSetIntrVBlankHandler
* XDPRXSS_HANDLER_DP_TLOST_EVENT           XDp_RxSetIntrTrainingLostHandler
* XDPRXSS_HANDLER_DP_VID_EVENT             XDp_RxSetIntrVideoHandler
* XDPRXSS_HANDLER_DP_INFO_PKT_EVENT        XDp_RxSetIntrInfoPktHandler
* XDPRXSS_HANDLER_DP_EXT_PKT_EVENT         XDp_RxSetIntrExtPktHandler
* XDPRXSS_HANDLER_DP_TDONE_EVENT           XDp_RxSetIntrTrainingDoneHandler
* XDPRXSS_HANDLER_DP_BW_CHG_EVENT          XDp_RxSetIntrBwChangeHandler
* XDPRXSS_HANDLER_DP_DWN_REQ_EVENT         XDp_RxSetIntrDownReqHandler
* XDPRXSS_HANDLER_DP_DWN_REP_EVENT         XDp_RxSetIntrDownReplyHandler
* XDPRXSS_HANDLER_DP_AUD_OVRFLW_EVENT      XDp_RxSetIntrAudioOverHandler
* XDPRXSS_HANDLER_DP_PAYLOAD_ALLOC_EVENT   XDp_RxSetIntrPayloadAllocHandler
* XDPRXSS_HANDLER_DP_ACT_RX_EVENT          XDp_RxSetIntrActRxHandler
* XDPRXSS_HANDLER_DP_CRC_TEST_EVENT        XDp_RxSetIntrCrcTestHandler
* XDPRXSS_HANDLER_HDCP_RPTR_TDSA_EVENT     XHdcp1x_SetCallBack
* XDPRXSS_HANDLER_UNPLUG_EVENT             UnplugCallback
* XDPRXSS_HANDLER_LINKBW_EVENT             LinkBwCallback
* XDPRXSS_HANDLER_PLL_RESET_EVENT          PllResetCallback
* </pre>
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
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
u32 XDpRxSs_SetCallBack(XDpRxSs *InstancePtr, u32 HandlerType,
			void *CallbackFunc, void *CallbackRef)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(HandlerType >= XDPRXSS_HANDLER_DP_VM_CHG_EVENT);
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	/* Assign callback based on handler type */
	switch (HandlerType) {
		case XDPRXSS_HANDLER_DP_VM_CHG_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_VMCHANGE,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_VM_CHG_STREAM_2_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_VMCHANGE_STREAM_2,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_VM_CHG_STREAM_3_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_VMCHANGE_STREAM_3,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_VM_CHG_STREAM_4_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_VMCHANGE_STREAM_4,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_PWR_CHG_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_PWRSTATECHANGE,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_DRV_HANDLER_DP_PWR_CHG_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_DRV_PWRSTATE,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_NO_VID_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_NOVIDEO,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_NO_VID_STREAM_2_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_NOVIDEO_STREAM_2,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_NO_VID_STREAM_3_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_NOVIDEO_STREAM_3,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_NO_VID_STREAM_4_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_NOVIDEO_STREAM_4,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_DRV_HANDLER_DP_NO_VID_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_DRV_NOVIDEO,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_VBLANK_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_VBLANK,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_VBLANK_STREAM_2_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_VBLANK_STREAM_2,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_VBLANK_STREAM_3_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_VBLANK_STREAM_3,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_VBLANK_STREAM_4_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_VBLANK_STREAM_4,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_TLOST_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_TRAININGLOST,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_VID_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_VIDEO,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_VID_STREAM_2_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_VIDEO_STREAM_2,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_VID_STREAM_3_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_VIDEO_STREAM_3,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_VID_STREAM_4_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_VIDEO_STREAM_4,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_DRV_HANDLER_DP_VID_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_DRV_VIDEO,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_INFO_PKT_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_AUD_INFOPKTRECV,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_INFO_PKT_STREAM_2_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_AUD_INFOPKTRECV_STREAM_2,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_INFO_PKT_STREAM_3_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_AUD_INFOPKTRECV_STREAM_3,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_INFO_PKT_STREAM_4_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_AUD_INFOPKTRECV_STREAM_4,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_EXT_PKT_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_AUD_EXTPKTRECV,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_EXT_PKT_STREAM_2_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_AUD_EXTPKTRECV_STREAM_2,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_EXT_PKT_STREAM_3_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_AUD_EXTPKTRECV_STREAM_3,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_EXT_PKT_STREAM_4_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_AUD_EXTPKTRECV_STREAM_4,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;


		case XDPRXSS_HANDLER_DP_TDONE_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_TRAININGDONE,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_BW_CHG_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_BWCHANGE,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_DWN_REQ_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_DOWNREQ,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_DWN_REP_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_DOWNREPLY,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_AUD_OVRFLW_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_AUD_PKTOVERFLOW,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_PAYLOAD_ALLOC_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_PAYLOADALLOC,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_ACT_RX_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_ACT_SEQ,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_CRC_TEST_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_CRC_TEST,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_HDCP_RPTR_TDSA_EVENT:
			XHdcp1x_SetCallBack(InstancePtr->Hdcp1xPtr,
				XHDCP1X_RPTR_HDLR_TRIG_DOWNSTREAM_AUTH,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_HDCP_AUTHENTICATED:
			XHdcp1x_SetCallBack(InstancePtr->Hdcp1xPtr,
				XHDCP1X_HANDLER_AUTHENTICATED,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;
		case XDPRXSS_HANDLER_HDCP22_AUTHENTICATED:
			if (InstancePtr->Hdcp22Ptr) {
				XHdcp22Rx_Dp_SetCallback(InstancePtr->Hdcp22Ptr,
					XHDCP22_RX_HANDLER_AUTHENTICATED,
					(void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
					(void *)CallbackRef);
				Status = XST_SUCCESS;
			} else {
				Status = XST_FAILURE;
			}
			break;

		case XDPRXSS_HANDLER_HDCP22_UNAUTHENTICATED:
			if (InstancePtr->Hdcp22Ptr) {
				XHdcp22Rx_Dp_SetCallback(InstancePtr->Hdcp22Ptr,
					XHDCP22_RX_HANDLER_UNAUTHENTICATED,
					(void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
					(void *)CallbackRef);
				Status = XST_SUCCESS;
			} else {
				Status = XST_FAILURE;
			}
			break;
		case XDPRXSS_HANDLER_HDCP22_SKE_SEND_EKS:
			if (InstancePtr->Hdcp22Ptr) {
				XHdcp22Rx_Dp_SetCallback(InstancePtr->Hdcp22Ptr,
						XHDCP22_RX_HANDLER_SKE_SEND_EKS,
					(void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
					(void *)CallbackRef);
				Status = XST_SUCCESS;
			} else {
				Status = XST_FAILURE;
			}
			break;
		case XDPRXSS_HANDLER_HDCP22_AUTHENTICATION_REQUEST:
			if (InstancePtr->Hdcp22Ptr) {
				XHdcp22Rx_Dp_SetCallback(InstancePtr->Hdcp22Ptr,
					XHDCP22_RX_HANDLER_AUTHENTICATION_REQUEST,
					(void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
					(void *)CallbackRef);
				Status = XST_SUCCESS;
			} else {
				Status = XST_FAILURE;
			}
			break;

		case XDPRXSS_HANDLER_HDCP22_ENCRYPTION_UPDATE:
			if (InstancePtr->Hdcp22Ptr) {
				XHdcp22Rx_Dp_SetCallback(InstancePtr->Hdcp22Ptr,
					XHDCP22_RX_HANDLER_ENCRYPTION_UPDATE,
					(void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
					(void *)CallbackRef);
				Status = XST_SUCCESS;
			} else {
				Status = XST_FAILURE;
			}
			break;
		case XDPRXSS_HANDLER_UNPLUG_EVENT:
			InstancePtr->UnplugCallback =
				(XDpRxSs_Callback)((void *)CallbackFunc);
			InstancePtr->UnplugRef = CallbackRef;
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_LINKBW_EVENT:
			InstancePtr->LinkBwCallback =
				(XDpRxSs_Callback)((void *)CallbackFunc);
			InstancePtr->LinkBwRef = CallbackRef;
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_PLL_RESET_EVENT:
			InstancePtr->PllResetCallback =
				(XDpRxSs_Callback)((void *)CallbackFunc);
			InstancePtr->PllResetRef = CallbackRef;
			Status = XST_SUCCESS;
			break;

		/* DP 1.4 callback(s) */
		case XDPRXSS_HANDLER_ACCESS_LANE_SET_EVENT:
			if (InstancePtr->DpPtr->Config.DpProtocol ==
			    XDP_PROTOCOL_DP_1_4) {
				XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_ACCESS_LANE_SET,
					CallbackFunc, CallbackRef);
				Status = XST_SUCCESS;
			} else {
				Status = XST_FAILURE;
			}
			break;

		case XDPRXSS_HANDLER_ACCESS_LINK_QUAL_EVENT:
			if (InstancePtr->DpPtr->Config.DpProtocol ==
			    XDP_PROTOCOL_DP_1_4) {
				XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_ACCESS_LINK_QUAL,
					CallbackFunc, CallbackRef);
				Status = XST_SUCCESS;
			} else {
				Status = XST_FAILURE;
			}
			break;

		case XDPRXSS_HANDLER_ACCESS_ERROR_COUNTER_EVENT:
			if (InstancePtr->DpPtr->Config.DpProtocol ==
			    XDP_PROTOCOL_DP_1_4) {
				XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_ACCESS_ERR_COUNTER,
					CallbackFunc, CallbackRef);
				Status = XST_SUCCESS;
			} else {
				Status = XST_FAILURE;
			}
			break;

		case XDPRXSS_HANDLER_DP_ADAPTIVESYNC_SDP_STREAM_1_EVENT:
			if (InstancePtr->DpPtr->Config.DpProtocol ==
				XDP_PROTOCOL_DP_1_4) {
				XDp_RxSetCallback(InstancePtr->DpPtr,
					XDP_RX_HANDLER_ADAPTIVE_SYNC_SDP,
					CallbackFunc, CallbackRef);
				Status = XST_SUCCESS;
			} else {
				Status = XST_FAILURE;
			}
			break;
		case XDPRXSS_HANDLER_DP_ADAPTIVESYNC_SDP_STREAM_2_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
				XDP_RX_HANDLER_ADAPTIVE_SYNC_SDP_STREAM_2,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;
		case XDPRXSS_HANDLER_DP_ADAPTIVESYNC_SDP_STREAM_3_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
				XDP_RX_HANDLER_ADAPTIVE_SYNC_SDP_STREAM_3,
				CallbackFunc, CallbackRef);
				Status = XST_SUCCESS;
			break;
		case XDPRXSS_HANDLER_DP_ADAPTIVESYNC_SDP_STREAM_4_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
				XDP_RX_HANDLER_ADAPTIVE_SYNC_SDP_STREAM_4,
				CallbackFunc, CallbackRef);
				Status = XST_SUCCESS;
			break;
		case XDPRXSS_HANDLER_DP_ADAPTIVESYNC_VBLANK_STREAM_1_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
				XDP_RX_HANDLER_ADAPTIVE_SYNC_VBLANK,
				CallbackFunc, CallbackRef);
				Status = XST_SUCCESS;
			break;
		case XDPRXSS_HANDLER_DP_ADAPTIVESYNC_VBLANK_STREAM_2_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
				XDP_RX_HANDLER_ADAPTIVE_SYNC_VBLANK_STREAM_2,
				CallbackFunc, CallbackRef);
				Status = XST_SUCCESS;
			break;
		case XDPRXSS_HANDLER_DP_ADAPTIVESYNC_VBLANK_STREAM_3_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
				XDP_RX_HANDLER_ADAPTIVE_SYNC_VBLANK_STREAM_3,
				CallbackFunc, CallbackRef);
				Status = XST_SUCCESS;
			break;
		case XDPRXSS_HANDLER_DP_ADAPTIVESYNC_VBLANK_STREAM_4_EVENT:
			XDp_RxSetCallback(InstancePtr->DpPtr,
				XDP_RX_HANDLER_ADAPTIVE_SYNC_VBLANK_STREAM_4,
				CallbackFunc, CallbackRef);
				Status = XST_SUCCESS;
			break;

		/* End of setting DP 1.4 callback(s) */

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
* DisplayPort RX Subsystem.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
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
void XDpRxSs_SetUserTimerHandler(XDpRxSs *InstancePtr,
		XDpRxSs_TimerHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	/* Set custom timer wait handler */
	XDp_SetUserTimerHandler(InstancePtr->DpPtr, CallbackFunc, CallbackRef);
}
/** @} */
