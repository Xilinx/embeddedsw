/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdptxss_intr.c
* @addtogroup dptxss_v6_7
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
* 5.0  jb  02/21/19 Added HDCP22 callback handles.
* 					Made the Timer counter interrupt handler available
* 					for both HDCP1x and 22.
* 					Added switch case for
* 6.5  rg  09/01/20 Added switch case XDPTXSS_HANDLER_DP_EXT_PKT_EVENT
*                   to register callback with extended packet transmit
*                   done handler.
* 6.4  rg  09/26/20 Added driver handler function XDpTxSs_WriteVscExtPktProcess
*		    for programming the extended packet up on receiving extended
*		    packet transmission done interrupt.
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
	u8 ext_cap = 0;
	u8 ext_dpcd_rd = 0;
	u8 Dpcd_ext [16];
	XDpTxSs *XDpTxSsPtr = (XDpTxSs *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDpTxSsPtr != NULL);
	Xil_AssertVoid(XDpTxSsPtr->IsReady == XIL_COMPONENT_IS_READY);
	XDpTxSs_UsrHpdEventData *UsrHpdEventData = &XDpTxSsPtr->UsrHpdEventData;

	if (XDp_TxIsConnected(XDpTxSsPtr->DpPtr)) {
		/* From here is the requirement per DP spec */
		Status = XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
				XDP_DPCD_MAX_LINK_RATE, 1,
				&UsrHpdEventData->MaxCapNew);
		Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
				XDP_DPCD_MAX_LANE_COUNT, 1,
				&UsrHpdEventData->MaxCapLanesNew);
		Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
				XDP_DPCD_TRAIN_AUX_RD_INTERVAL, 1, &ext_cap);

		if(ext_cap & 0x80) {
			/* if EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled */
			XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
				XDP_EDID_DPCD_MAX_LINK_RATE, 1, &ext_cap);
			if(ext_cap == XDP_DPCD_LINK_BW_SET_810GBPS){
				UsrHpdEventData->MaxCapNew = 0x1E;
			}

			/* UCD400 required reading these extended registers */
			XDp_TxAuxRead(XDpTxSsPtr->DpPtr,XDP_DPCD_SINK_COUNT_ESI,
					1, &ext_dpcd_rd);
			XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
				XDP_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI0,
				1, &ext_dpcd_rd);
			XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
				XDP_DPCD_SINK_LANE0_1_STATUS, 1, &ext_dpcd_rd);
			XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
				XDP_DPCD_SINK_LANE2_3_STATUS, 1, &ext_dpcd_rd);
			XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
				XDP_DPCD_SINK_ALIGN_STATUS_UPDATED_ESI,
				1, &ext_dpcd_rd);
			XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
				XDP_DPCD_SINK_STATUS_ESI, 1, &ext_dpcd_rd);
			XDp_TxAuxRead(XDpTxSsPtr->DpPtr, 0x2200, 16, &Dpcd_ext);

			if ((Dpcd_ext[5] & 0x1)) {
				Status = XDp_TxAuxRead(XDpTxSsPtr->DpPtr, 0x0080,
						16, &Dpcd_ext);
			}
		}

		Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
				XDP_DPCD_REV, 12, UsrHpdEventData->Tmp);
		Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
				XDP_DPCD_SINK_COUNT, 6,	UsrHpdEventData->Tmp);
		Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_REV, 11,
				&UsrHpdEventData->Dpcd);
		Status |= XDp_TxGetEdidBlock(XDpTxSsPtr->DpPtr,
				UsrHpdEventData->EdidOrg, 0);

		if (UsrHpdEventData->EdidOrg[XDP_EDID_EXT_BLOCK_COUNT] > 0)
                       Status |= XDp_TxGetEdidBlock(XDpTxSsPtr->DpPtr,
                                                    UsrHpdEventData->EdidOrg_1,
                                                    1);
		if (UsrHpdEventData->EdidOrg_1[XDP_EDID_EXT_BLOCK_COUNT] > 1)
                        Status |= XDp_TxGetEdidBlock(XDpTxSsPtr->DpPtr,
                                                     UsrHpdEventData->EdidOrg_2,
                                                     2);

		Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_SINK_COUNT,
				1, &UsrHpdEventData->Rd200);
		Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
				XDP_DPCD_STATUS_LANE_0_1, 1,
				&UsrHpdEventData->Lane0Sts);
		Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
				XDP_DPCD_STATUS_LANE_2_3, 1,
				&UsrHpdEventData->Lane2Sts);

		if(Status != XST_SUCCESS){
			xdbg_printf(XDBG_DEBUG_GENERAL, "AUX access had trouble!\r\n");
		}
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
	u8 ext_cap = 0;
	u8 ext_dpcd_rd = 0;


	/* Verify arguments. */
	Xil_AssertVoid(XDpTxSsPtr != NULL);
	Xil_AssertVoid(XDpTxSsPtr->IsReady == XIL_COMPONENT_IS_READY);
	XDpTxSs_UsrHpdPulseData *UsrHpdPulseData = &XDpTxSsPtr->UsrHpdPulseData;

	if (XDp_TxIsConnected(XDpTxSsPtr->DpPtr)) {

	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr,
			 XDP_DPCD_SINK_COUNT, 6, UsrHpdPulseData->AuxValues);

	if ((UsrHpdPulseData->AuxValues[4] & 0x40)) {
		XDp_TxDisableMainLink(XDpTxSsPtr->DpPtr);
	}

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

	Status |= XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_TRAIN_AUX_RD_INTERVAL, 1,
			&ext_cap);

	/* UCD400 required reading these extended registers */
	if(ext_cap & 0x80){
		/* if EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled */
		XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_EDID_DPCD_MAX_LINK_RATE,
			1, &ext_cap);
		XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_SINK_COUNT_ESI,
			1, &ext_dpcd_rd);
		XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI0,
			1, &ext_dpcd_rd);
		XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_SINK_LANE0_1_STATUS,
			1, &ext_dpcd_rd);
		XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_SINK_LANE2_3_STATUS,
			1, &ext_dpcd_rd);
		XDp_TxAuxRead(XDpTxSsPtr->DpPtr, XDP_DPCD_SINK_ALIGN_STATUS_UPDATED_ESI,
			1, &ext_dpcd_rd);
		XDp_TxAuxRead(XDpTxSsPtr->DpPtr,XDP_DPCD_SINK_STATUS_ESI,
			1, &ext_dpcd_rd);
	}

	Status |= XDp_TxGetEdidBlock(XDpTxSsPtr->DpPtr,
			UsrHpdPulseData->Edid, 0);

	if(Status != XST_SUCCESS){
		xdbg_printf(XDBG_DEBUG_GENERAL, "AUX access had trouble!\r\n");
	}
	}
}

/*****************************************************************************/
/**
*
* This function writes the pixel encoding format and colorimetry formats in to
* the extended packet registers at DP TX offset addresses "0x0330 to 0x0350".
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_WriteVscExtPktProcess(void *InstancePtr)
{
	int i;
	XDpTxSs *XDpTxSsPtr = (XDpTxSs *)InstancePtr;
	XDp *XDpPtr = XDpTxSsPtr->DpPtr;

	if (XDpPtr->TxInstance.ColorimetryThroughVsc) {
		XDp_WriteReg(XDpPtr->Config.BaseAddr, XDP_TX_AUDIO_EXT_DATA(1),
			XDpPtr->TxInstance.VscPacket.Header);
		for (i = 0; i < XDPTXSS_EXT_DATA_2ND_TO_9TH_WORD; i++) {
			XDp_WriteReg(XDpPtr->Config.BaseAddr, XDP_TX_AUDIO_EXT_DATA(i+2),
					XDpPtr->TxInstance.VscPacket.Payload[i]);
		}
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
* (XDPTXSS_HANDLER_DP_EXT_PKT_EVENT)       XDP_TX_HANDLER_EXTPKT_TXD
* (XDPTXSS_DRV_HANDLER_DP_EXT_PKT_EVENT)   XDP_TX_HANDLER_DRV_EXTPKT_TXD
* (XDPTXSS_HANDLER_DP_VSYNC)               XDP_TX_HANDLER_VSYNC
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

		case XDPTXSS_HANDLER_DP_EXT_PKT_EVENT:
			XDp_TxSetCallback(InstancePtr->DpPtr,
				XDP_TX_HANDLER_EXTPKT_TXD,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPTXSS_DRV_HANDLER_DP_EXT_PKT_EVENT:
			XDp_TxSetCallback(InstancePtr->DpPtr,
					XDP_TX_HANDLER_DRV_EXTPKT_TXD,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPTXSS_HANDLER_DP_VSYNC:
			XDp_TxSetCallback(InstancePtr->DpPtr,
					XDP_TX_HANDLER_VSYNC,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPTXSS_HANDLER_HDCP_RPTR_EXCHG:
			XHdcp1x_SetCallBack(InstancePtr->Hdcp1xPtr,
				XHDCP1X_RPTR_HDLR_REPEATER_EXCHANGE,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPTXSS_HANDLER_HDCP22_AUTHENTICATED:
			/* Register HDCP 2.2 callbacks */
			XHdcp22Tx_Dp_SetCallback(InstancePtr->Hdcp22Ptr,
				XHDCP22_TX_HANDLER_AUTHENTICATED,
				(void *)(XHdcp22_Tx_Dp_Callback)CallbackFunc,
				(void *)CallbackRef);
			Status = XST_SUCCESS;
			break;
		case XDPTXSS_HANDLER_HDCP22_UNAUTHENTICATED:
			/** Register HDCP 2.2 callbacks */
			XHdcp22Tx_Dp_SetCallback(InstancePtr->Hdcp22Ptr,
				XHDCP22_TX_HANDLER_UNAUTHENTICATED,
				(void *)(XHdcp22_Tx_Dp_Callback)CallbackFunc,
				(void *)CallbackRef);
			Status = XST_SUCCESS;
			break;
		case XDPTXSS_HANDLER_HDCP22_UPDATE_DOWNSTREAM_TOPOLOGY:
			/** Register HDCP 2.2 callbacks */
			XHdcp22Tx_Dp_SetCallback(InstancePtr->Hdcp22Ptr,
					XHDCP22_TX_HANDLER_DOWNSTREAM_TOPOLOGY_AVAILABLE,
					(void *)(XHdcp22_Tx_Dp_Callback)CallbackFunc,
					(void *)CallbackRef);
			Status = XST_SUCCESS;
			break;

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
