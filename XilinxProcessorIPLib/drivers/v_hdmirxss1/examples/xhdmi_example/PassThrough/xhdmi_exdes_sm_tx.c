/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file
*
* This file defines the top level structure and APIs for the Xilinx
* Video Receiver system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
*              dd/mm/yy
* ----- ------ -------- --------------------------------------------------
* 1.00  YB     11/05/19 Initial release.
*</pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/
#include "xhdmi_exdes_sm_tx.h"

#define DEBUG_VCKE
#define DEBUG_DC
#define DEBUG_422

/************************** Constant Definitions ****************************/

static const XV_Tx_Hdmi_Events XHdmi_Tx_EventsPriorityQueue[XV_TX_HDMI_NUM_EVENTS] = {
		XV_TX_HDMI_EVENT_DISCONNECTED,
		XV_TX_HDMI_EVENT_CONNECTED,
		XV_TX_HDMI_EVENT_BRDGUNLOCKED,
		XV_TX_HDMI_EVENT_BRDGUNDERFLOW,
		XV_TX_HDMI_EVENT_BRDGOVERFLOW,
		XV_TX_HDMI_EVENT_VSYNC,
		XV_TX_HDMI_EVENT_STREAMUP,
		XV_TX_HDMI_EVENT_STREAMDOWN,
		XV_TX_HDMI_EVENT_POLL
};

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
XV_Tx xhdmi_example_tx_controller;

XV_Tx_Debug_Printf XV_Tx_DebugPrintf = NULL;
XV_Tx_Debug_Printf XV_TxDebugTxSMPrintf = NULL;
XV_Tx_Debug_Printf XV_TxDebugTxNewStreamSetupPrintf = NULL;

/************************** Function Prototypes *****************************/
#ifdef USE_HDCP_HDMI_TX
static void XV_Tx_Hdcp_Authenticated_Cb(void *CallbackRef);
static void XV_Tx_Hdcp_Unauthenticated_Cb(void *CallbackRef);
#endif

static void XV_Tx_HdmiTx_Connect_Cb(void *CallbackRef);
static void XV_Tx_HdmiTx_Toggle_Cb(void *CallbackRef);
static void XV_Tx_HdmiTx_BrdgUnlocked_Cb(void *CallbackRef);
static void XV_Tx_HdmiTx_BrdgUnderFlow_Cb(void *CallbackRef);
static void XV_Tx_HdmiTx_BrdgOverFlow_Cb(void *CallbackRef);
static void XV_Tx_HdmiTx_VSync_Cb(void *CallbackRef);
static void XV_Tx_HdmiTx_StreamDown_Cb(void *CallbackRef);
static void XV_Tx_HdmiTx_StreamUp_Cb(void *CallbackRef);
static void XV_Tx_HdmiTx_FrlConfig_Cb(void *CallbackRef);
static void XV_Tx_HdmiTx_FrlFfe_Cb(void *CallbackRef);
static void XV_Tx_HdmiTx_FrlStart_Cb(void *CallbackRef);
static void XV_Tx_HdmiTx_FrlStop_Cb(void *CallbackRef);
static void XV_Tx_HdmiTx_TmdsConfig_Cb(void *CallbackRef);

void XV_Tx_HdmiTx_StateDisconnected(XV_Tx *InstancePtr,
				XV_Tx_Hdmi_Events Event,
				XV_Tx_Hdmi_State *NextStatePtr);
void XV_Tx_HdmiTx_StateConnected(XV_Tx *InstancePtr,
				XV_Tx_Hdmi_Events Event,
				XV_Tx_Hdmi_State *NextStatePtr);
void XV_Tx_HdmiTx_StateBrdgUnlocked(XV_Tx *InstancePtr,
				XV_Tx_Hdmi_Events Event,
				XV_Tx_Hdmi_State *NextStatePtr);
void XV_Tx_HdmiTx_StateVSync(XV_Tx *InstancePtr,
				XV_Tx_Hdmi_Events Event,
				XV_Tx_Hdmi_State *NextStatePtr);
void XV_Tx_HdmiTx_StateStreamOn(XV_Tx *InstancePtr,
				XV_Tx_Hdmi_Events Event,
				XV_Tx_Hdmi_State *NextStatePtr);
void XV_Tx_HdmiTx_StateStreamOff(XV_Tx *InstancePtr,
				XV_Tx_Hdmi_Events Event,
				XV_Tx_Hdmi_State *NextStatePtr);
void XV_Tx_HdmiTx_StateFrlConfig(XV_Tx *InstancePtr,
				XV_Tx_Hdmi_Events Event,
				XV_Tx_Hdmi_State *NextStatePtr);
void XV_Tx_HdmiTx_StateFrlFfe(XV_Tx *InstancePtr,
				XV_Tx_Hdmi_Events Event,
				XV_Tx_Hdmi_State *NextStatePtr);
void XV_Tx_HdmiTx_StateFrlStart(XV_Tx *InstancePtr,
				XV_Tx_Hdmi_Events Event,
				XV_Tx_Hdmi_State *NextStatePtr);
void XV_Tx_HdmiTx_StateFrlStop(XV_Tx *InstancePtr,
				XV_Tx_Hdmi_Events Event,
				XV_Tx_Hdmi_State *NextStatePtr);
void XV_Tx_HdmiTx_StateTmdsConfig(XV_Tx *InstancePtr,
				XV_Tx_Hdmi_Events Event,
				XV_Tx_Hdmi_State *NextStatePtr);

/* *** Enter State *** */
void XV_Tx_HdmiTx_EnterStateDisconnected(XV_Tx *InstancePtr);
void XV_Tx_HdmiTx_EnterStateConnected(XV_Tx *InstancePtr);
void XV_Tx_HdmiTx_EnterStateBrdgUnlocked(XV_Tx *InstancePtr);
void XV_Tx_HdmiTx_EnterStateVSync(XV_Tx *InstancePtr);
void XV_Tx_HdmiTx_EnterStateStreamOn(XV_Tx *InstancePtr);
void XV_Tx_HdmiTx_EnterStateStreamOff(XV_Tx *InstancePtr);
void XV_Tx_HdmiTx_EnterStateFrlConfig(XV_Tx *InstancePtr);
void XV_Tx_HdmiTx_EnterStateFrlFfe(XV_Tx *InstancePtr);
void XV_Tx_HdmiTx_EnterStateFrlStart(XV_Tx *InstancePtr);
void XV_Tx_HdmiTx_EnterStateFrlStop(XV_Tx *InstancePtr);
void XV_Tx_HdmiTx_EnterStateTmdsConfig(XV_Tx *InstancePtr);

void XV_Tx_HdmiTx_ProcessPendingEvents(XV_Tx *InstancePtr);

void XV_Tx_HdmiTx_PushEvent(XV_Tx *InstancePtr,
			    XV_Tx_Hdmi_Events Event);
static void XV_Tx_HdmiTx_SendEvent(XV_Tx *InstancePtr,
				   XV_Tx_Hdmi_Events Event);

void XV_Tx_HdmiTx_ProcessEvents(XV_Tx *InstancePtr,
				XV_Tx_Hdmi_Events Event);
void XV_Tx_HdmiTx_StateEnter(XV_Tx *InstancePtr,
			     XV_Tx_Hdmi_State State);

const char *XV_Tx_Hdmi_Tx_StatetoString(XV_Tx_Hdmi_State State);
const char *XV_Tx_Hdmi_Tx_EventtoString(XV_Tx_Hdmi_Events Event);

/**************************** Type Definitions ******************************/

/************************** Function Definitions ****************************/

/**** **** **** **** **** ****  VPHY callbacks  **** **** **** **** **** ****/
/*****************************************************************************/
/**
*
* This function is called when the GT TX reference input clock has changed.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void Hdmiphy1HdmiTxInitCallback(void *CallbackRef)
{
	XV_Tx *txInst = (XV_Tx *)CallbackRef;
	xdbg_xv_tx_print("%s,%d,\r\n", __func__, __LINE__);
	XV_HdmiTxSs1_RefClockChangeInit(txInst->HdmiTxSs);
}

/*****************************************************************************/
/**
*
* This function is called when the GT TX has been initialized
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void Hdmiphy1HdmiTxReadyCallback(void *CallbackRef)
{
	xil_printf("%s,%d,\r\n", __func__, __LINE__);
}

void XV_Tx_VPhy_SetCallbacks(XV_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	XHdmiphy1 *Hdmiphy1Inst = InstancePtr->VidPhy;

	/* VPHY callback setup */
	XHdmiphy1_SetHdmiCallback(Hdmiphy1Inst,
				XHDMIPHY1_HDMI_HANDLER_TXINIT,
				(void *)Hdmiphy1HdmiTxInitCallback,
				(void *)InstancePtr);
	XHdmiphy1_SetHdmiCallback(Hdmiphy1Inst,
				XHDMIPHY1_HDMI_HANDLER_TXREADY,
				(void *)Hdmiphy1HdmiTxReadyCallback,
				(void *)InstancePtr);
}

/* * * * * * * * * * * * * * * * * VPHY * * * * * * * * * * * * * * * * * * */

void XV_Tx_SetDebugPrints(XV_Tx_Debug_Printf PrintFunc)
{
	XV_Tx_DebugPrintf = PrintFunc;
}

void XV_Tx_SetDebugStateMachinePrints(XV_Tx_Debug_Printf PrintFunc)
{
	XV_TxDebugTxSMPrintf = PrintFunc;
}

void XV_Tx_SetDebugTxNewStreamSetupPrints(XV_Tx_Debug_Printf PrintFunc)
{
	XV_TxDebugTxNewStreamSetupPrintf = PrintFunc;
}

void XV_Tx_Poll(XV_Tx *InstancePtr)
{
	XV_Tx_HdmiTx_ProcessPendingEvents(InstancePtr);
	/* Alternatively,
	 * XV_Tx_HdmiTx_ProcessEvents(InstancePtr, XV_TX_HDMI_EVENT_POLL);
	 */
}


void XV_Tx_Hdcp_Poll(XV_Tx *InstancePtr)
{
#ifdef USE_HDCP_HDMI_TX
	/* Ensuring two thing before polling HDCP :-
	 * 1. The HDCP IP is ready in the Tx SS.
	 * 2. !!DONOT!! wait for authentication to start on the
	 *    downstream interface to poll, as the polling
	 *    will process the connect, disconnect, stream up
	 *    and stream down events in the HDMI TX SS
	 *    HDCP state machine.
	 *
	 * If the user wants to change the instance/flow at which
	 * HDCP authentication starts they need to modify this
	 * part of the code, as we are starting HDCP authentication
	 * after the first Vsync is sent out and will poll the
	 * downstream after the stream is up.
	 */
	if (XV_HdmiTxSs1_HdcpIsReady(InstancePtr->HdmiTxSs)) {
		XV_HdmiTxSs1_HdcpPoll(InstancePtr->HdmiTxSs);
	}
#endif
}

void XV_Tx_SetHdcpAuthReqExclusion(XV_Tx *InstancePtr, u8 Set)
{
	InstancePtr->ExclHdcpAuthReqFlag = Set;
}

u8 XV_Tx_GetHdcpAuthReqExclusion(XV_Tx *InstancePtr)
{
	return InstancePtr->ExclHdcpAuthReqFlag;
}

#ifdef USE_HDCP_HDMI_TX
/*****************************************************************************/
/**
*
* This function is called to trigger authentication for each downstream
* interface.
*
* @param    InstancePtr is a pointer to the Hdmi Transmitter State
*           machine layer instance.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void XV_Tx_Hdcp_Authenticate(XV_Tx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

#ifdef CHECK_HDCP_CAPS_BEFORE_AUTH_REQUEST
	u8 DwnStrmIsHdcpCapable = FALSE;

	if (XV_HdmiTxSs1_IsSinkHdcp14Capable(InstancePtr->HdmiTxSs) ||
	    XV_HdmiTxSs1_IsSinkHdcp22Capable(InstancePtr->HdmiTxSs)) {
		DwnStrmIsHdcpCapable = TRUE;
	}
#endif

	if (InstancePtr->HdcpConfig.IsReady) {
		/* Downstream interface stream up */
		if (InstancePtr->HdcpConfig.DownstreamInstanceStreamUp == TRUE
#ifdef CHECK_HDCP_CAPS_BEFORE_AUTH_REQUEST
		    && DwnStrmIsHdcpCapable == TRUE
#endif
		   ) {
			/* Trigger authentication on Idle, or if stream has toggled. */
			if (!(XV_HdmiTxSs1_HdcpIsAuthenticated(InstancePtr->HdmiTxSs)) &&
			    !(XV_HdmiTxSs1_HdcpIsInProgress(InstancePtr->HdmiTxSs))) {
				/* The commented prints can be used to profile
				 * HDCP authentication on idle stream.
				 */
				/* xil_printf("   ->HDCP TX IDLE -----" */
				/*            "Push Auth ----->\r\n"); */
				XV_HdmiTxSs1_HdcpPushEvent(InstancePtr->HdmiTxSs,
						XV_HDMITXSS1_HDCP_AUTHENTICATE_EVT);
			} else if(XV_HdmiTxSs1_IsStreamToggled(InstancePtr->HdmiTxSs)) {
				/* The commented prints can be used to profile
				 * HDCP authentication on a toggle request.
				 */
				/* xil_printf("   ->HDCP TX TOGGLED -----" */
				/*            "Push Auth ----->\r\n"); */
				XV_HdmiTxSs1_HdcpPushEvent(InstancePtr->HdmiTxSs,
						XV_HDMITXSS1_HDCP_AUTHENTICATE_EVT);
			}
		}
	}
}

/*****************************************************************************/
/**
*
* This function is called to set the HDCP capability on the
* downstream interfaces. After the capability is set authentication
* is initiated.
*
* @param    InstancePtr is a pointer to the XHdcp_Repeater instance.
* @param    Protocol is defined by XV_HdmiTxSs1_HdcpProtocol type with the
*           following valid options:
*           - XV_HDMITXSS1_HDCP_NONE sets protocol to none
*             and disables downstream content blocking. This option
*             should be used for debug purposes only.
*           - XV_HDMITXSS1_HDCP_14 set protocol to 1.4
*           - XV_HDMITXSS1_HDCP_22 set protocol to 2.2
*           - XV_HDMITXSS1_HDCP_BOTH set protocol to both
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_Tx_Hdcp_SetCapability(XV_Tx *InstancePtr, int Protocol)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	if (InstancePtr->HdcpConfig.IsReady) {

		/* Set desired downstream capability */
		XV_HdmiTxSs1_HdcpSetCapability(InstancePtr->HdmiTxSs, Protocol);

		/* Push authentication request */
		XV_HdmiTxSs1_HdcpPushEvent(InstancePtr->HdmiTxSs,
				XV_HDMITXSS1_HDCP_AUTHENTICATE_EVT);
	}
}
#endif

u8 XV_Tx_IsConnected(XV_Tx *InstancePtr)
{
	if ((InstancePtr->StateInfo.CurrentState ==
	     XV_TX_HDMI_STATE_DISCONNECTED) ||
	    !(XV_HdmiTxSs1_IsStreamConnected(InstancePtr->HdmiTxSs))) {
		return FALSE;
	} else {
		return TRUE;
	}
}

void XV_Tx_SetAudioFormatAndChannels(XV_Tx *InstancePtr,
		XV_HdmiTx1_AudioFormatType AudioFormat, u8 AudioChannels)
{
	InstancePtr->ConfigInfo.AudioFormat = AudioFormat;
	xhdmi_example_tx_controller.ConfigInfo.AudioChannels = AudioChannels;
}

void XV_Tx_SetUseInternalACR (XV_Tx *InstancePtr)
{
	XV_HdmiTxSs1_SetIntACR(InstancePtr->HdmiTxSs);
}

void XV_Tx_SetUseExternalACR (XV_Tx *InstancePtr)
{
	XV_HdmiTxSs1_SetExtACR(InstancePtr->HdmiTxSs);
}

void XV_Tx_SetAudSamplingFreq (XV_Tx *InstancePtr,
		XHdmiC_SamplingFrequencyVal AudSampleFreqVal)
{
	XV_HdmiTxSs1_SetSampleFrequency(InstancePtr->HdmiTxSs,
			AudSampleFreqVal);
}

void XV_Tx_SetVic(XV_Tx *InstancePtr, u8 Vic)
{
	XV_HdmiTxSs1_SetVideoIDCode(InstancePtr->HdmiTxSs, Vic);
}

void XV_Tx_SetHdmiFrlMode(XV_Tx *InstancePtr)
{
	InstancePtr->ConfigInfo.IsFrl = TRUE;
	InstancePtr->ConfigInfo.IsHdmi = TRUE;
}

void XV_Tx_SetHdmiTmdsMode(XV_Tx *InstancePtr)
{
	InstancePtr->ConfigInfo.IsFrl = FALSE;
	InstancePtr->ConfigInfo.IsHdmi = TRUE;
}

void XV_Tx_SetDviMode(XV_Tx *InstancePtr)
{
	InstancePtr->ConfigInfo.IsFrl = FALSE;
	InstancePtr->ConfigInfo.IsHdmi = FALSE;
}

void XV_Tx_SetLineRate(XV_Tx *InstancePtr, u64 LineRate)
{
	InstancePtr->ConfigInfo.LineRate = LineRate;
}

u64 XV_Tx_GetLineRate(XV_Tx *InstancePtr)
{
	return InstancePtr->ConfigInfo.LineRate;
}

void XV_Tx_SetEdidParsingDone(XV_Tx *InstancePtr, u8 Set)
{
	InstancePtr->ConfigInfo.SinkEdidParsed = Set;
}

void XV_Tx_SetFrlEdidInfo(XV_Tx *InstancePtr, XV_VidC_Supp IsSCDCPresent,
			u8 MaxFrlRateSupp)
{
	InstancePtr->ConfigInfo.FrlEdidInfo.IsSCDCPresent = IsSCDCPresent;
	InstancePtr->ConfigInfo.FrlEdidInfo.MaxFrlRateSupp = MaxFrlRateSupp;
}

void XV_Tx_SetFrlIntVidCkeGen(XV_Tx *InstancePtr)
{
	XV_HdmiTxSs1_SetFrlIntVidCke(InstancePtr->HdmiTxSs);
}

void XV_Tx_SetFrlExtVidCkeGen(XV_Tx *InstancePtr)
{
	XV_HdmiTxSs1_SetFrlExtVidCke(InstancePtr->HdmiTxSs);
}

void XV_Tx_SetFRLCkeSrcToExternal(XV_Tx *InstancePtr, u32 Value)
{
	if (Value == TRUE) {
		InstancePtr->ConfigInfo.FrlCkeSrc = XV_TX_HDMI_FRL_CKE_SRC_EXTERNAL;
	} else if (Value == FALSE) {
		InstancePtr->ConfigInfo.FrlCkeSrc = XV_TX_HDMI_FRL_CKE_SRC_INTERNAL;
	} else {
		xil_printf(ANSI_COLOR_RED"Configuring the CKE to an external "
				"source can either be true or false, this option is not "
				"recognized !!"ANSI_COLOR_RESET"\r\n");
	}
}

u32 XV_Tx_StartFrlTraining (XV_Tx *InstancePtr,
			    XV_VidC_Supp IsSCDCPresent,
			    u8 MaxFrlRate)
{

	u32 Status = XST_FAILURE;

	if (MaxFrlRate > 0 && IsSCDCPresent == XVIDC_SUPPORTED) {
		/* Need to be called from TXS. */
		Status = XV_HdmiTxSs1_StartFrlTraining(InstancePtr->HdmiTxSs,
				MaxFrlRate);
	}

	if (Status == XST_FAILURE) {
		XV_HdmiTxSS1_SetHdmiTmdsMode(InstancePtr->HdmiTxSs);
	}

	return Status;
}

/*
 * This API is offered to the Exdes layer as an abstraction to
 * start the
 */
u32 XV_Tx_VideoSetupAndStart(XV_Tx *InstancePtr,
		XVidC_VideoStream *HdmiTxSsVidStreamPtr)
{
	u32 Status = XST_SUCCESS;

	u8 VPhyGTQuadId = 0;
	u64 TmdsClock = 0;
	u64 LnkClock;
	u64 VidClock;

#ifndef DEBUG_VCKE
	XHdmiphy1_PllType PllType;
	u64 TxFrlLineRate;
	u64 TxTmdsLineRate;
	XHdmiphy1_ChannelId ChId;
#endif

	XHdmiphy1_ChannelId Hdmiphy1ChannelId = XHDMIPHY1_CHANNEL_ID_CHA;
	XHdmiphy1 *VPhyInst = (XHdmiphy1 *)InstancePtr->VidPhy;

	/* Before starting the TX video check if the EDID parsing is
	 * complete. Proceed with starting the video only if the EDID
	 * parsing is complete, otherwise return.
	 */
	if (InstancePtr->ConfigInfo.SinkEdidParsed != TRUE) {
		xil_printf("SinkEdID parsed failed\r\n");
		return XST_FAILURE;
	}

	/* Reset the Hw Aux fifo. */
	/* XV_HdmiTxSs1_Reset(InstancePtr->HdmiTxSs); */

	xdbg_xv_tx_print("%s,%d. Tx Line Rate (set from app) = %d \r\n",
			__func__, __LINE__, InstancePtr->ConfigInfo.LineRate);

	/* XV_HdmiTxSS1_SetHdmiFrlMode(InstancePtr->HdmiTxSs); */

	if (InstancePtr->HdmiTxSs->HdmiTx1Ptr->Stream.IsFrl != TRUE) {
		xdbg_xv_tx_print("VidMode != XHDMIC_VIDMODE_HDMI_FRL!!!\r\n");
		if ((InstancePtr->ConfigInfo.LineRate / 1000000) > 3400) {
			/* TX reference clock */
			XV_HdmiTxSs1_SetTmdsClockRatio(
					InstancePtr->HdmiTxSs, 1);
			XV_HdmiTxSs1_SetVideoStreamScramblingFlag(
					InstancePtr->HdmiTxSs, TRUE);
		} else {
			XV_HdmiTxSs1_SetTmdsClockRatio(
					InstancePtr->HdmiTxSs, 0);
			XV_HdmiTxSs1_SetVideoStreamScramblingFlag(
					InstancePtr->HdmiTxSs, FALSE);
		}
	}

	xdbg_xv_tx_print("%s,%d : VmId = %d, ColorDepth = %d,"
			"ColorFormat = %d\r\n", __func__, __LINE__,
			HdmiTxSsVidStreamPtr->VmId,
			HdmiTxSsVidStreamPtr->ColorDepth,
			HdmiTxSsVidStreamPtr->ColorFormatId);

	TmdsClock = XV_HdmiTxSs1_SetStream(InstancePtr->HdmiTxSs,
			HdmiTxSsVidStreamPtr->Timing,
			HdmiTxSsVidStreamPtr->FrameRate,
			HdmiTxSsVidStreamPtr->ColorFormatId,
			HdmiTxSsVidStreamPtr->ColorDepth,
			NULL);

	xdbg_xv_tx_new_stream_setup_print("%s,%d. Tmds clock = %d \r\n"
			"\tNew Hdmi Tx Stream Params : PPC %d, "
			"ColorDepth %d, ColorFrmtId %d. \r\n",
			__func__, __LINE__, TmdsClock,
			HdmiTxSsVidStreamPtr->PixPerClk,
			HdmiTxSsVidStreamPtr->ColorDepth,
			HdmiTxSsVidStreamPtr->ColorFormatId);

	if (TmdsClock == 0) {
		if (InstancePtr->GetTxFrlClkCb != NULL) {
			InstancePtr->GetTxFrlClkCb(InstancePtr->GetTxFrlClkCbRef);
			if (VPhyInst->HdmiTxRefClkHz != 0) {
				TmdsClock = VPhyInst->HdmiTxRefClkHz;
			} else {
				return XST_FAILURE;
			}
		}
	}

	if (InstancePtr->HdmiTxSs->HdmiTx1Ptr->Stream.IsFrl != TRUE) {
		xdbg_xv_tx_new_stream_setup_print("Setting TX:Video On TMDS Mode \r\n");

		VPhyInst->HdmiTxRefClkHz = TmdsClock;
		Status = XHdmiphy1_SetHdmiTxParam(VPhyInst,
						  VPhyGTQuadId,
						  Hdmiphy1ChannelId,
						  HdmiTxSsVidStreamPtr->PixPerClk,
						  HdmiTxSsVidStreamPtr->ColorDepth,
						  HdmiTxSsVidStreamPtr->ColorFormatId);

		xdbg_xv_tx_print("%s,%d. After configuring VidPhy Hdmi Tx Params : PPC %d, "
			"ColorDepth %d, ColorFrmtId %d. \r\n",
			__func__, __LINE__,
			InstancePtr->HdmiTxSs->HdmiTx1Ptr->Stream.Video.PixPerClk,
			InstancePtr->HdmiTxSs->HdmiTx1Ptr->Stream.Video.ColorDepth,
			InstancePtr->HdmiTxSs->HdmiTx1Ptr->Stream.Video.ColorFormatId);

		XHdmiphy1_Clkout1OBufTdsEnable(InstancePtr->VidPhy, XHDMIPHY1_DIR_TX, FALSE);

		if (InstancePtr->SetupTxTmdsRefClkCb != NULL) {
			InstancePtr->SetupTxTmdsRefClkCb(InstancePtr->SetupTxTmdsRefClkCbRef);
		}
	} else {
		xdbg_xv_tx_new_stream_setup_print("Setting TX:Video On FRL Mode \r\n");
		/* FRL Stream Stop */
		XV_HdmiTxSs1_FrlStreamStop(InstancePtr->HdmiTxSs);

		/* Execute FRL register update */
		XV_HdmiTx1_FrlExecute(InstancePtr->HdmiTxSs->HdmiTx1Ptr);


#ifndef DEBUG_VCKE
		/* Determine PLL type. */
		PllType = XHdmiphy1_GetPllType(VPhyInst, 0, XHDMIPHY1_DIR_TX,
				XHDMIPHY1_CHANNEL_ID_CH1);

		/* Determine which channel(s) to operate on. */
		ChId = XHdmiphy1_GetRcfgChId(VPhyInst, 0,
					     XHDMIPHY1_DIR_TX, PllType);

		/* Correct TMDS Clock according to TMDS ratio */
		TmdsClock = (InstancePtr->HdmiTxSs->HdmiTx1Ptr->Stream.TMDSClockRatio ?
						TmdsClock / 4 : TmdsClock);


		/* Set TX reference clock */
		VPhyInst->HdmiTxRefClkHz = TmdsClock;

		if (InstancePtr->SetupTxRefClkCb != NULL) {
			InstancePtr->SetupTxRefClkCb(
					InstancePtr->SetupTxRefClkCbRef);
		}
		TmdsClock = VPhyInst->HdmiTxRefClkHz;

		usleep(1000000);

		/* Store FRL Line rate */
		TxFrlLineRate = XHdmiphy1_GetLineRateHz(VPhyInst, 0, ChId);

		/* Use TMDS Line Rate */
		TxTmdsLineRate = (u64) (InstancePtr->HdmiTxSs->HdmiTx1Ptr->Stream.TMDSClockRatio ?
					TmdsClock*40 : TmdsClock*10);
		XHdmiphy1_CfgLineRate(VPhyInst, 0, ChId, TxTmdsLineRate);

		XHdmiphy1_HdmiCfgCalcMmcmParam(VPhyInst, 0,
				XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_TX,
				InstancePtr->HdmiTxSs->HdmiTx1Ptr->Stream.Video.PixPerClk,
				InstancePtr->HdmiTxSs->HdmiTx1Ptr->Stream.Video.ColorDepth);

		XHdmiphy1_MmcmSetClkinsel(VPhyInst, 0,
					  XHDMIPHY1_DIR_TX,
					  MMCM_CLKINSEL_CLKIN1);
		XHdmiphy1_MmcmStart(VPhyInst, 0, XHDMIPHY1_DIR_TX);

		/* Restore FRL Line rate */
		XHdmiphy1_CfgLineRate(VPhyInst, 0, ChId, TxFrlLineRate);
#endif

#ifdef DEBUG_VCKE
		/* Calculate Link and Video Clocks */
		u32 PixelRate;

		PixelRate = (HdmiTxSsVidStreamPtr->Timing.HTotal *
		             HdmiTxSsVidStreamPtr->Timing.F0PVTotal *
		             HdmiTxSsVidStreamPtr->FrameRate) / 1000;

		if (HdmiTxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_YCRCB_420) {
			PixelRate = PixelRate / 2;
		}

		VidClock = PixelRate/(InstancePtr->HdmiTxSs->Config.Ppc);

		if (HdmiTxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_YCRCB_422) {
			LnkClock = VidClock;
		} else {
			LnkClock = (VidClock * (HdmiTxSsVidStreamPtr->ColorDepth)) / 8;
		}

#else
		VidClock = ((TxTmdsLineRate / 1000) / 10) / (InstancePtr->HdmiTxSs->Config.Ppc);
		LnkClock = (VidClock * (HdmiTxSsVidStreamPtr->ColorDepth)) / 8;
#endif

		xil_printf("XV_Tx_VideoSetupAndStart - TX CFG: "
			   "LCLK %d VCLK %d\r\n",
			   (u32)LnkClock, (u32)VidClock);

		/* Reset Bridge to clear the FIFO */
		XV_HdmiTxSs1_VRST(InstancePtr->HdmiTxSs, TRUE);
		XV_HdmiTxSs1_VRST(InstancePtr->HdmiTxSs, FALSE);

		/* Configure HDMI TX FRL Link and Video Clock registers */
		XV_HdmiTx1_SetFrlLinkClock(InstancePtr->HdmiTxSs->HdmiTx1Ptr,
					   (u32)LnkClock);

#ifdef DEBUG_VCKE
		if (InstancePtr->ConfigInfo.FrlCkeSrc ==
		    XV_TX_HDMI_FRL_CKE_SRC_INTERNAL) {
			xdbg_xv_tx_print("%s,%d, Colorbar / Independent TX "
					": Internal Cke\r\n",
					__func__, __LINE__);
			XV_Tx_SetFrlIntVidCkeGen(InstancePtr);
		} else if (InstancePtr->ConfigInfo.FrlCkeSrc ==
		           XV_TX_HDMI_FRL_CKE_SRC_EXTERNAL) {
			xdbg_xv_tx_print("%s,%d, Pass-Through : External Cke\r\n",
					__func__, __LINE__);
			XV_Tx_SetFrlExtVidCkeGen(InstancePtr);
		}

		XV_HdmiTx1_SetFrlVidClock(InstancePtr->HdmiTxSs->HdmiTx1Ptr,
				(u32)VidClock);
#else
		XV_HdmiTx1_SetFrlVidClock(InstancePtr->HdmiTxSs->HdmiTx1Ptr, 0);
#endif

		/* Execute FRL register update */
		XV_HdmiTx1_FrlExecute(InstancePtr->HdmiTxSs->HdmiTx1Ptr);

		/* FRL StreamStart */
		XV_HdmiTxSs1_FrlStreamStart(InstancePtr->HdmiTxSs);
	}

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	} else {
		return XST_SUCCESS;
	}
}

u32 XV_Tx_GetNVal(XV_Tx *InstancePtr,
		XHdmiC_SamplingFrequencyVal SampFreqVal)
{
	XHdmiC_SamplingFrequencyVal SampFreq;
	XHdmiC_FRLCharRate FRLCharRate;
	u32 NVal;

	if (InstancePtr->HdmiTxSs->HdmiTx1Ptr->Stream.IsFrl == TRUE) {

		switch (InstancePtr->HdmiTxSs->HdmiTx1Ptr->Stream.Frl.LineRate) {
			case 3:
				FRLCharRate = R_166_667;
				break;
			case 6:
				FRLCharRate = R_333_333;
				break;
			case 8:
				FRLCharRate = R_444_444;
				break;
			case 10:
				FRLCharRate = R_555_556;
				break;
			case 12:
				FRLCharRate = R_666_667;
				break;
			default:
				FRLCharRate = R_166_667;
				break;
		}
		NVal = XHdmiC_FRL_GetNVal(FRLCharRate, SampFreqVal);
	} else {
		SampFreq = XHdmiC_GetAudIFSampFreq(SampFreqVal);
		NVal =
			XHdmiC_TMDS_GetNVal
			(XV_HdmiTxSs1_GetTmdsClockFreqHz(InstancePtr->HdmiTxSs),
					SampFreq);
	}
	return NVal;

}

u32 XV_Tx_SetTriggerCallbacks(XV_Tx *InstancePtr,
		XV_Tx_Trigger_CallbackHandler Handler,
		void *Callback, void *CallbackRef)
{
	int Status = XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr);
	Xil_AssertNonvoid(Callback);
	Xil_AssertNonvoid(CallbackRef);

	switch (Handler) {
	case XV_TX_TRIG_HANDLER_CONNECTION_CHANGE:
		InstancePtr->TxCableConnectionChange = Callback;
		InstancePtr->TxCableConnectionChangeCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XV_TX_TRIG_HANDLER_SETUP_TXTMDSREFCLK:
		InstancePtr->SetupTxTmdsRefClkCb = Callback;
		InstancePtr->SetupTxTmdsRefClkCbRef = CallbackRef;
		break;

	case XV_TX_TRIG_HANDLER_SETUP_TXFRLREFCLK:
		InstancePtr->SetupTxFrlRefClkCb = Callback;
		InstancePtr->SetupTxFrlRefClkCbRef = CallbackRef;
		break;

	case XV_TX_GET_FRL_CLOCK:
		InstancePtr->GetTxFrlClkCb = Callback;
		InstancePtr->GetTxFrlClkCbRef = CallbackRef;
		break;

	case XV_TX_TRIG_HANDLER_SETUP_AUDVID:
		InstancePtr->SetupAudVidSrc = Callback;
		InstancePtr->SetupAudVidSrcCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XV_TX_TRIG_HANDLER_STREAM_ON:
		InstancePtr->TxStreamOn = Callback;
		InstancePtr->TxStreamOnCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;
	case XV_TX_TRIG_HANDLER_ENABLE_CABLE_DRIVERS:
		InstancePtr->EnableCableDriversCb = Callback;
		InstancePtr->EnableCableDriversCbRef = CallbackRef;
		break;

	case XV_TX_TRIG_HANDLER_VSYNC_RECV:
		InstancePtr->TxVidSyncRecv = Callback;
		InstancePtr->TxVidSyncRecvCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XV_TX_TRIG_HANDLER_STREAM_OFF:
		InstancePtr->TxStreamOff = Callback;
		InstancePtr->TxStreamOffCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XV_TX_TRIG_HANDLER_READYTOSTARTTX:
		InstancePtr->TxReadyToStart = Callback;
		InstancePtr->TxReadyToStartCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XV_TX_TRIG_HANDLER_FRL_FFE_CONFIG_DEVICE:
		InstancePtr->TxFrlFfeConfigDeviceCb = Callback;
		InstancePtr->TxFrlFfeConfigDeviceCbRef = CallbackRef;
		break;

	case XV_TX_TRIG_HANDLER_FRL_CONFIG_DEVICE_SETUP:
		InstancePtr->TxFrlConfigDeviceSetupCb = Callback;
		InstancePtr->TxFrlConfigDeviceSetupCbRef = CallbackRef;
		break;

#ifdef USE_HDCP_HDMI_TX
	case XV_TX_TRIG_HANDLER_HDCP_FORCE_BLANKING:
		InstancePtr->HdcpForceBlankCb = Callback;
		InstancePtr->HdcpForceBlankCbRef = CallbackRef;
		Status = XST_SUCCESS;
		break;
#endif

	default:
		Status = XST_FAILURE;
		break;
	}

	return Status;
}


/*****************************************************************************/
/**
* This function initializes the callbacks for the HDMI TX sub-system events.
*
* @param    InstancePtr is the pointer reference to the Xilinx Video
*           Receiver instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
u32 XV_Tx_HdmiTxSs_Setup_Callbacks(XV_Tx *InstancePtr)
{
	u32 Status = XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr);
	Status |= XV_HdmiTxSs1_SetCallback(InstancePtr->HdmiTxSs,
			XV_HDMITXSS1_HANDLER_CONNECT,
			(void *)XV_Tx_HdmiTx_Connect_Cb,
			(void *)InstancePtr);

	Status |= XV_HdmiTxSs1_SetCallback(InstancePtr->HdmiTxSs,
			XV_HDMITXSS1_HANDLER_TOGGLE,
			(void *)XV_Tx_HdmiTx_Toggle_Cb,
			(void *)InstancePtr);

	Status |= XV_HdmiTxSs1_SetCallback(InstancePtr->HdmiTxSs,
			XV_HDMITXSS1_HANDLER_BRDGUNLOCK,
			(void *)XV_Tx_HdmiTx_BrdgUnlocked_Cb,
			(void *)InstancePtr);

	Status |= XV_HdmiTxSs1_SetCallback(InstancePtr->HdmiTxSs,
			XV_HDMITXSS1_HANDLER_BRDGUNDERFLOW,
			(void *)XV_Tx_HdmiTx_BrdgUnderFlow_Cb,
			(void *)InstancePtr);

	Status |= XV_HdmiTxSs1_SetCallback(InstancePtr->HdmiTxSs,
			XV_HDMITXSS1_HANDLER_BRDGOVERFLOW,
			(void *)XV_Tx_HdmiTx_BrdgOverFlow_Cb,
			(void *)InstancePtr);

	Status |= XV_HdmiTxSs1_SetCallback(InstancePtr->HdmiTxSs,
			XV_HDMITXSS1_HANDLER_VS,
			(void *)XV_Tx_HdmiTx_VSync_Cb,
			(void *)InstancePtr);

	Status |= XV_HdmiTxSs1_SetCallback(InstancePtr->HdmiTxSs,
			XV_HDMITXSS1_HANDLER_STREAM_DOWN,
			(void *)XV_Tx_HdmiTx_StreamDown_Cb,
			(void *)InstancePtr);

	Status |= XV_HdmiTxSs1_SetCallback(InstancePtr->HdmiTxSs,
			XV_HDMITXSS1_HANDLER_STREAM_UP,
			(void *)XV_Tx_HdmiTx_StreamUp_Cb,
			(void *)InstancePtr);

	Status |= XV_HdmiTxSs1_SetCallback(InstancePtr->HdmiTxSs,
			XV_HDMITXSS1_HANDLER_FRL_CONFIG,
			(void *)XV_Tx_HdmiTx_FrlConfig_Cb,
			(void *)InstancePtr);

	Status |= XV_HdmiTxSs1_SetCallback(InstancePtr->HdmiTxSs,
			XV_HDMITXSS1_HANDLER_FRL_FFE,
			(void *)XV_Tx_HdmiTx_FrlFfe_Cb,
			(void *)InstancePtr);

	Status |= XV_HdmiTxSs1_SetCallback(InstancePtr->HdmiTxSs,
			XV_HDMITXSS1_HANDLER_FRL_START,
			(void *)XV_Tx_HdmiTx_FrlStart_Cb,
			(void *)InstancePtr);

	Status |= XV_HdmiTxSs1_SetCallback(InstancePtr->HdmiTxSs,
			XV_HDMITXSS1_HANDLER_FRL_STOP,
			(void *)XV_Tx_HdmiTx_FrlStop_Cb,
			(void *)InstancePtr);

	Status |= XV_HdmiTxSs1_SetCallback(InstancePtr->HdmiTxSs,
			XV_HDMITXSS1_HANDLER_TMDS_CONFIG,
			(void *)XV_Tx_HdmiTx_TmdsConfig_Cb,
			(void *)InstancePtr);

	return Status;
}

#ifdef USE_HDCP_HDMI_TX
u32 XV_Tx_Hdcp_SetCallbacks(XV_Tx *InstancePtr)
{
	u32 Status = XST_SUCCESS;

	Status |= XV_HdmiTxSs1_SetCallback(InstancePtr->HdmiTxSs,
			XV_HDMITXSS1_HANDLER_HDCP_AUTHENTICATED,
			(void *)XV_Tx_Hdcp_Authenticated_Cb,
			(void *)InstancePtr);

	Status |= XV_HdmiTxSs1_SetCallback(InstancePtr->HdmiTxSs,
			XV_HDMITXSS1_HANDLER_HDCP_UNAUTHENTICATED,
			(void *)XV_Tx_Hdcp_Unauthenticated_Cb,
			(void *)InstancePtr);

	return Status;
}
#endif

/*****************************************************************************/
/**
* This function initializes the HdmiTxSs, and corresponding Hdmiphy1 instance
* and sets the corresponding interrupts and their callbacks.
*
* @param    InstancePtr is the callback reference to the HDMI TX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
u32 XV_Tx_Hdmi_Initialize(XV_Tx *InstancePtr, u32 HdmiTxSsDevId,
		u32 VPhyDevId, XV_Tx_IntrVecId IntrVecIds)
{
	XV_HdmiTxSs1_Config *XV_HdmiTxSs1_ConfigPtr;
	u32 Status = XST_SUCCESS;

	/**
	 * Initialize the HDMI TX SS
	 */
	XV_HdmiTxSs1_ConfigPtr = XV_HdmiTxSs1_LookupConfig(HdmiTxSsDevId);

	if (XV_HdmiTxSs1_ConfigPtr == NULL) {
		InstancePtr->HdmiTxSs->IsReady = 0;
	}

	/* Initialize top level and all included sub-cores */
	Status = XV_HdmiTxSs1_CfgInitialize(InstancePtr->HdmiTxSs,
					XV_HdmiTxSs1_ConfigPtr,
					XV_HdmiTxSs1_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: HDMI TX Subsystem Initialization Failed %d\r\n",
		        Status);
	}

	/* Register HDMI TX SS Interrupt Handler with Interrupt Controller */
#if defined(__arm__) || (__aarch64__)
	Status |= XScuGic_Connect(InstancePtr->Intc,
			IntrVecIds.IntrVecId_HdmiTxSs,
			(XInterruptHandler)XV_HdmiTxSS1_HdmiTx1IntrHandler,
			(void *)InstancePtr->HdmiTxSs);

/* HDCP 1.4 */
#ifdef XPAR_XHDCP_NUM_INSTANCES
	/* HDCP 1.4 Cipher interrupt */
	Status |= XScuGic_Connect(InstancePtr->Intc,
			IntrVecIds.IntrVecId_Hdcp14,
			(XInterruptHandler)XV_HdmiTxSS1_HdcpIntrHandler,
			(void *)InstancePtr->HdmiTxSs);

	/* HDCP 1.4 Timer interrupt */
	Status |= XScuGic_Connect(InstancePtr->Intc,
			IntrVecIds.IntrVecId_Hdcp14Timer,
			(XInterruptHandler)XV_HdmiTxSS1_HdcpTimerIntrHandler,
			(void *)InstancePtr->HdmiTxSs);
#endif

/* HDCP 2.2 */
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
	Status |= XScuGic_Connect(InstancePtr->Intc,
			IntrVecIds.IntrVecId_Hdcp22Timer,
			(XInterruptHandler)XV_HdmiTxSS1_Hdcp22TimerIntrHandler,
			(void *)InstancePtr->HdmiTxSs);
#endif

#else
	/* Register HDMI TX SS Interrupt Handler with Interrupt Controller */
	Status |= XIntc_Connect(InstancePtr->Intc,
			IntrVecIds.IntrVecId_HdmiTxSs,
			(XInterruptHandler)XV_HdmiTxSS1_HdmiTx1IntrHandler,
			(void *)InstancePtr->HdmiTxSs);

/* HDCP 1.4 */
#ifdef XPAR_XHDCP_NUM_INSTANCES
	/* HDCP 1.4 Cipher interrupt */
	Status |= XIntc_Connect(InstancePtr->Intc,
			IntrVecIds.IntrVecId_Hdcp14,
			(XInterruptHandler)XV_HdmiTxSS1_HdcpIntrHandler,
			(void *)InstancePtr->HdmiTxSs);

	/* HDCP 1.4 Timer interrupt */
	Status |= XIntc_Connect(InstancePtr->Intc,
			IntrVecIds.IntrVecId_Hdcp14Timer,
			(XInterruptHandler)XV_HdmiTxSS1_HdcpTimerIntrHandler,
			(void *)InstancePtr->HdmiTxSs);
#endif

/* HDCP 2.2 */
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
	Status |= XIntc_Connect(InstancePtr->Intc,
			IntrVecIds.IntrVecId_Hdcp22Timer,
			(XInterruptHandler)XV_HdmiTxSS1_Hdcp22TimerIntrHandler,
			(void *)InstancePtr->HdmiTxSs);
#endif

#endif

	if (Status == XST_SUCCESS) {
#if defined(__arm__) || (__aarch64__)
		XScuGic_Enable(InstancePtr->Intc,
			IntrVecIds.IntrVecId_HdmiTxSs);

		/* HDCP 1.4 */
#ifdef XPAR_XHDCP_NUM_INSTANCES
		/* HDCP 1.4 Cipher interrupt */
		XScuGic_Enable(InstancePtr->Intc,
			IntrVecIds.IntrVecId_Hdcp14);

		/* HDCP 1.4 Timer interrupt */
		XScuGic_Enable(InstancePtr->Intc,
			IntrVecIds.IntrVecId_Hdcp14Timer);
#endif

/* HDCP 2.2 */
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
		XScuGic_Enable(InstancePtr->Intc,
			IntrVecIds.IntrVecId_Hdcp22Timer);
#endif

#else
		XIntc_Enable(InstancePtr->Intc,
			IntrVecIds.IntrVecId_HdmiTxSs);

/* HDCP 1.4 */
#ifdef XPAR_XHDCP_NUM_INSTANCES
		/* HDCP 1.4 Cipher interrupt */
		XIntc_Enable(InstancePtr->Intc,
			IntrVecIds.IntrVecId_Hdcp14);

		/* HDCP 1.4 Timer interrupt */
		XIntc_Enable(InstancePtr->Intc,
			IntrVecIds.IntrVecId_Hdcp14Timer);
#endif

/* HDCP 2.2 */
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
		XIntc_Enable(InstancePtr->Intc,
			IntrVecIds.IntrVecId_Hdcp22Timer);
#endif

#endif
	} else {
		xil_printf("ERR:: Unable to register HDMI "
			   "TX interrupt handler for ");
		xil_printf("HDMI TX SS initialization error\r\n");
		return XST_FAILURE;
	}

	/* Initialize the HDMI TX SS interrupts. */
	XV_Tx_HdmiTxSs_Setup_Callbacks(InstancePtr);

	/**
	 * Initialize the VPhy related to the HdmiTxSs if it isn't ready yet.
	 */
	if (!InstancePtr->VidPhy->IsReady) {
		xil_printf("Initializing Video Phy with Video Transmitter \r\n");
		XHdmiphy1_Config *XHdmiphy1CfgPtr;

		/* Initialize Video PHY */
		XHdmiphy1CfgPtr = XHdmiphy1_LookupConfig(VPhyDevId);
		if (XHdmiphy1CfgPtr == NULL) {
			xil_printf("Video PHY device not found\r\n\r\n");
			return XST_FAILURE;
		}

		/* Register VPHY Interrupt Handler */
#if defined(__arm__) || (__aarch64__)
		Status = XScuGic_Connect(InstancePtr->Intc,
#else
		Status = XIntc_Connect(InstancePtr->Intc,
#endif
					IntrVecIds.IntrVecId_VPhy,
					(XInterruptHandler)XHdmiphy1_InterruptHandler,
					(void *)InstancePtr->VidPhy);

		if (Status != XST_SUCCESS) {
			xil_printf("HDMI VPHY Interrupt Vec ID not found!\r\n");
			return XST_FAILURE;
		}

		/* Initialize HDMI VPHY */
		Status = XHdmiphy1_Hdmi_CfgInitialize(InstancePtr->VidPhy, 0,
						      XHdmiphy1CfgPtr);

		if (Status != XST_SUCCESS) {
			xil_printf("HDMI VPHY initialization error\r\n");
			return XST_FAILURE;
		}

		/* Enable VPHY Interrupt */
#if defined(__arm__) || (__aarch64__)
		XScuGic_Enable(InstancePtr->Intc,
				IntrVecIds.IntrVecId_VPhy);
#else
		XIntc_Enable(InstancePtr->Intc,
				IntrVecIds.IntrVecId_VPhy);
#endif
	} else {
		xil_printf("Not Initializing Video Phy with Video "
				"Transmitter as it is already ready.\r\n");
	}

	/* Initialize the VPhy interrupts for HDMI TX. */
	XV_Tx_VPhy_SetCallbacks(InstancePtr);

#ifdef USE_HDCP_HDMI_TX
	/**
	 * Initialize HDCP
	 */
	xil_printf("Initializing HDCP for the Video Transmitter \r\n");
	/* Set the HDCP configurations to 0. */
	InstancePtr->HdcpConfig.DownstreamInstanceBinded = 0;
	InstancePtr->HdcpConfig.DownstreamInstanceConnected = FALSE;
	InstancePtr->HdcpConfig.DownstreamInstanceStreamUp = FALSE;
	InstancePtr->HdcpConfig.IsRepeater = FALSE;

	/* Initialize the HDCP related callbacks. */
	Status = XV_Tx_Hdcp_SetCallbacks(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: Unable to set HDCP callbacks !!\r\n");
		return XST_FAILURE;
	}

	/* Increment the number of associated downstream instances. */
	InstancePtr->HdcpConfig.DownstreamInstanceBinded++;
	InstancePtr->HdcpConfig.IsReady = TRUE;

	/* Disable Repeater. */
	/* XV_HdmiTxSs_HdcpSetRepeater(InstancePtr->HdmiTxSs, FALSE); */
#endif
	/**
	 * Set the default HDMI Video Transmitter configurations.
	 */
	InstancePtr->ConfigInfo.AudioFormat = XV_HDMITX1_AUDFMT_LPCM;
	InstancePtr->ConfigInfo.AudioChannels = 2;
	InstancePtr->ConfigInfo.SinkEdidParsed = FALSE;
	InstancePtr->ConfigInfo.Vsync0_after_steamOn = FALSE;

	InstancePtr->ErrorStats.TxBrdgOverflowCnt = 0;
	InstancePtr->ErrorStats.TxBrdgUnderflowCnt = 0;

	InstancePtr->XV_Tx_StreamState = XV_TX_HDMITXSS_STREAM_DOWN;

	InstancePtr->StateInfo.CurrentState = XV_TX_HDMI_STATE_DISCONNECTED;
	InstancePtr->StateInfo.PreviousState = XV_TX_HDMI_STATE_DISCONNECTED;
	InstancePtr->StateInfo.HdmiTxPendingEvents = 0x0;

	return Status;
}

#ifdef USE_HDCP_HDMI_TX
/*****************************************************************************/
/**
* This function implements the callback for HDCP authentication event.
*
* @param    InstancePtr is the callback reference to the HDMI TX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_Hdcp_Authenticated_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	int HdcpProtocol;
	XV_Tx *txInst = (XV_Tx *)CallbackRef;
	int Status;

	xdbg_xv_tx_print(ANSI_COLOR_HIGH_YELLOW" "ANSI_COLOR_BG_BLUE"%s, %d "
			"Hdcp-Authenticated"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__);

	if (XV_HdmiTxSs1_HdcpIsAuthenticated(txInst->HdmiTxSs)) {
		/* Check the downstream interface protocol */
		HdcpProtocol = XV_HdmiTxSs1_HdcpGetProtocol(txInst->HdmiTxSs);

		switch (HdcpProtocol) {
	        /* HDCP 2.2 */
	        case XV_HDMITXSS1_HDCP_22:
			xdbg_printf(XDBG_DEBUG_GENERAL, "HDCP 2.2 downstream "
	        		"authenticated\r\n");
			Status = XV_HdmiTxSs1_HdcpEnableEncryption(txInst->HdmiTxSs);
			if (Status == XST_FAILURE) {
				xil_printf("Packet Encryption Fail "
					   "for HDCP 2.2\r\n");
			}
			break;

	        /* HDCP 1.4 */
	        case XV_HDMITXSS1_HDCP_14:
			xdbg_printf(XDBG_DEBUG_GENERAL, "HDCP 1.4 downstream "
	        		  "authenticated\r\n");
			Status = XV_HdmiTxSs1_HdcpEnableEncryption(txInst->HdmiTxSs);
			if (Status == XST_FAILURE) {
				xil_printf("Packet Encryption Fail "
					   "for HDCP 1.4\r\n");
			}
			break;
	      }
	}

	if (txInst->HdcpForceBlankCb != NULL) {
		txInst->HdcpForceBlankCb(txInst->HdcpForceBlankCbRef);
	}
}

/*****************************************************************************/
/**
* This function implements the callback for HDCP Un-authentication event.
*
* @param    InstancePtr is the callback reference to the HDMI TX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_Hdcp_Unauthenticated_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Tx *txInst = (XV_Tx *)CallbackRef;

	xdbg_xv_tx_print(ANSI_COLOR_HIGH_YELLOW" "ANSI_COLOR_BG_BLUE"%s, %d "
			"Hdcp-Un-authenticated"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__);

	if (txInst->HdcpForceBlankCb != NULL) {
		txInst->HdcpForceBlankCb(txInst->HdcpForceBlankCbRef);
	}
}
#endif

/*****************************************************************************/
/**
* This function implements the callback for the HDMI TX Connect event.
*
* @param    InstancePtr is the callback reference to the HDMI TX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_HdmiTx_Connect_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Tx *txInst = (XV_Tx *)CallbackRef;

	if(txInst->HdmiTxSs->IsStreamConnected == (FALSE)) {
		xdbg_xv_tx_print(ANSI_COLOR_YELLOW" "ANSI_COLOR_BG_HIGH_CYAN
				"%s, %d Disconnected"ANSI_COLOR_RESET"\r\n",
				__func__, __LINE__);

		/* Alternatively, if a priority queue is established, use :-
		 * XV_Tx_HdmiTx_PushEvent(txInst, XV_TX_HDMI_EVENT_DISCONNECTED);
		 */
		XV_Tx_HdmiTx_SendEvent(txInst, XV_TX_HDMI_EVENT_DISCONNECTED);

	} else {
		xdbg_xv_tx_print(ANSI_COLOR_YELLOW" "ANSI_COLOR_BG_HIGH_CYAN
				"%s, %d Connected"ANSI_COLOR_RESET"\r\n",
				__func__, __LINE__);

		/* Alternatively, if a priority queue is established, use :-
		 * XV_Tx_HdmiTx_PushEvent(txInst, XV_TX_HDMI_EVENT_CONNECTED);
		 */
		XV_Tx_HdmiTx_SendEvent(txInst, XV_TX_HDMI_EVENT_CONNECTED);
	}

}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI TX Toggle event.
*
* @param    CallbackRef is the callback reference to the
*           Xilinx Video Transmitter.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_HdmiTx_Toggle_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	xdbg_xv_tx_print(ANSI_COLOR_YELLOW" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
				ANSI_COLOR_RESET"\r\n", __func__, __LINE__);

	XV_Tx *txInst = (XV_Tx *)CallbackRef;

	XV_HdmiTxSs1_StreamStart(txInst->HdmiTxSs);

#ifdef USE_HDCP_HDMI_TX
	/* Call HDCP connect callback */
	XV_Tx_Hdcp_Authenticate(txInst);
#endif
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI TX Bridge underflow
* event.
*
* @param    CallbackRef is the callback reference to the Xilinx Video
*           Transmitter.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_HdmiTx_BrdgUnlocked_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	xdbg_xv_tx_print(ANSI_COLOR_YELLOW" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
			ANSI_COLOR_RESET"\r\n", __func__, __LINE__);

	XV_Tx *txInst = (XV_Tx *)CallbackRef;

	/* Alternatively, if a priority queue is established, use :-
	 * XV_Tx_HdmiTx_PushEvent(txInst, XV_TX_HDMI_EVENT_BRDGUNLOCKED);
	 */
	XV_Tx_HdmiTx_SendEvent(txInst, XV_TX_HDMI_EVENT_BRDGUNLOCKED);
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI TX Bridge underflow
* event.
*
* @param    CallbackRef is the callback reference to the Xilinx Video
*           Transmitter.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_HdmiTx_BrdgUnderFlow_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Tx *txInst = (XV_Tx *)CallbackRef;

	txInst->ErrorStats.TxBrdgUnderflowCnt++;
	/* Update any statistics here if necessary or push to the state
	 * machine.
	 * Currently, not doing anything on receiving the Bridge Underflow
	 * interrupt.
	 */
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI TX Bridge overflow
* event.
*
* @param    CallbackRef is the callback reference to the Xilinx Video
*           Transmitter.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_HdmiTx_BrdgOverFlow_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	xdbg_xv_tx_print(ANSI_COLOR_YELLOW" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
			ANSI_COLOR_RESET"\r\n", __func__, __LINE__);

	XV_Tx *txInst = (XV_Tx *)CallbackRef;

	txInst->ErrorStats.TxBrdgOverflowCnt++;
	/* Update any statistics here if necessary or push to the state
	 * machine.
	 * Currently, not doing anything on receiving the Bridge Overflow
	 * interrupt.
	 */
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI TX Video Sync event.
*
* @param    CallbackRef is the callback reference to the Xilinx Video
*           Transmitter.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_HdmiTx_VSync_Cb(void *CallbackRef)
{
	XV_Tx *txInst = (XV_Tx *)CallbackRef;

	/* xdbg_xv_tx_print(ANSI_COLOR_YELLOW" "ANSI_COLOR_BG_HIGH_CYAN
	 *		    "%s, %d "ANSI_COLOR_RESET"\r\n",
	 *		    __func__, __LINE__);
	 */

	/* Alternatively, if a priority queue is established, use :-
	 * XV_Tx_HdmiTx_PushEvent(txInst, XV_TX_HDMI_EVENT_VSYNC);
	 */
	XV_Tx_HdmiTx_SendEvent(txInst, XV_TX_HDMI_EVENT_VSYNC);

}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI TX Stream Down event.
*
* @param    InstancePtr is the callback reference to the HDMI TX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_HdmiTx_StreamDown_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	xdbg_xv_tx_print(ANSI_COLOR_YELLOW" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
			ANSI_COLOR_RESET"\r\n", __func__, __LINE__);

	XV_Tx *txInst = (XV_Tx *)CallbackRef;

	txInst->ErrorStats.TxBrdgOverflowCnt  = 0;
	txInst->ErrorStats.TxBrdgUnderflowCnt = 0;

	/* Alternatively, if a priority queue is established, use :-
	 * XV_Tx_HdmiTx_PushEvent(txInst, XV_TX_HDMI_EVENT_STREAMDOWN);
	 */
	XV_Tx_HdmiTx_SendEvent(txInst, XV_TX_HDMI_EVENT_STREAMDOWN);
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI TX Stream Up event.
*
* @param    InstancePtr is the callback reference to the HDMI TX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_HdmiTx_StreamUp_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	xdbg_xv_tx_print(ANSI_COLOR_YELLOW" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
			ANSI_COLOR_RESET"\r\n", __func__, __LINE__);

	XV_Tx *txInst = (XV_Tx *)CallbackRef;

	/* Alternatively, if a priority queue is established, use :-
	 * XV_Tx_HdmiTx_PushEvent(txInst, XV_TX_HDMI_EVENT_STREAMUP);
	 */
	XV_Tx_HdmiTx_SendEvent(txInst, XV_TX_HDMI_EVENT_STREAMUP);
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI TX FRL configuration
* request event.
*
* @param    InstancePtr is the callback reference to the HDMI TX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_HdmiTx_FrlConfig_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	xdbg_xv_tx_print(ANSI_COLOR_YELLOW" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
			ANSI_COLOR_RESET"\r\n", __func__, __LINE__);

	XV_Tx *txInst = (XV_Tx *)CallbackRef;

	/* Alternatively, if a priority queue is established, use :-
	 * XV_Tx_HdmiTx_PushEvent(txInst, XV_TX_HDMI_EVENT_FRLCONFIG);
	 */
	XV_Tx_HdmiTx_SendEvent(txInst, XV_TX_HDMI_EVENT_FRLCONFIG);
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI TX FRL FFE event.
*
* @param    InstancePtr is the callback reference to the HDMI TX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_HdmiTx_FrlFfe_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	xdbg_xv_tx_print(ANSI_COLOR_YELLOW" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
			ANSI_COLOR_RESET"\r\n", __func__, __LINE__);

	XV_Tx *txInst = (XV_Tx *)CallbackRef;

	/* Alternatively, if a priority queue is established, use :-
	 * XV_Tx_HdmiTx_PushEvent(txInst, XV_TX_HDMI_EVENT_FRLFFE);
	 */
	XV_Tx_HdmiTx_SendEvent(txInst, XV_TX_HDMI_EVENT_FRLFFE);
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI TX FRL Start event.
*
* @param    InstancePtr is the callback reference to the HDMI TX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_HdmiTx_FrlStart_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	xdbg_xv_tx_print(ANSI_COLOR_YELLOW" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
			ANSI_COLOR_RESET"\r\n", __func__, __LINE__);

	XV_Tx *txInst = (XV_Tx *)CallbackRef;

	/* Alternatively, if a priority queue is established, use :-
	 * XV_Tx_HdmiTx_PushEvent(txInst, XV_TX_HDMI_EVENT_FRLSTART);
	 */
	XV_Tx_HdmiTx_SendEvent(txInst, XV_TX_HDMI_EVENT_FRLSTART);
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI TX FRL Stop event.
*
* @param    InstancePtr is the callback reference to the HDMI TX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_HdmiTx_FrlStop_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	xdbg_xv_tx_print(ANSI_COLOR_YELLOW" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
			ANSI_COLOR_RESET"\r\n", __func__, __LINE__);

	XV_Tx *txInst = (XV_Tx *)CallbackRef;

	/* Alternatively, if a priority queue is established, use :-
	 * XV_Tx_HdmiTx_PushEvent(txInst, XV_TX_HDMI_EVENT_FRLSTOP);
	 */
	XV_Tx_HdmiTx_SendEvent(txInst, XV_TX_HDMI_EVENT_FRLSTOP);
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI TX TMDS Configuration
* request event.
*
* @param    InstancePtr is the callback reference to the HDMI TX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_HdmiTx_TmdsConfig_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	xdbg_xv_tx_print(ANSI_COLOR_YELLOW" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
			ANSI_COLOR_RESET"\r\n", __func__, __LINE__);

	XV_Tx *txInst = (XV_Tx *)CallbackRef;

	/* Alternatively, if a priority queue is established, use :-
	 * XV_Tx_HdmiTx_PushEvent(txInst, XV_TX_HDMI_EVENT_TMDSCONFIG);
	 */
	XV_Tx_HdmiTx_SendEvent(txInst, XV_TX_HDMI_EVENT_TMDSCONFIG);
}

/******************* XV TX STATE MACHINE IMPLEMENTATION **********************/

/*****************************************************************************/
/**
* This function
*
* @param    InstancePtr is the callback reference to the Xilinx Video Receiver
*           instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_Tx_HdmiTx_ProcessPendingEvents(XV_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	XV_Tx_Hdmi_Events EventPending;

	for (int i = 0 ; i < XV_TX_HDMI_NUM_EVENTS ; i++) {
		if ((InstancePtr->StateInfo.HdmiTxPendingEvents) &
		    (u16)(0x1 << i)) {
			EventPending = XHdmi_Tx_EventsPriorityQueue[i];

			/* Process the pending event. */
			XV_Tx_HdmiTx_ProcessEvents(InstancePtr, EventPending);

			/* Clear the pending event. */
			InstancePtr->StateInfo.HdmiTxPendingEvents &=
					(u16)(~(0x1 << i));

		}
	}
}

/*****************************************************************************/
/**
* This function
*
* @param    InstancePtr is the callback reference to the Xilinx Video Receiver
*           instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_Tx_HdmiTx_PushEvent(XV_Tx *InstancePtr,
					XV_Tx_Hdmi_Events Event)
{
	Xil_AssertVoid(InstancePtr);
	Xil_AssertVoid(Event < XV_TX_HDMI_NUM_EVENTS);

	for (int i = 0 ; i < XV_TX_HDMI_NUM_EVENTS ; i++) {
		if (Event == XHdmi_Tx_EventsPriorityQueue[i]) {
			InstancePtr->StateInfo.HdmiTxPendingEvents |= (u16)(0x1 << i);
		}
	}
}

/*****************************************************************************/
/**
* This function
*
* @param    InstancePtr is the callback reference to the Xilinx Video Receiver
*           instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Tx_HdmiTx_SendEvent(XV_Tx *InstancePtr,
					XV_Tx_Hdmi_Events Event)
{
	Xil_AssertVoid(InstancePtr);
	Xil_AssertVoid(Event < XV_TX_HDMI_NUM_EVENTS);

	XV_Tx_HdmiTx_ProcessEvents(InstancePtr, Event);
}

/*****************************************************************************/
/**
* This function
*
* @param    InstancePtr is the callback reference to the Xilinx Video Receiver
*           instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_Tx_HdmiTx_ProcessEvents(XV_Tx *InstancePtr,
					XV_Tx_Hdmi_Events Event)
{
	Xil_AssertVoid(InstancePtr);
	Xil_AssertVoid(Event < XV_TX_HDMI_NUM_EVENTS);

	XV_Tx_Hdmi_State NextState =
			InstancePtr->StateInfo.CurrentState;

	switch (InstancePtr->StateInfo.CurrentState) {
	case XV_TX_HDMI_STATE_DISCONNECTED:
		XV_Tx_HdmiTx_StateDisconnected(InstancePtr, Event, &NextState);
		break;

	case XV_TX_HDMI_STATE_CONNECTED:
		XV_Tx_HdmiTx_StateConnected(InstancePtr, Event, &NextState);
		break;

	case XV_TX_HDMI_STATE_BRDG_UNLOCKED:
		XV_Tx_HdmiTx_StateBrdgUnlocked(InstancePtr, Event, &NextState);
		break;

	case XV_TX_HDMI_STATE_VSYNC:
		XV_Tx_HdmiTx_StateVSync(InstancePtr, Event, &NextState);
		break;

	case XV_TX_HDMI_STATE_STREAMON:
		XV_Tx_HdmiTx_StateStreamOn(InstancePtr, Event, &NextState);
		break;

	case XV_TX_HDMI_STATE_STREAMOFF:
		XV_Tx_HdmiTx_StateStreamOff(InstancePtr, Event, &NextState);
		break;

	case XV_TX_HDMI_STATE_FRLCONFIG:
		XV_Tx_HdmiTx_StateFrlConfig(InstancePtr, Event, &NextState);
		break;

	case XV_TX_HDMI_STATE_FRLFFE:
		XV_Tx_HdmiTx_StateFrlFfe(InstancePtr, Event, &NextState);
		break;

	case XV_TX_HDMI_STATE_FRLSTART:
		XV_Tx_HdmiTx_StateFrlStart(InstancePtr, Event, &NextState);
		break;

	case XV_TX_HDMI_STATE_FRLSTOP:
		XV_Tx_HdmiTx_StateFrlStop(InstancePtr, Event, &NextState);
		break;

	case XV_TX_HDMI_STATE_TMDSCONFIG:
		XV_Tx_HdmiTx_StateTmdsConfig(InstancePtr, Event, &NextState);
		break;

	default:
		break;
	}

	if (InstancePtr->StateInfo.CurrentState != NextState) {

		xdbg_xv_tx_statemachine_print("%s,%d: Event %s : < "
				"State %s -> State %s >\r\n", __func__,
				__LINE__, XV_Tx_Hdmi_Tx_EventtoString(Event),
				XV_Tx_Hdmi_Tx_StatetoString(
					InstancePtr->StateInfo.CurrentState),
				XV_Tx_Hdmi_Tx_StatetoString(NextState));

		InstancePtr->StateInfo.PreviousState =
				InstancePtr->StateInfo.CurrentState;

		XV_Tx_HdmiTx_StateEnter(InstancePtr, NextState);

		InstancePtr->StateInfo.CurrentState = NextState;
	}

}

void XV_Tx_HdmiTx_StateEnter(XV_Tx *InstancePtr, XV_Tx_Hdmi_State State)
{
	Xil_AssertVoid(InstancePtr);

	switch (State) {
	case XV_TX_HDMI_STATE_DISCONNECTED:
		XV_Tx_HdmiTx_EnterStateDisconnected(InstancePtr);
		break;

	case XV_TX_HDMI_STATE_CONNECTED:
		XV_Tx_HdmiTx_EnterStateConnected(InstancePtr);
		break;

	case XV_TX_HDMI_STATE_BRDG_UNLOCKED:
		XV_Tx_HdmiTx_EnterStateBrdgUnlocked(InstancePtr);
		break;

	case XV_TX_HDMI_STATE_VSYNC:
		XV_Tx_HdmiTx_EnterStateVSync(InstancePtr);
		break;

	case XV_TX_HDMI_STATE_STREAMON:
		XV_Tx_HdmiTx_EnterStateStreamOn(InstancePtr);
		break;

	case XV_TX_HDMI_STATE_STREAMOFF:
		XV_Tx_HdmiTx_EnterStateStreamOff(InstancePtr);
		break;

	case XV_TX_HDMI_STATE_FRLCONFIG:
		XV_Tx_HdmiTx_EnterStateFrlConfig(InstancePtr);
		break;

	case XV_TX_HDMI_STATE_FRLFFE:
		XV_Tx_HdmiTx_EnterStateFrlFfe(InstancePtr);
		break;

	case XV_TX_HDMI_STATE_FRLSTART:
		XV_Tx_HdmiTx_EnterStateFrlStart(InstancePtr);
		break;

	case XV_TX_HDMI_STATE_FRLSTOP:
		XV_Tx_HdmiTx_EnterStateFrlStop(InstancePtr);
		break;

	case XV_TX_HDMI_STATE_TMDSCONFIG:
		XV_Tx_HdmiTx_EnterStateTmdsConfig(InstancePtr);
		break;

	default:
		break;
	}

}

void XV_Tx_HdmiTx_StateDisconnected(XV_Tx *InstancePtr,
					XV_Tx_Hdmi_Events Event,
					XV_Tx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	/* Transition from state disconnected to connected, is only
	 * allowed when a connected event is received.
	 *
	 * All other events - stream down, stream up and
	 * VSync are ignored, if the hdmi tx is in
	 * the disconnected state.
	 */
	switch (Event) {
	case XV_TX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_TX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_TX_HDMI_EVENT_CONNECTED:
		*NextStatePtr = XV_TX_HDMI_STATE_CONNECTED;
		break;

	case XV_TX_HDMI_EVENT_BRDGUNLOCKED:

		break;

	case XV_TX_HDMI_EVENT_BRDGUNDERFLOW:

		break;

	case XV_TX_HDMI_EVENT_BRDGOVERFLOW:

		break;

	case XV_TX_HDMI_EVENT_VSYNC:

		break;

	case XV_TX_HDMI_EVENT_STREAMUP:

		break;

	case XV_TX_HDMI_EVENT_STREAMDOWN:

		break;

	case XV_TX_HDMI_EVENT_POLL:

		break;

	case XV_TX_HDMI_EVENT_FRLCONFIG:

		break;

	case XV_TX_HDMI_EVENT_FRLFFE:

		break;

	case XV_TX_HDMI_EVENT_FRLSTART:

		break;

	case XV_TX_HDMI_EVENT_FRLSTOP:

		break;

	case XV_TX_HDMI_EVENT_TMDSCONFIG:

		break;

	default:
		break;
	}
}

void XV_Tx_HdmiTx_EnterStateDisconnected(XV_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	XHdmiphy1_IBufDsEnable(InstancePtr->VidPhy, 0, XHDMIPHY1_DIR_TX, (FALSE));

	if (InstancePtr->TxCableConnectionChange != NULL) {
		InstancePtr->TxCableConnectionChange(
			InstancePtr->TxCableConnectionChangeCallbackRef);
	}

#ifdef USE_HDCP_HDMI_TX
	InstancePtr->HdcpConfig.DownstreamInstanceConnected = FALSE;
	InstancePtr->HdcpConfig.DownstreamInstanceStreamUp = FALSE;
#endif
}

void XV_Tx_HdmiTx_StateConnected(XV_Tx *InstancePtr,
				 XV_Tx_Hdmi_Events Event,
				 XV_Tx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_TX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_TX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_TX_HDMI_EVENT_CONNECTED:

		break;

	case XV_TX_HDMI_EVENT_BRDGUNLOCKED:
		*NextStatePtr = XV_TX_HDMI_STATE_BRDG_UNLOCKED;
		break;

	case XV_TX_HDMI_EVENT_BRDGUNDERFLOW:
		InstancePtr->ErrorStats.TxBrdgUnderflowCnt++;
		break;

	case XV_TX_HDMI_EVENT_BRDGOVERFLOW:
		InstancePtr->ErrorStats.TxBrdgOverflowCnt++;
		break;

	case XV_TX_HDMI_EVENT_VSYNC:
		/* Wait until stream up before processing any VSync events.
		 * If a Vsync is received before stream up, that is an
		 * error condition that we are ignoring.
		 */
		break;

	case XV_TX_HDMI_EVENT_STREAMUP:
		*NextStatePtr = XV_TX_HDMI_STATE_STREAMON;
		break;

	case XV_TX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_TX_HDMI_STATE_STREAMOFF;
		break;

	case XV_TX_HDMI_EVENT_FRLCONFIG:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_TX_HDMI_EVENT_FRLFFE:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLFFE;
		break;

	case XV_TX_HDMI_EVENT_FRLSTART:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTART;
		break;

	case XV_TX_HDMI_EVENT_FRLSTOP:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTOP;
		break;

	case XV_TX_HDMI_EVENT_TMDSCONFIG:
		XV_HdmiTxSS1_SetHdmiTmdsMode(InstancePtr->HdmiTxSs);
		*NextStatePtr = XV_TX_HDMI_STATE_TMDSCONFIG;
		break;

	case XV_TX_HDMI_EVENT_POLL:

		break;

	default:
		break;
	}
}

void XV_Tx_HdmiTx_EnterStateConnected(XV_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);
	u32 FrlTrainingStatus = XST_FAILURE;

	XHdmiphy1_IBufDsEnable(InstancePtr->VidPhy, 0,
			       XHDMIPHY1_DIR_TX, (TRUE));

	/* Disable TX TDMS clock */
	XHdmiphy1_Clkout1OBufTdsEnable(InstancePtr->VidPhy,
				       XHDMIPHY1_DIR_TX, (FALSE));

	XHdmiphy1_Hdmi20Config(InstancePtr->VidPhy, 0, XHDMIPHY1_DIR_TX);

	/* Setting FRL link and video clocks to 0. */
	XV_HdmiTx1_SetFrlLinkClock(InstancePtr->HdmiTxSs->HdmiTx1Ptr, 0);
	XV_HdmiTx1_SetFrlVidClock(InstancePtr->HdmiTxSs->HdmiTx1Ptr, 0);

	if (InstancePtr->TxCableConnectionChange != NULL) {
		InstancePtr->TxCableConnectionChange(
			InstancePtr->TxCableConnectionChangeCallbackRef);
	}

	if (InstancePtr->ConfigInfo.IsFrl) {
		FrlTrainingStatus = XV_Tx_StartFrlTraining(InstancePtr,
				InstancePtr->ConfigInfo.FrlEdidInfo.IsSCDCPresent,
				InstancePtr->ConfigInfo.FrlEdidInfo.MaxFrlRateSupp);

		if (FrlTrainingStatus == XST_FAILURE) {
			/* goto TMDS mode if FRL training fails, and start tx*/
			if (InstancePtr->TxReadyToStart != NULL) {
				InstancePtr->TxReadyToStart(
					InstancePtr->TxReadyToStartCallbackRef);
			}
		}
	} else {
		/* This means that tx is ready to start */
		if (InstancePtr->TxReadyToStart != NULL) {
			InstancePtr->TxReadyToStart(InstancePtr->TxReadyToStartCallbackRef);
		}
	}

#ifdef USE_HDCP_HDMI_TX
	InstancePtr->HdcpConfig.DownstreamInstanceConnected = TRUE;
#endif

}

void XV_Tx_HdmiTx_StateBrdgUnlocked(XV_Tx *InstancePtr,
					XV_Tx_Hdmi_Events Event,
					XV_Tx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_TX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_TX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_TX_HDMI_EVENT_CONNECTED:
		*NextStatePtr = XV_TX_HDMI_STATE_CONNECTED;
		break;

	case XV_TX_HDMI_EVENT_BRDGUNLOCKED:

		break;

	case XV_TX_HDMI_EVENT_BRDGUNDERFLOW:
		InstancePtr->ErrorStats.TxBrdgUnderflowCnt++;
		break;

	case XV_TX_HDMI_EVENT_BRDGOVERFLOW:
		InstancePtr->ErrorStats.TxBrdgOverflowCnt++;
		break;

	case XV_TX_HDMI_EVENT_VSYNC:
		*NextStatePtr = XV_TX_HDMI_STATE_VSYNC;
		break;

	case XV_TX_HDMI_EVENT_STREAMUP:
		*NextStatePtr = XV_TX_HDMI_STATE_STREAMON;
		break;

	case XV_TX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_TX_HDMI_STATE_STREAMOFF;
		break;

	case XV_TX_HDMI_EVENT_FRLCONFIG:
		XV_HdmiTxSS1_SetHdmiFrlMode(InstancePtr->HdmiTxSs);
		*NextStatePtr = XV_TX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_TX_HDMI_EVENT_FRLFFE:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLFFE;
		break;

	case XV_TX_HDMI_EVENT_FRLSTART:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTART;
		break;

	case XV_TX_HDMI_EVENT_FRLSTOP:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTOP;
		break;

	case XV_TX_HDMI_EVENT_TMDSCONFIG:
		XV_HdmiTxSS1_SetHdmiTmdsMode(InstancePtr->HdmiTxSs);
		*NextStatePtr = XV_TX_HDMI_STATE_TMDSCONFIG;
		break;

	case XV_TX_HDMI_EVENT_POLL:

		break;

	default:
		break;
	}
}

void XV_Tx_HdmiTx_EnterStateBrdgUnlocked(XV_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

}

void XV_Tx_HdmiTx_StateVSync(XV_Tx *InstancePtr,
				XV_Tx_Hdmi_Events Event,
				XV_Tx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_TX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_TX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_TX_HDMI_EVENT_CONNECTED:

		break;

	case XV_TX_HDMI_EVENT_BRDGUNLOCKED:

		break;

	case XV_TX_HDMI_EVENT_BRDGUNDERFLOW:
		InstancePtr->ErrorStats.TxBrdgUnderflowCnt++;
		break;

	case XV_TX_HDMI_EVENT_BRDGOVERFLOW:
		InstancePtr->ErrorStats.TxBrdgOverflowCnt++;
		break;

	case XV_TX_HDMI_EVENT_VSYNC:
		if (InstancePtr->TxVidSyncRecv != NULL) {
			InstancePtr->TxVidSyncRecv(
				InstancePtr->TxVidSyncRecvCallbackRef);
		}
		break;

	case XV_TX_HDMI_EVENT_STREAMUP:
		*NextStatePtr = XV_TX_HDMI_STATE_STREAMON;
		break;

	case XV_TX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_TX_HDMI_STATE_STREAMOFF;
		break;

	case XV_TX_HDMI_EVENT_FRLCONFIG:
		XV_HdmiTxSS1_SetHdmiFrlMode(InstancePtr->HdmiTxSs);
		*NextStatePtr = XV_TX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_TX_HDMI_EVENT_FRLFFE:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLFFE;
		break;

	case XV_TX_HDMI_EVENT_FRLSTART:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTART;
		break;

	case XV_TX_HDMI_EVENT_FRLSTOP:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTOP;
		break;

	case XV_TX_HDMI_EVENT_TMDSCONFIG:
		XV_HdmiTxSS1_SetHdmiTmdsMode(InstancePtr->HdmiTxSs);
		*NextStatePtr = XV_TX_HDMI_STATE_TMDSCONFIG;
		break;

	case XV_TX_HDMI_EVENT_POLL:

		break;

	default:
		break;
	}
}

void XV_Tx_HdmiTx_EnterStateVSync(XV_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);
#ifdef CHECK_HDCP_CAPS_BEFORE_AUTH_REQUEST
	u32 DwnStrmIsHdcpCapable = FALSE;
#endif

	/* Check for the 1st Vsync received after stream on. */
	if (InstancePtr->ConfigInfo.Vsync0_after_steamOn == TRUE) {

		/* Clear the flag to track the
		 * first vsync after stream on. */
		InstancePtr->ConfigInfo.Vsync0_after_steamOn = FALSE;

#ifdef USE_HDCP_HDMI_TX
#ifdef CHECK_HDCP_CAPS_BEFORE_AUTH_REQUEST
		/* Start the HDCP authentication. */
		if (XV_HdmiTxSs1_IsSinkHdcp14Capable(InstancePtr->HdmiTxSs) ||
		    XV_HdmiTxSs1_IsSinkHdcp22Capable(InstancePtr->HdmiTxSs)) {
			DwnStrmIsHdcpCapable = TRUE;
		}

#endif

		if (XV_HdmiTxSs1_IsStreamConnected(InstancePtr->HdmiTxSs)
#ifdef CHECK_HDCP_CAPS_BEFORE_AUTH_REQUEST
			&& (DwnStrmIsHdcpCapable == TRUE)
#endif
			) {
			/* Use the comment below to profile the HDCP
			 * authentication attempt on the first vsync.
			 */
			/* xdbg_xv_tx_print("Received 1st Vysnc after stream " */
			/*                  "on, starting HDCP ....\r\n"); */
			XV_HdmiTxSs1_HdcpPushEvent(InstancePtr->HdmiTxSs,
						XV_HDMITXSS1_HDCP_AUTHENTICATE_EVT);

			InstancePtr->HdcpConfig.DownstreamInstanceStreamUp = TRUE;

			/* Poll the HDCP TX to ensure that
			 * authentication completes.
			 *
			 * XV_Tx_Hdcp_Poll(InstancePtr);
			 */
		}
#endif
	}

	if (InstancePtr->TxVidSyncRecv != NULL) {
		InstancePtr->TxVidSyncRecv(
			InstancePtr->TxVidSyncRecvCallbackRef);
	}

}

void XV_Tx_HdmiTx_StateStreamOn(XV_Tx *InstancePtr,
				XV_Tx_Hdmi_Events Event,
				XV_Tx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_TX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_TX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_TX_HDMI_EVENT_CONNECTED:

		break;

	case XV_TX_HDMI_EVENT_BRDGUNLOCKED:
		*NextStatePtr = XV_TX_HDMI_STATE_BRDG_UNLOCKED;
		break;

	case XV_TX_HDMI_EVENT_BRDGUNDERFLOW:
		InstancePtr->ErrorStats.TxBrdgUnderflowCnt++;
		break;

	case XV_TX_HDMI_EVENT_BRDGOVERFLOW:
		InstancePtr->ErrorStats.TxBrdgOverflowCnt++;
		break;

	case XV_TX_HDMI_EVENT_VSYNC:
		InstancePtr->ConfigInfo.Vsync0_after_steamOn = TRUE;
		*NextStatePtr = XV_TX_HDMI_STATE_VSYNC;
		break;

	case XV_TX_HDMI_EVENT_STREAMUP:

		break;

	case XV_TX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_TX_HDMI_STATE_STREAMOFF;
		break;

	case XV_TX_HDMI_EVENT_POLL:

		break;

	case XV_TX_HDMI_EVENT_FRLCONFIG:
		XV_HdmiTxSS1_SetHdmiFrlMode(InstancePtr->HdmiTxSs);
		*NextStatePtr = XV_TX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_TX_HDMI_EVENT_FRLFFE:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLFFE;
		break;

	case XV_TX_HDMI_EVENT_FRLSTART:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTART;
		break;

	case XV_TX_HDMI_EVENT_FRLSTOP:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTOP;
		break;

	case XV_TX_HDMI_EVENT_TMDSCONFIG:
		XV_HdmiTxSS1_SetHdmiTmdsMode(InstancePtr->HdmiTxSs);
		*NextStatePtr = XV_TX_HDMI_STATE_TMDSCONFIG;
		break;

	default:
		break;
	}
}

void XV_Tx_HdmiTx_EnterStateStreamOn(XV_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	XHdmiphy1 *XV_Tx_Hdmiphy1Ptr = (XHdmiphy1 *)InstancePtr->VidPhy;
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)InstancePtr->HdmiTxSs;
	u64 TxLineRate;
	XHdmiphy1_PllType TxPllType;

	/* Update the stream statistics (Yet to be added). */

	/* Set the stream state. */
	InstancePtr->XV_Tx_StreamState = XV_TX_HDMITXSS_STREAM_UP;

	/* Set HDMI/DVI mode based on the info of the downstream sink. */
	xdbg_xv_tx_print(ANSI_COLOR_YELLOW "StreamOn. "
			"ConfigInfo.IsHdmi = %d\r\n"
			ANSI_COLOR_RESET "\r\n",
			InstancePtr->ConfigInfo.IsHdmi);
	if (InstancePtr->ConfigInfo.IsHdmi == FALSE) {
		/* Downstream sink is DVI. */
		XV_HdmiTxSs1_AudioMute(HdmiTxSs1Ptr, TRUE);
		XV_HdmiTxSS1_SetDviMode(HdmiTxSs1Ptr);
	} else {
		/* Downstream sink is HDMI or HDMI FRL */
		if (XV_HdmiTxSs1_GetTransportMode(HdmiTxSs1Ptr) == TRUE) {
			XV_HdmiTxSS1_SetHdmiFrlMode(HdmiTxSs1Ptr);
		} else {
			XV_HdmiTxSS1_SetHdmiTmdsMode(HdmiTxSs1Ptr);
		}
		XV_HdmiTxSs1_AudioMute(HdmiTxSs1Ptr, FALSE);
	}

	/* Set the sampling rate. */
	XV_HdmiTxSs1_SetSamplingRate(InstancePtr->HdmiTxSs,
				     InstancePtr->VidPhy->HdmiTxSampleRate);

	/* Get the Tx PLL type and the corresponding Line rate. */
	TxPllType = XHdmiphy1_GetPllType(XV_Tx_Hdmiphy1Ptr, 0,
				XHDMIPHY1_DIR_TX, XHDMIPHY1_CHANNEL_ID_CH1);

	if ((TxPllType == XHDMIPHY1_PLL_TYPE_CPLL)) {
		TxLineRate = XHdmiphy1_GetLineRateHz(XV_Tx_Hdmiphy1Ptr, 0,
						XHDMIPHY1_CHANNEL_ID_CH1);

	} else if ((TxPllType == XHDMIPHY1_PLL_TYPE_QPLL) ||
	           (TxPllType == XHDMIPHY1_PLL_TYPE_QPLL0)) {
		TxLineRate = XHdmiphy1_GetLineRateHz(XV_Tx_Hdmiphy1Ptr, 0,
						XHDMIPHY1_CHANNEL_ID_CMN0);
	} else {
		TxLineRate = XHdmiphy1_GetLineRateHz(XV_Tx_Hdmiphy1Ptr, 0,
						XHDMIPHY1_CHANNEL_ID_CMN1);
	}

	InstancePtr->ConfigInfo.LineRate = TxLineRate;

	/* Setup the audio and video sources. */
	if (InstancePtr->SetupAudVidSrc != NULL) {
		InstancePtr->SetupAudVidSrc
			(InstancePtr->SetupAudVidSrcCallbackRef);
	}

	/* Set the Audio format and channels. */
	XV_HdmiTxSs1_SetAudioFormat(HdmiTxSs1Ptr,
				    InstancePtr->ConfigInfo.AudioFormat);
	XV_HdmiTxSs1_SetAudioChannels(HdmiTxSs1Ptr,
				      InstancePtr->ConfigInfo.AudioChannels);

	XV_HdmiTxSs1_StreamStart(InstancePtr->HdmiTxSs);

	/* Stream is on. */
	if (InstancePtr->TxStreamOn != NULL) {
		InstancePtr->TxStreamOn(InstancePtr->TxStreamOnCallbackRef);
	}

	/* Enable the cable drivers. */
	if (InstancePtr->EnableCableDriversCb != NULL) {
		InstancePtr->EnableCableDriversCb
				(InstancePtr->EnableCableDriversCbRef);
	}
	/* Enable the TMDS clock */
	XHdmiphy1_Clkout1OBufTdsEnable
				(XV_Tx_Hdmiphy1Ptr, XHDMIPHY1_DIR_TX, (TRUE));
}

void XV_Tx_HdmiTx_StateStreamOff(XV_Tx *InstancePtr,
				 XV_Tx_Hdmi_Events Event,
				 XV_Tx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_TX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_TX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_TX_HDMI_EVENT_CONNECTED:

		break;

	case XV_TX_HDMI_EVENT_BRDGUNLOCKED:
		*NextStatePtr = XV_TX_HDMI_STATE_BRDG_UNLOCKED;
		break;

	case XV_TX_HDMI_EVENT_BRDGUNDERFLOW:

		break;

	case XV_TX_HDMI_EVENT_BRDGOVERFLOW:

		break;

	case XV_TX_HDMI_EVENT_VSYNC:

		break;

	case XV_TX_HDMI_EVENT_STREAMUP:
		*NextStatePtr = XV_TX_HDMI_STATE_STREAMON;
		break;

	case XV_TX_HDMI_EVENT_STREAMDOWN:

		break;

	case XV_TX_HDMI_EVENT_FRLCONFIG:
		XV_HdmiTxSS1_SetHdmiFrlMode(InstancePtr->HdmiTxSs);
		*NextStatePtr = XV_TX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_TX_HDMI_EVENT_FRLFFE:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLFFE;
		break;

	case XV_TX_HDMI_EVENT_FRLSTART:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTART;
		break;

	case XV_TX_HDMI_EVENT_FRLSTOP:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTOP;
		break;

	case XV_TX_HDMI_EVENT_TMDSCONFIG:
		XV_HdmiTxSS1_SetHdmiTmdsMode(InstancePtr->HdmiTxSs);
		*NextStatePtr = XV_TX_HDMI_STATE_TMDSCONFIG;
		break;

	case XV_TX_HDMI_EVENT_POLL:

		break;

	default:
		break;
	}
}

void XV_Tx_HdmiTx_EnterStateStreamOff(XV_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	/* Here for robustness we should get ready to receive the stream again. */

	/* Set the stream state. */
	InstancePtr->XV_Tx_StreamState = XV_TX_HDMITXSS_STREAM_DOWN;

#ifdef USE_HDCP_HDMI_TX
	InstancePtr->HdcpConfig.DownstreamInstanceStreamUp = FALSE;
#endif

	InstancePtr->TxStreamOff(InstancePtr->TxStreamOffCallbackRef);

	/* In the future, a stream statistics counter can be maintained here. */
}

void XV_Tx_HdmiTx_StateFrlConfig(XV_Tx *InstancePtr,
					XV_Tx_Hdmi_Events Event,
					XV_Tx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_TX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_TX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_TX_HDMI_EVENT_CONNECTED:

		break;

	case XV_TX_HDMI_EVENT_BRDGUNLOCKED:
		break;

	case XV_TX_HDMI_EVENT_BRDGUNDERFLOW:

		break;

	case XV_TX_HDMI_EVENT_BRDGOVERFLOW:

		break;

	case XV_TX_HDMI_EVENT_VSYNC:

		break;

	case XV_TX_HDMI_EVENT_STREAMUP:
		break;

	case XV_TX_HDMI_EVENT_STREAMDOWN:

		break;

	case XV_TX_HDMI_EVENT_POLL:

		break;

	case XV_TX_HDMI_EVENT_FRLCONFIG:
		XV_Tx_HdmiTx_EnterStateFrlConfig(InstancePtr);
		break;

	case XV_TX_HDMI_EVENT_FRLFFE:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLFFE;
		break;

	case XV_TX_HDMI_EVENT_FRLSTART:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTART;
		break;

	case XV_TX_HDMI_EVENT_FRLSTOP:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTOP;
		break;

	case XV_TX_HDMI_EVENT_TMDSCONFIG:
		*NextStatePtr = XV_TX_HDMI_STATE_TMDSCONFIG;
		break;

	default:
		break;
	}
}

void XV_Tx_HdmiTx_EnterStateFrlConfig(XV_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	xdbg_xv_tx_print("Frl config ...\r\n");

	u64 LineRate = ((u64)(InstancePtr->HdmiTxSs->HdmiTx1Ptr->Stream.Frl.LineRate)) *
			((u64)(1e9));
	u8 NChannels = InstancePtr->HdmiTxSs->HdmiTx1Ptr->Stream.Frl.Lanes;
	/* For addtional information, print out linerate and nchannel,
	 * xdbg_xv_tx_print("LineRate: %lu, NChannels: %d\r\n",
	 *			 LineRate, NChannels);
	 */

	/* Enable GT TX Refclk Input if TMDS and FRL are not sharing the
	 * same MGTREFCLK port
	 */
	if (InstancePtr->VidPhy->Config.TxRefClkSel !=
			InstancePtr->VidPhy->Config.TxFrlRefClkSel) {
		XHdmiphy1_IBufDsEnable(InstancePtr->VidPhy, 0,
				XHDMIPHY1_DIR_TX, (TRUE));
	}

	XHdmiphy1_Hdmi21Config(InstancePtr->VidPhy, 0, XHDMIPHY1_DIR_TX,
			       LineRate, NChannels);

	if (InstancePtr->VidPhy->Config.TxRefClkSel ==
			InstancePtr->VidPhy->Config.TxFrlRefClkSel) {
		if (InstancePtr->SetupTxFrlRefClkCb != NULL) {
			InstancePtr->SetupTxFrlRefClkCb(
					InstancePtr->SetupTxFrlRefClkCbRef);
		}
	}

	XV_Tx_SetHdmiFrlMode(InstancePtr);

	if (InstancePtr->TxFrlConfigDeviceSetupCb != NULL) {
		InstancePtr->TxFrlConfigDeviceSetupCb(
				InstancePtr->TxFrlConfigDeviceSetupCbRef);
	}

}

void XV_Tx_HdmiTx_StateFrlFfe(XV_Tx *InstancePtr,
			      XV_Tx_Hdmi_Events Event,
			      XV_Tx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_TX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_TX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_TX_HDMI_EVENT_CONNECTED:

		break;

	case XV_TX_HDMI_EVENT_BRDGUNLOCKED:
		break;

	case XV_TX_HDMI_EVENT_BRDGUNDERFLOW:

		break;

	case XV_TX_HDMI_EVENT_BRDGOVERFLOW:

		break;

	case XV_TX_HDMI_EVENT_VSYNC:

		break;

	case XV_TX_HDMI_EVENT_STREAMUP:
		break;

	case XV_TX_HDMI_EVENT_STREAMDOWN:

		break;

	case XV_TX_HDMI_EVENT_POLL:

		break;

	case XV_TX_HDMI_EVENT_FRLCONFIG:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_TX_HDMI_EVENT_FRLFFE:
		XV_Tx_HdmiTx_EnterStateFrlFfe(InstancePtr);
		break;

	case XV_TX_HDMI_EVENT_FRLSTART:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTART;
		break;

	case XV_TX_HDMI_EVENT_FRLSTOP:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTOP;
		break;

	case XV_TX_HDMI_EVENT_TMDSCONFIG:
		*NextStatePtr = XV_TX_HDMI_STATE_TMDSCONFIG;
		break;

	default:
		break;
	}
}

void XV_Tx_HdmiTx_EnterStateFrlFfe(XV_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	/* Call the Frl Ffe callback for the user to configure
	 * any external cards for HDMI 2.1 during the Frl
	 * ffe training.
	 */
	InstancePtr->TxFrlFfeConfigDeviceCb(
			InstancePtr->TxFrlFfeConfigDeviceCbRef);
}

void XV_Tx_HdmiTx_StateFrlStart(XV_Tx *InstancePtr,
					XV_Tx_Hdmi_Events Event,
					XV_Tx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_TX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_TX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_TX_HDMI_EVENT_CONNECTED:

		break;

	case XV_TX_HDMI_EVENT_BRDGUNLOCKED:
		break;

	case XV_TX_HDMI_EVENT_BRDGUNDERFLOW:

		break;

	case XV_TX_HDMI_EVENT_BRDGOVERFLOW:

		break;

	case XV_TX_HDMI_EVENT_VSYNC:

		break;

	case XV_TX_HDMI_EVENT_STREAMUP:
		*NextStatePtr = XV_TX_HDMI_STATE_STREAMON;
		break;

	case XV_TX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_TX_HDMI_STATE_STREAMOFF;
		break;

	case XV_TX_HDMI_EVENT_POLL:

		break;

	case XV_TX_HDMI_EVENT_FRLCONFIG:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_TX_HDMI_EVENT_FRLFFE:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLFFE;
		break;

	case XV_TX_HDMI_EVENT_FRLSTART:
		break;

	case XV_TX_HDMI_EVENT_FRLSTOP:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTOP;
		break;

	case XV_TX_HDMI_EVENT_TMDSCONFIG:
		*NextStatePtr = XV_TX_HDMI_STATE_TMDSCONFIG;
		break;

	default:
		break;
	}
}

void XV_Tx_HdmiTx_EnterStateFrlStart(XV_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	InstancePtr->TxReadyToStart(InstancePtr->TxReadyToStartCallbackRef);
}

void XV_Tx_HdmiTx_StateFrlStop(XV_Tx *InstancePtr,
				XV_Tx_Hdmi_Events Event,
				XV_Tx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_TX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_TX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_TX_HDMI_EVENT_CONNECTED:

		break;

	case XV_TX_HDMI_EVENT_BRDGUNLOCKED:
		break;

	case XV_TX_HDMI_EVENT_BRDGUNDERFLOW:

		break;

	case XV_TX_HDMI_EVENT_BRDGOVERFLOW:

		break;

	case XV_TX_HDMI_EVENT_VSYNC:

		break;

	case XV_TX_HDMI_EVENT_STREAMUP:
		*NextStatePtr = XV_TX_HDMI_STATE_STREAMON;
		break;

	case XV_TX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_TX_HDMI_STATE_STREAMOFF;
		break;

	case XV_TX_HDMI_EVENT_POLL:

		break;

	case XV_TX_HDMI_EVENT_FRLCONFIG:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_TX_HDMI_EVENT_FRLFFE:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLFFE;
		break;

	case XV_TX_HDMI_EVENT_FRLSTART:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTART;
		break;

	case XV_TX_HDMI_EVENT_FRLSTOP:
		break;

	case XV_TX_HDMI_EVENT_TMDSCONFIG:
		*NextStatePtr = XV_TX_HDMI_STATE_TMDSCONFIG;
		break;

	default:
		break;
	}
}

void XV_Tx_HdmiTx_EnterStateFrlStop(XV_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	/* Nothing to do here yet . */
}

void XV_Tx_HdmiTx_StateTmdsConfig(XV_Tx *InstancePtr,
					XV_Tx_Hdmi_Events Event,
					XV_Tx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_TX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_TX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_TX_HDMI_EVENT_CONNECTED:

		break;

	case XV_TX_HDMI_EVENT_BRDGUNLOCKED:
		break;

	case XV_TX_HDMI_EVENT_BRDGUNDERFLOW:

		break;

	case XV_TX_HDMI_EVENT_BRDGOVERFLOW:

		break;

	case XV_TX_HDMI_EVENT_VSYNC:

		break;

	case XV_TX_HDMI_EVENT_STREAMUP:
		*NextStatePtr = XV_TX_HDMI_STATE_STREAMON;
		break;

	case XV_TX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_TX_HDMI_STATE_STREAMOFF;
		break;

	case XV_TX_HDMI_EVENT_POLL:

		break;

	case XV_TX_HDMI_EVENT_FRLCONFIG:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_TX_HDMI_EVENT_FRLFFE:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLFFE;
		break;

	case XV_TX_HDMI_EVENT_FRLSTART:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTART;
		break;

	case XV_TX_HDMI_EVENT_FRLSTOP:
		*NextStatePtr = XV_TX_HDMI_STATE_FRLSTOP;
		break;

	case XV_TX_HDMI_EVENT_TMDSCONFIG:
		XV_Tx_HdmiTx_EnterStateTmdsConfig(InstancePtr);
		break;

	default:
		break;
	}
}

void XV_Tx_HdmiTx_EnterStateTmdsConfig(XV_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	xdbg_xv_tx_print("Tmds Config ...\r\n");

	XHdmiphy1_Hdmi20Config(InstancePtr->VidPhy, 0, XHDMIPHY1_DIR_TX);
	XV_HdmiTx1_SetFrlLinkClock(InstancePtr->HdmiTxSs->HdmiTx1Ptr, 0);
	XV_HdmiTx1_SetFrlVidClock(InstancePtr->HdmiTxSs->HdmiTx1Ptr, 0);

	if (InstancePtr->ConfigInfo.IsHdmi == FALSE) {
		/* If downstream sink only suports DVI, then we must
		 * set the HDMI mode to DVI.
		 */
		XV_HdmiTxSS1_SetDviMode(InstancePtr->HdmiTxSs);
	} else {
		/* If the downstream sink is capable of HDMI or HDMI FRL
		 * then we set the HDMI mode to TMDS as the source has
		 * given us a tmds config request.
		 */
		XV_HdmiTxSS1_SetHdmiTmdsMode(InstancePtr->HdmiTxSs);
	}

	if (InstancePtr->TxReadyToStart != NULL) {
		InstancePtr->TxReadyToStart(
				InstancePtr->TxReadyToStartCallbackRef);
	}
}

const char *XV_Tx_Hdmi_Tx_StatetoString(XV_Tx_Hdmi_State State)
{
	const char *stateNameString;

	switch (State) {
	case XV_TX_HDMI_STATE_DISCONNECTED:
		stateNameString = "dis-connected";
		break;

	case XV_TX_HDMI_STATE_CONNECTED:
		stateNameString = "connected";
		break;

	case XV_TX_HDMI_STATE_BRDG_UNLOCKED:
		stateNameString = "bridge-unlocked";
		break;

	case XV_TX_HDMI_STATE_VSYNC:
		stateNameString = "vsync";
		break;

	case XV_TX_HDMI_STATE_STREAMON:
		stateNameString = "stream-on";
		break;

	case XV_TX_HDMI_STATE_STREAMOFF:
		stateNameString = "stream-off";
		break;

	case XV_TX_HDMI_STATE_FRLCONFIG:
		stateNameString = "frl-config";
		break;

	case XV_TX_HDMI_STATE_FRLFFE:
		stateNameString = "frl-fee";
		break;

	case XV_TX_HDMI_STATE_FRLSTART:
		stateNameString = "frl-start";
		break;

	case XV_TX_HDMI_STATE_FRLSTOP:
		stateNameString = "frl-stop";
		break;

	case XV_TX_HDMI_STATE_TMDSCONFIG:
		stateNameString = "tmds-config";
		break;

	default:
		stateNameString = "unknown";
		break;
	}

	return stateNameString;
}

const char *XV_Tx_Hdmi_Tx_EventtoString(XV_Tx_Hdmi_Events Event)
{
	const char *eventNameString;

	switch (Event) {
	case XV_TX_HDMI_EVENT_DISCONNECTED:
		eventNameString = "dis-connected";
		break;

	case XV_TX_HDMI_EVENT_CONNECTED:
		eventNameString = "connected";
		break;

	case XV_TX_HDMI_EVENT_BRDGUNLOCKED:
		eventNameString = "bridge-unlocked";
		break;

	case XV_TX_HDMI_EVENT_BRDGUNDERFLOW:
		eventNameString = "bridge-underflow";
		break;

	case XV_TX_HDMI_EVENT_BRDGOVERFLOW:
		eventNameString = "bridge-overflow";
		break;

	case XV_TX_HDMI_EVENT_VSYNC:
		eventNameString = "vsync";
		break;

	case XV_TX_HDMI_EVENT_STREAMUP:
		eventNameString = "stream-up";
		break;

	case XV_TX_HDMI_EVENT_STREAMDOWN:
		eventNameString = "stream-down";
		break;

	case XV_TX_HDMI_EVENT_POLL:
		eventNameString = "poll";
		break;

	case XV_TX_HDMI_EVENT_FRLCONFIG:
		eventNameString = "frl-config";
		break;

	case XV_TX_HDMI_EVENT_FRLFFE:
		eventNameString = "frl-ffe";
		break;

	case XV_TX_HDMI_EVENT_FRLSTART:
		eventNameString = "frl-start";
		break;

	case XV_TX_HDMI_EVENT_FRLSTOP:
		eventNameString = "frl-stop";
		break;

	case XV_TX_HDMI_EVENT_TMDSCONFIG:
		eventNameString = "tmds-config";
		break;

	default:
		eventNameString = "unknown";
		break;
	}

	return eventNameString;
}
