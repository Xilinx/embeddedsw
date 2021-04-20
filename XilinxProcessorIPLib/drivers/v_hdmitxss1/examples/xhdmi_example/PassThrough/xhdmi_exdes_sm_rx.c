/******************************************************************************
* Copyright (C) 2018 – 2021 Xilinx, Inc.  All rights reserved.
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
#include "xhdmi_exdes_sm_rx.h"
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES

#if (XPAR_HDMIPHY1_0_TRANSCEIVER == 6) /*GTYE4*/
#define XPS_BOARD_VCU118
#else
/* Place-holder for other boards in future */
#endif


/************************** Constant Definitions ****************************/

static const XV_Rx_Hdmi_Events XHdmi_Rx_EventsPriorityQueue[] = {
		XV_RX_HDMI_EVENT_DISCONNECTED,
		XV_RX_HDMI_EVENT_CONNECTED,
		XV_RX_HDMI_EVENT_STREAMINIT,
		XV_RX_HDMI_EVENT_STREAMUP,
		XV_RX_HDMI_EVENT_STREAMDOWN,
		XV_RX_HDMI_NUM_EVENTS
};

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
XV_Rx xhdmi_example_rx_controller;

XV_Rx_Debug_Printf XV_Rx_DebugPrintf = NULL;
XV_Rx_Debug_Printf XV_RxDebugRxSMPrintf = NULL;

/************************** Function Prototypes *****************************/

static u32 XV_Rx_HdmiRx_StreamInit_CfgMmcm(XV_Rx *InstancePtr);

#ifdef USE_HDCP_HDMI_RX
static void XV_Rx_Hdcp_StreamManageRequest_Cb(void *CallbackRef);
static void XV_Rx_Hdcp_Authenticated_Cb(void *CallbackRef);
static void XV_Rx_Hdcp_Unauthenticated_Cb(void *CallbackRef);
static void XV_Rx_Hdcp_EncryptionUpdate_Cb(void *CallbackRef);
#endif

static void XV_Rx_HdmiRx_Connect_Cb(void *CallbackRef);
static void XV_Rx_HdmiRx_BrdgOverFlow_Cb(void *CallbackRef);
static void XV_Rx_HdmiRx_Aux_Cb(void *CallbackRef);
static void XV_Rx_HdmiRx_Audio_Cb(void *CallbackRef);
static void XV_Rx_HdmiRx_LinkStatus_Cb(void *CallbackRef);
static void XV_Rx_HdmiRx_Ddc_Cb(void *CallbackRef);
static void XV_Rx_HdmiRx_StreamDown_Cb(void *CallbackRef);
static void XV_Rx_HdmiRx_StreamInit_Cb(void *CallbackRef);
static void XV_Rx_HdmiRx_StreamUp_Cb(void *CallbackRef);
static void XV_Rx_HdmiRx_PhyReset_Cb(void *CallbackRef);
static void XV_Rx_HdmiRx_FrlConfig_Cb(void *CallbackRef);
static void XV_Rx_HdmiRx_FrlStart_Cb(void *CallbackRef);
static void XV_Rx_HdmiRx_TmdsConfig_Cb(void *CallbackRef);
static void XV_Rx_HdmiRX_VrrVfp_Cb(void *CallbackRef);
static void XV_Rx_HdmiRX_VtemPkt_Cb(void *CallbackRef);
static void XV_Rx_HdmiRX_DynHdrPkt_Cb(void *CallbackRef);

static void XV_Rx_HdmiRx_StateDisconnected(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr);
static void XV_Rx_HdmiRx_StateConnected(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr);
static void XV_Rx_HdmiRx_StateNoStream(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr);
static void XV_Rx_HdmiRx_StateStreamInitialized(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr);
static void XV_Rx_HdmiRx_StateStreamOn(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr);
static void XV_Rx_HdmiRx_StateStreamOff(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr);
static void XV_Rx_HdmiRx_StatePhyReset(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr);
static void XV_Rx_HdmiRx_StateFrlConfig(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr);
static void XV_Rx_HdmiRx_StateFrlStart(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr);
static void XV_Rx_HdmiRx_StateTmdsConfig(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr);

/* *** Enter State *** */
static void XV_Rx_HdmiRx_EnterStateDisconnected(XV_Rx *InstancePtr);
static void XV_Rx_HdmiRx_EnterStateConnected(XV_Rx *InstancePtr);
static void XV_Rx_HdmiRx_EnterStateNoStream(XV_Rx *InstancePtr);
static void XV_Rx_HdmiRx_EnterStateStreamInitialized(XV_Rx *InstancePtr);
static void XV_Rx_HdmiRx_EnterStateStreamOn(XV_Rx *InstancePtr);
static void XV_Rx_HdmiRx_EnterStateStreamOff(XV_Rx *InstancePtr);
static void XV_Rx_HdmiRx_EnterStatePhyReset(XV_Rx *InstancePtr);
static void XV_Rx_HdmiRx_EnterStateFrlConfig(XV_Rx *InstancePtr);
static void XV_Rx_HdmiRx_EnterStateFrlStart(XV_Rx *InstancePtr);
static void XV_Rx_HdmiRx_EnterStateTmdsConfig(XV_Rx *InstancePtr);

static void XV_Rx_HdmiRx_ProcessPendingEvents(XV_Rx *InstancePtr);
/* Not defining the PushEvent API as static as it is currently
 * unused. If a priority queue is used for handling interrupts,
 * it is recommended that this API be static.
 */
void XV_Rx_HdmiRx_PushEvent(XV_Rx *InstancePtr,
				XV_Rx_Hdmi_Events Event);

static void XV_Rx_HdmiRx_SendEvent(XV_Rx *InstancePtr,
				XV_Rx_Hdmi_Events Event);
static void XV_Rx_HdmiRx_ProcessEvents(XV_Rx *InstancePtr,
				XV_Rx_Hdmi_Events Event);
static void XV_Rx_HdmiRx_StateEnter(XV_Rx *InstancePtr,
				XV_Rx_Hdmi_State State);

const char *XV_Rx_Hdmi_Rx_StatetoString(XV_Rx_Hdmi_State State);
const char *XV_Rx_Hdmi_Rx_EventtoString(XV_Rx_Hdmi_Events Event);

/************************** Function Definitions ****************************/

/**** **** **** **** **** ****  VPHY callbacks  **** **** **** **** **** ****/

/*****************************************************************************/
/**
*
* This function is called when the GT RX reference input clock has changed.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void Hdmiphy1HdmiRxInitCallback(void *CallbackRef)
{

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	XHdmiphy1 *Hdmiphy1Ptr = (XHdmiphy1 *)recvInst->VidPhy;
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)recvInst->HdmiRxSs;

	xdbg_xv_rx_print("%s,%d\r\n", __func__, __LINE__);

	XV_HdmiRxSs1_RefClockChangeInit(HdmiRxSs1Ptr);
	Hdmiphy1Ptr->HdmiRxTmdsClockRatio = HdmiRxSs1Ptr->TMDSClockRatio;
}

/*****************************************************************************/
/**
*
* This function is called when the GT RX has been initialized.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void Hdmiphy1HdmiRxReadyCallback(void *CallbackRef)
{
	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	XHdmiphy1 *Hdmiphy1Ptr = (XHdmiphy1 *)recvInst->VidPhy;
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)recvInst->HdmiRxSs;

	XHdmiphy1_PllType RxPllType;

	RxPllType = XHdmiphy1_GetPllType(Hdmiphy1Ptr, 0,
				     XHDMIPHY1_DIR_RX,
				     XHDMIPHY1_CHANNEL_ID_CH1);

	xdbg_xv_rx_print("%s,%d : XHdmiphy1_ClkDetGetRefClkFreqHz: %d\r\n",
			__func__, __LINE__, Hdmiphy1Ptr->HdmiRxRefClkHz);

	if (!(RxPllType == XHDMIPHY1_PLL_TYPE_CPLL)) {
		XV_HdmiRxSs1_SetStream(HdmiRxSs1Ptr,
				       Hdmiphy1Ptr->HdmiRxRefClkHz,
				       (XHdmiphy1_GetLineRateHz(
						Hdmiphy1Ptr, 0,
						XHDMIPHY1_CHANNEL_ID_CMN0) /
					1000000));

	} else {
		XV_HdmiRxSs1_SetStream(HdmiRxSs1Ptr,
				       Hdmiphy1Ptr->HdmiRxRefClkHz,
				       (XHdmiphy1_GetLineRateHz(
						Hdmiphy1Ptr, 0,
						XHDMIPHY1_CHANNEL_ID_CH1) /
					1000000));
	}
}

void XV_Rx_VPhy_SetCallbacks(XV_Rx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	XHdmiphy1 *Hdmiphy1Inst = InstancePtr->VidPhy;

	XHdmiphy1_SetHdmiCallback(Hdmiphy1Inst,
				XHDMIPHY1_HDMI_HANDLER_RXINIT,
				(void *)Hdmiphy1HdmiRxInitCallback,
				(void *)InstancePtr);

	XHdmiphy1_SetHdmiCallback(Hdmiphy1Inst,
				XHDMIPHY1_HDMI_HANDLER_RXREADY,
				(void *)Hdmiphy1HdmiRxReadyCallback,
				(void *)InstancePtr);

}

/* * * * * * * * * * * * * * * * * VPHY * * * * * * * * * * * * * * * * * * */

void XV_Rx_SetDebugPrints(XV_Rx_Debug_Printf PrintFunc)
{
    XV_Rx_DebugPrintf = PrintFunc;
}

void XV_Rx_SetDebugStateMachinePrints(XV_Rx_Debug_Printf PrintFunc)
{
	XV_RxDebugRxSMPrintf = PrintFunc;
}

void XV_Rx_HdmiPoll(XV_Rx *InstancePtr)
{
	XV_Rx_HdmiRx_ProcessPendingEvents(InstancePtr);
}

void XV_Rx_Hdcp_Poll(XV_Rx *InstancePtr)
{
#ifdef USE_HDCP_HDMI_RX
	/* Check if the connection exists before polling
	 * HDCP to save processing load.
	 *
	 * As the Poll does the 'process event' for the HDMIRXSS HDCP
	 * state machine, we must always keep calling it and react
	 * to the upstream access, regarless of the stream status. */
	/* if (XV_HdmiRxSs1_HdcpIsReady(InstancePtr->HdmiRxSs)) { */
		XV_HdmiRxSs1_HdcpPoll(InstancePtr->HdmiRxSs);
	/* } */
#endif
}

#ifdef USE_HDCP_HDMI_RX
/*****************************************************************************/
/**
*
* This function is called to set the HDCP capability on the upstream
* interface. HPD is toggled to get the attention of the transmitter.
*
* @param    InstancePtr is a pointer to the XHdcp_Repeater instance.
* @param    Protocol is defined by XV_HdmiRxSs1_HdcpProtocol type with the
*           following valid options:
*           - XV_HDMIRXSS1_HDCP_NONE sets protocol to none
*           - XV_HDMIRXSS1_HDCP_BOTH set protocol to both
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_Rx_Hdcp_SetCapability(XV_Rx *InstancePtr, int Protocol)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	if (InstancePtr->HdcpConfig.IsReady) {
		/* Set desired upstream capability */
		XV_HdmiRxSs1_HdcpSetCapability(InstancePtr->HdmiRxSs, Protocol);

		/* Toggle HPD to get attention of upstream transmitter */
		XV_HdmiRxSs1_ToggleHpd(InstancePtr->HdmiRxSs);
	}
}

u8 XV_Rx_Hdcp_GetStreamType(XV_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return InstancePtr->HdcpConfig.StreamType;

	/* ALthernatively, the stream type can be retrived from the RX SS,
	 *	u8 StreamType;
	 *	StreamType = XV_HdmiRxSs1_HdcpGetContentStreamType(
	 *					InstancePtr->HdmiRxSs);
	 */
}
#endif

/*****************************************************************************/
/**
*
* This function sets the HPD on the HDMI RXSS.
*
* @param  VphyPtr is a pointer to the VPHY instance.
* @param  HdmiRxSsPtr is a pointer to the HDMI RX Subsystem instance.
* @param  Hpd is a flag used to set the HPD.
*   - TRUE drives HPD high
*   - FALSE drives HPD low
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Rx_SetHpd(XV_Rx *InstancePtr, u8 Hpd)
{
	XV_HdmiRxSs1 *HdmiRxSsPtr = InstancePtr->HdmiRxSs;
	XHdmiphy1 *HdmiphyPtr = InstancePtr->VidPhy;

	if (Hpd == TRUE) {
		XV_HdmiRxSs1_SetHpd(HdmiRxSsPtr, Hpd);
		XHdmiphy1_IBufDsEnable(HdmiphyPtr, 0, XHDMIPHY1_DIR_RX, (TRUE));
	} else {
		XHdmiphy1_MmcmPowerDown(HdmiphyPtr, 0, XHDMIPHY1_DIR_RX, FALSE);
		XHdmiphy1_Clkout1OBufTdsEnable(HdmiphyPtr, XHDMIPHY1_DIR_RX, (FALSE));
		XHdmiphy1_IBufDsEnable(HdmiphyPtr, 0, XHDMIPHY1_DIR_RX, (FALSE));
		XV_HdmiRxSs1_SetHpd(HdmiRxSsPtr, Hpd);
	}
}

u8 XV_Rx_IsStreamOn(XV_Rx *InstancePtr)
{
	if (InstancePtr->StateInfo.CurrentState == XV_RX_HDMI_STATE_STREAMON) {
		return TRUE;
	} else {
		return FALSE;
	}
}

u8 XV_Rx_IsConnected(XV_Rx *InstancePtr)
{
	if (InstancePtr->StateInfo.CurrentState == XV_RX_HDMI_STATE_DISCONNECTED) {
		return FALSE;
	} else {
		return TRUE;
	}
}

u32 XV_Rx_PulsePllReset(XV_Rx *InstancePtr)
{
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = InstancePtr->HdmiRxSs;
	XHdmiphy1 *Hdmiphy1Ptr = InstancePtr->VidPhy;

	/* Reset RX when the link error has reached its maximum */
	if (HdmiRxSs1Ptr->IsLinkStatusErrMax &&
		(Hdmiphy1Ptr->Quads[0].Plls[0].RxState == XHDMIPHY1_GT_STATE_READY)) {

		/* Pulse RX PLL reset */
		XHdmiphy1_ClkDetFreqReset(Hdmiphy1Ptr, 0, XHDMIPHY1_DIR_RX);
	} else {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

void XV_Rx_TmdsClkEnable(XV_Rx *InstancePtr, u32 Set)
{
	XHdmiphy1_Clkout1OBufTdsEnable(InstancePtr->VidPhy,
			XHDMIPHY1_DIR_RX, Set);
}

u64 XV_Rx_GetLineRate(XV_Rx *InstancePtr)
{
	u64 LineRate;

	XHdmiphy1_PllType RxPllType;
	RxPllType = XHdmiphy1_GetPllType(InstancePtr->VidPhy,
				     0,
				     XHDMIPHY1_DIR_RX,
				     XHDMIPHY1_CHANNEL_ID_CH1);

	if (!(RxPllType == XHDMIPHY1_PLL_TYPE_CPLL)) {
		LineRate = InstancePtr->VidPhy->Quads[0].Plls[
					XHDMIPHY1_CHANNEL_ID_CMN0 -
					XHDMIPHY1_CHANNEL_ID_CH1].LineRateHz;
	} else {
		LineRate = InstancePtr->VidPhy->Quads[0].Plls[0].LineRateHz;
	}

	return LineRate;
}

XHdmiC_SamplingFrequencyVal XV_Rx_GetFrlAudSampFreq (XV_Rx *InstancePtr)
{
	XHdmiC_FRLCharRate FRLCharRate;
	XHdmiC_SamplingFrequencyVal SampFreq = XHDMIC_SAMPLING_FREQ;
	u32 CTSVal;
	u32 NVal;

	CTSVal =  XV_HdmiRxSs1_GetAudioAcrCtsVal(InstancePtr->HdmiRxSs);
	NVal = XV_HdmiRxSs1_GetAudioAcrNVal(InstancePtr->HdmiRxSs);
	InstancePtr->AcrNVal   = NVal;
	InstancePtr->AcrCtsVal = CTSVal;

	if (InstancePtr->HdmiRxSs->HdmiRx1Ptr->Stream.IsFrl == TRUE) {

	    switch (InstancePtr->HdmiRxSs->HdmiRx1Ptr->Stream.Frl.LineRate) {
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
	    SampFreq = XHdmiC_FRL_GetAudSampFreq(FRLCharRate,
	    				CTSVal, NVal);
	}
	return SampFreq;

}

XHdmiC_SamplingFrequencyVal XV_Rx_GetTmdsAudSampFreq (XV_Rx *InstancePtr)
{
	u32 RefClk;
	XHdmiC_SamplingFrequency SampFreq = XHDMIC_SAMPLING_FREQ;
	XHdmiC_SamplingFrequencyVal SampFreqVal = XHDMIC_SAMPLING_FREQUENCY;
	u32 NVal;
	u32 CTSVal;

	NVal = XV_HdmiRxSs1_GetAudioAcrNVal(InstancePtr->HdmiRxSs);
	CTSVal =  XV_HdmiRxSs1_GetAudioAcrCtsVal(InstancePtr->HdmiRxSs);
	if (InstancePtr->HdmiRxSs->TMDSClockRatio)
		RefClk = InstancePtr->VidPhy->HdmiRxRefClkHz * 4;
	else
		RefClk = InstancePtr->VidPhy->HdmiRxRefClkHz ;
	InstancePtr->AcrNVal = NVal;
	InstancePtr->AcrCtsVal = CTSVal;

	if (InstancePtr->HdmiRxSs->HdmiRx1Ptr->Stream.IsFrl != TRUE) {
		SampFreq = XHdmiC_TMDS_GetAudSampFreq(RefClk, NVal, CTSVal);
		SampFreqVal = XHdmiC_GetAudSampFreqVal(SampFreq);
	}
	return SampFreqVal;

}

u32 XV_Rx_AcrNValDiffCheck(XV_Rx *InstancePtr)
{
	u32 NVal = 0;

	NVal = XV_HdmiRxSs1_GetAudioAcrNVal(InstancePtr->HdmiRxSs);

	if (NVal != InstancePtr->AcrNVal) {
		InstancePtr->AcrNVal = NVal;
		return XST_SUCCESS;
	}
	return XST_FAILURE;
}

static u32 XV_Rx_HdmiRx_StreamInit_CfgMmcm(XV_Rx *InstancePtr)
{

	XVidC_VideoStream *HdmiRxSsVidStreamPtr;
	u32 Status = XST_SUCCESS;
	XVidC_PixelsPerClock CorePpc;
	/* Calculate RX MMCM parameters
	 * In the application the YUV422 colordepth is 12 bits
	 * However the HDMI transports YUV422 in 8 bits.
	 * Therefore force the colordepth to 8 bits when the
	 * colorspace is YUV422
	 */

	HdmiRxSsVidStreamPtr = XV_HdmiRxSs1_GetVideoStream(InstancePtr->HdmiRxSs);
	CorePpc =  XV_HdmiRxSs1_GetCorePpc(InstancePtr->HdmiRxSs);

	/* An additonal check can be added here, to ensure that the input
	 * for mmcm params are valid here.
	 */

	if (HdmiRxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_YCRCB_422) {
		Status = XHdmiphy1_HdmiCfgCalcMmcmParam(&Hdmiphy1,
	                                        0,
	                                        XHDMIPHY1_CHANNEL_ID_CH1,
	                                        XHDMIPHY1_DIR_RX,
											CorePpc,
	                                        XVIDC_BPC_8);
	}
	/* Other colorspaces */
	else {
		Status = XHdmiphy1_HdmiCfgCalcMmcmParam(&Hdmiphy1,
	                                        0,
	                                        XHDMIPHY1_CHANNEL_ID_CH1,
	                                        XHDMIPHY1_DIR_RX,
											CorePpc,
	                                        HdmiRxSsVidStreamPtr->ColorDepth);
	}

	if (Status == XST_FAILURE) {
	        return XST_FAILURE;
	}

	/* Enable and configure RX MMCM */
	XHdmiphy1_MmcmStart(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX);

	return Status;
}

u32 XV_Rx_SetTriggerCallbacks(XV_Rx *InstancePtr,
		XV_Rx_Trigger_CallbackHandler Handler,
		void *Callback, void *CallbackRef)
{
	int Status = XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr);
	Xil_AssertNonvoid(Callback);
	Xil_AssertNonvoid(CallbackRef);

	switch (Handler) {
	case XV_RX_TRIG_HANDLER_CONNECTION_CHANGE:
		InstancePtr->RxCableConnectionChange = Callback;
		InstancePtr->RxCableConnectionChangeCallbackRef = CallbackRef;
		break;

	case XV_RX_TRIG_HANDLER_STREAM_OFF:
		InstancePtr->RxStreamOff = Callback;
		InstancePtr->RxStreamOffCallbackRef = CallbackRef;
		break;

	case XV_RX_TRIG_HANDLER_STREAM_ON:
		InstancePtr->RxStreamOn = Callback;
		InstancePtr->RxStreamOnCallbackRef = CallbackRef;
		break;

	case XV_RX_TRIG_HANDLER_AUDIOCONFIG:
		InstancePtr->RxAudio = Callback;
		InstancePtr->RxAudioCallbackRef = CallbackRef;
		break;

	case XV_RX_TRIG_HANDLER_AUXEVENT:
		InstancePtr->RxAuxCb = Callback;
		InstancePtr->RxAuxCbRef = CallbackRef;
		break;

	case XV_RX_TRIG_HANDLER_CLKSRC_CONFIG:
		InstancePtr->RxClkSrcConfig = Callback;
		InstancePtr->RxClkSrcConfigCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XV_RX_TRIG_HANDLER_CLKSRC_SEL:
		InstancePtr->RxClkSrcSelCb = Callback;
		InstancePtr->RxClkSrcSelCallbackRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case XV_RX_TRIG_HANDLER_VRRVFPEVENT:
		InstancePtr->RxVrrVfpCb = Callback;
		InstancePtr->RxVrrVfpCbRef = CallbackRef;
		break;

	case XV_RX_TRIG_HANDLER_VTEMEVENT:
		InstancePtr->RxVtemCb = Callback;
		InstancePtr->RxVtemCbRef = CallbackRef;
		break;

	case XV_RX_TRIG_HANDLER_DYNHDREVENT:
		InstancePtr->RxDynHdrCb = Callback;
		InstancePtr->RxDynHdrCbRef = CallbackRef;
		break;

#ifdef USE_HDCP_HDMI_RX
	case XV_RX_TRIG_HANDLER_HDCP_SET_CONTENTSTREAMTYPE:
		InstancePtr->HdcpSetContentStreamTypeCb = Callback;
		InstancePtr->HdcpSetContentStreamTypeCbRef = CallbackRef;
		break;

	case XV_RX_TRIG_HANDLER_HDCP_FORCE_BLANKING:
		InstancePtr->HdcpForceBlankCb = Callback;
		InstancePtr->HdcpForceBlankCbRef = CallbackRef;
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
* This function initializes the callbacks for the HDMI RX sub-system events.
*
* @param    InstancePtr is the pointer reference to the Xilinx Video
*           Receiver instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
u32 XV_Rx_HdmiRxSs_Setup_Callbacks(XV_Rx *InstancePtr)
{
	u32 Status = XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
					XV_HDMIRXSS1_HANDLER_CONNECT,
					(void *)XV_Rx_HdmiRx_Connect_Cb,
					(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
					XV_HDMIRXSS1_HANDLER_BRDGOVERFLOW,
					(void *)XV_Rx_HdmiRx_BrdgOverFlow_Cb,
					(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
					XV_HDMIRXSS1_HANDLER_AUX,
					(void *)XV_Rx_HdmiRx_Aux_Cb,
					(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
					XV_HDMIRXSS1_HANDLER_AUD,
					(void *)XV_Rx_HdmiRx_Audio_Cb,
					(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
					XV_HDMIRXSS1_HANDLER_LNKSTA,
					(void *)XV_Rx_HdmiRx_LinkStatus_Cb,
					(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
					XV_HDMIRXSS1_HANDLER_DDC,
					(void *)XV_Rx_HdmiRx_Ddc_Cb,
					(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
					XV_HDMIRXSS1_HANDLER_STREAM_DOWN,
					(void *)XV_Rx_HdmiRx_StreamDown_Cb,
					(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
					XV_HDMIRXSS1_HANDLER_STREAM_INIT,
					(void *)XV_Rx_HdmiRx_StreamInit_Cb,
					(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
					XV_HDMIRXSS1_HANDLER_STREAM_UP,
					(void *)XV_Rx_HdmiRx_StreamUp_Cb,
					(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
					XV_HDMIRXSS1_HANDLER_PHY_RESET,
					(void *)XV_Rx_HdmiRx_PhyReset_Cb,
					(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
					XV_HDMIRXSS1_HANDLER_FRL_CONFIG,
					(void *)XV_Rx_HdmiRx_FrlConfig_Cb,
					(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
					XV_HDMIRXSS1_HANDLER_FRL_START,
					(void *)XV_Rx_HdmiRx_FrlStart_Cb,
					(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
					XV_HDMIRXSS1_HANDLER_TMDS_CONFIG,
					(void *)XV_Rx_HdmiRx_TmdsConfig_Cb,
					(void *)InstancePtr);


	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
			        XV_HDMIRXSS1_HANDLER_VFP_CH,
					(void *)XV_Rx_HdmiRX_VrrVfp_Cb,
					(void *)InstancePtr);


	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
			        XV_HDMIRXSS1_HANDLER_VRR_RDY,
					(void *)XV_Rx_HdmiRX_VtemPkt_Cb,
					(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
			        XV_HDMIRXSS1_HANDLER_DYN_HDR,
					(void *)XV_Rx_HdmiRX_DynHdrPkt_Cb,
					(void *)InstancePtr);

	return Status;
}

#ifdef USE_HDCP_HDMI_RX
/*****************************************************************************/
/**
* This function initializes the callbacks for the HDCP functionality of
* HDMI RX SS.
*
* @param    InstancePtr is the pointer reference to the Xilinx Video
*           Receiver instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
u32 XV_Rx_Hdcp_SetCallbacks(XV_Rx *InstancePtr)
{
	u32 Status = XST_SUCCESS;

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
				XV_HDMIRXSS1_HANDLER_HDCP_STREAM_MANAGE_REQUEST,
				(void *)XV_Rx_Hdcp_StreamManageRequest_Cb,
				(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
				XV_HDMIRXSS1_HANDLER_HDCP_AUTHENTICATED,
				(void *)XV_Rx_Hdcp_Authenticated_Cb,
				(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
				XV_HDMIRXSS1_HANDLER_HDCP_UNAUTHENTICATED,
				(void *)XV_Rx_Hdcp_Unauthenticated_Cb,
				(void *)InstancePtr);

	Status |= XV_HdmiRxSs1_SetCallback(InstancePtr->HdmiRxSs,
				XV_HDMIRXSS1_HANDLER_HDCP_ENCRYPTION_UPDATE,
				(void *)XV_Rx_Hdcp_EncryptionUpdate_Cb,
				(void *)InstancePtr);

	return Status;
}
#endif

/*****************************************************************************/
/**
* This function initializes the HdmiRxSs, and corresponding Hdmiphy1 instance
* and sets the corresponding interrupts and their callbacks.
*
* @param    InstancePtr is the callback reference to the HDMI RX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
u32 XV_Rx_Hdmi_Initialize(XV_Rx *InstancePtr, u32 HdmiRxSsDevId,
		u32 VPhyDevId, XV_Rx_IntrVecId IntrVecIds)
{
	XV_HdmiRxSs1_Config *XV_HdmiRxSs1_ConfigPtr;
	u32 Status = XST_SUCCESS;

	XV_HdmiRxSs1_ConfigPtr = XV_HdmiRxSs1_LookupConfig(HdmiRxSsDevId);

	if (XV_HdmiRxSs1_ConfigPtr == NULL) {
		HdmiRxSs.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	/* Initialize top level and all included sub-cores */
	Status = XV_HdmiRxSs1_CfgInitialize(InstancePtr->HdmiRxSs,
					XV_HdmiRxSs1_ConfigPtr,
					XV_HdmiRxSs1_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: HDMI RX Subsystem Initialization "
			   "failed %d\r\n", Status);
		return(XST_FAILURE);
	}

	/* Register HDMI RX SS Interrupt Handler with Interrupt Controller */
#if defined(__arm__) || (__aarch64__)
	Status |= XScuGic_Connect(InstancePtr->Intc,
			/* XPAR_FABRIC_V_HDMIRXSS1_0_VEC_ID, */
			IntrVecIds.IntrVecId_HdmiRxSs,
			(XInterruptHandler)XV_HdmiRxSS1_HdmiRxIntrHandler,
			(void *)InstancePtr->HdmiRxSs);


#ifdef XPAR_XHDCP_NUM_INSTANCES
	/* HDCP 1.4 Cipher interrupt */
	Status |= XScuGic_Connect(InstancePtr->Intc,
			/* XPAR_FABRIC_V_HDMIRXSS1_0_HDCP14_IRQ_VEC_ID, */
			IntrVecIds.IntrVecId_Hdcp14,
			(XInterruptHandler)XV_HdmiRxSS1_HdcpIntrHandler,
			(void *)InstancePtr->HdmiRxSs);

	Status |= XScuGic_Connect(InstancePtr->Intc,
			/* XPAR_FABRIC_V_HDMIRXSS1_0_HDCP14_TIMER_IRQ_VEC_ID, */
			IntrVecIds.IntrVecId_Hdcp14Timer,
			(XInterruptHandler)XV_HdmiRxSS1_HdcpTimerIntrHandler,
			(void *)InstancePtr->HdmiRxSs);
#endif

#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
	/* HDCP 2.2 Timer interrupt */
	Status |= XScuGic_Connect(InstancePtr->Intc,
			/* XPAR_FABRIC_V_HDMIRXSS1_0_HDCP22_TIMER_IRQ_VEC_ID, */
			IntrVecIds.IntrVecId_Hdcp22Timer,
			(XInterruptHandler)XV_HdmiRxSS1_Hdcp22TimerIntrHandler,
			(void *)InstancePtr->HdmiRxSs);
#endif

#else
	Status |= XIntc_Connect(InstancePtr->Intc,
			IntrVecIds.IntrVecId_HdmiRxSs,
			(XInterruptHandler)XV_HdmiRxSS1_HdmiRxIntrHandler,
			(void *)InstancePtr->HdmiRxSs);

#ifdef XPAR_XHDCP_NUM_INSTANCES
	/* HDCP 1.4 Cipher interrupt */
	Status |= XIntc_Connect(InstancePtr->Intc,
				/* XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_IRQ_VEC_ID, */
				IntrVecIds.IntrVecId_Hdcp14,
				(XInterruptHandler)XV_HdmiRxSS1_HdcpIntrHandler,
				(void *)InstancePtr->HdmiRxSs);

	/* HDCP 1.4 Timer interrupt */
	Status |= XIntc_Connect(InstancePtr->Intc,
			/* XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_TIMER_IRQ_VEC_ID, */
			IntrVecIds.IntrVecId_Hdcp14Timer,
			(XInterruptHandler)XV_HdmiRxSS1_HdcpTimerIntrHandler,
			(void *)InstancePtr->HdmiRxSs);
#endif

#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
	/* HDCP 2.2 Timer interrupt */
	Status |= XIntc_Connect(InstancePtr->Intc,
			/* XPAR_INTC_0_V_HDMIRXSS_0_HDCP22_TIMER_IRQ_VEC_ID, */
			IntrVecIds.IntrVecId_Hdcp22Timer,
			(XInterruptHandler)XV_HdmiRxSS1_Hdcp22TimerIntrHandler,
			(void *)InstancePtr->HdmiRxSs);
#endif

#endif

	if (Status == XST_SUCCESS) {
#if defined(__arm__) || (__aarch64__)
		XScuGic_Enable(InstancePtr->Intc,
				IntrVecIds.IntrVecId_HdmiRxSs);

#ifdef XPAR_XHDCP_NUM_INSTANCES
		XScuGic_Enable(InstancePtr->Intc,
			/* XPAR_FABRIC_V_HDMIRXSS1_0_HDCP14_IRQ_VEC_ID */
			IntrVecIds.IntrVecId_Hdcp14);
		XScuGic_Enable(InstancePtr->Intc,
			/* XPAR_FABRIC_V_HDMIRXSS1_0_HDCP14_TIMER_IRQ_VEC_ID */
			IntrVecIds.IntrVecId_Hdcp14Timer);
#endif
#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
		XScuGic_Enable(InstancePtr->Intc,
			/* XPAR_FABRIC_V_HDMIRXSS1_0_HDCP22_TIMER_IRQ_VEC_ID */
			IntrVecIds.IntrVecId_Hdcp22Timer);
#endif

#else
		XIntc_Enable(InstancePtr->Intc,
			IntrVecIds.IntrVecId_HdmiRxSs);

#ifdef XPAR_XHDCP_NUM_INSTANCES
		/* HDCP 1.4 Cipher interrupt */
		XIntc_Enable(InstancePtr->Intc,
			/* XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_IRQ_VEC_ID */
			IntrVecIds.IntrVecId_Hdcp14);

		/* HDCP 1.4 Timer interrupt */
		XIntc_Enable(InstancePtr->Intc,
			/*XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_TIMER_IRQ_VEC_ID */
			IntrVecIds.IntrVecId_Hdcp14Timer);
#endif

#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
		/* HDCP 2.2 Timer interrupt */
		XIntc_Enable(InstancePtr->Intc,
			/* XPAR_INTC_0_V_HDMIRXSS_0_HDCP22_TIMER_IRQ_VEC_ID */
			IntrVecIds.IntrVecId_Hdcp22Timer);
#endif

#endif
	} else {
		xil_printf("ERR:: Unable to register HDMI RX interrupt handler");
		xil_printf("HDMI RX SS initialization error\r\n");
		return XST_FAILURE;
	}

	/* Initialize the RX SS interrupts. */
	XV_Rx_HdmiRxSs_Setup_Callbacks(InstancePtr);


	/* Initialize the VPhy related to the HdmiRxSs if it isn't ready yet. */
	if (!(InstancePtr->VidPhy->IsReady == 0xFFFFFFFF)) {
		xil_printf("Initializing Video Phy with Video Receiver \r\n");
		XHdmiphy1_Config *XHdmiphy1CfgPtr;

		/*
		 *  Initialize Video PHY
		 *  The GT needs to be initialized after the HDMI RX.
		 *  The reason for this is the GtRxInitStartCallback
		 *  calls the RX stream down callback.
		 *
	     */
		XHdmiphy1CfgPtr = XHdmiphy1_LookupConfig(VPhyDevId);
		if (XHdmiphy1CfgPtr == NULL) {
			xil_printf("Video PHY device not found\r\n\r\n");
			return XST_FAILURE;
		}

		/* Register VPHY Interrupt Handler */
#if defined(__arm__) || (__aarch64__)
		Status = XScuGic_Connect(InstancePtr->Intc,
					/* XPAR_FABRIC_VID_PHY_CONTROLLER_IRQ_INTR, */
#else
		Status = XIntc_Connect(InstancePtr->Intc,
					/* XPAR_INTC_0_VPHY_0_VEC_ID, */
#endif
					IntrVecIds.IntrVecId_VPhy,
					(XInterruptHandler)XHdmiphy1_InterruptHandler,
					(void *)InstancePtr->VidPhy);

		if (Status != XST_SUCCESS) {
			xil_printf("HDMI VPHY Interrupt Vec ID not found!\r\n");
			return XST_FAILURE;
		}

		/* Initialize HDMI VPHY */
		Status = XHdmiphy1_Hdmi_CfgInitialize(&Hdmiphy1, 0,
						      XHdmiphy1CfgPtr);

		if (Status != XST_SUCCESS) {
			xil_printf("HDMI VPHY initialization error\r\n");
			return XST_FAILURE;
		}

		/* Enable VPHY Interrupt */
#if defined(__arm__) || (__aarch64__)
		XScuGic_Enable(InstancePtr->Intc,
				/* XPAR_FABRIC_VID_PHY_CONTROLLER_IRQ_INTR */
				IntrVecIds.IntrVecId_VPhy);
#else
		XIntc_Enable(InstancePtr->Intc,
				/* XPAR_INTC_0_VPHY_0_VEC_ID */
				IntrVecIds.IntrVecId_VPhy);
#endif
	} else {
		xil_printf("Not Initializing Video Phy with Video "
				"Transmitter as it is already ready.\r\n");
	}

#ifdef USE_HDCP_HDMI_RX
	xil_printf("Initializing Hdcp for Video Receiver \r\n");
	/* Set the HDCP configurations to 0. */
	InstancePtr->HdcpConfig.UpstreamInstanceBinded = 0;
	InstancePtr->HdcpConfig.UpstreamInstanceConnected = FALSE;
	InstancePtr->HdcpConfig.UpstreamInstanceStreamUp = FALSE;
	InstancePtr->HdcpConfig.IsRepeater = FALSE;
	InstancePtr->HdcpConfig.StreamType = XV_HDMIRXSS1_HDCP_STREAMTYPE_0;

	/* Initialize the HDCP related callbacks. */
	Status = XV_Rx_Hdcp_SetCallbacks(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: Unable to set HDCP callbacks !!\r\n");
		return XST_FAILURE;
	}

	/* Increment the number of associated downstream instances. */
	InstancePtr->HdcpConfig.UpstreamInstanceBinded++;
	InstancePtr->HdcpConfig.IsReady = TRUE;

	/* Disable Repeater. */
	/* XV_HdmiRxSs1_HdcpSetRepeater(InstancePtr->HdmiRxSs, FALSE); */
#endif

	/* Initialize the VPhy interrupts. */
	XV_Rx_VPhy_SetCallbacks(InstancePtr);

	/* Set the default configurations for Video Receiver for HDMI. */
	InstancePtr->StreamInitFail = FALSE;

	InstancePtr->StateInfo.CurrentState = XV_RX_HDMI_STATE_DISCONNECTED;
	InstancePtr->StateInfo.PreviousState = XV_RX_HDMI_STATE_DISCONNECTED;
	InstancePtr->StateInfo.HdmiRxPendingEvents = 0x0;

	return Status;
}

#ifdef USE_HDCP_HDMI_RX
/*****************************************************************************/
/**
* This function implements the callback for HDCP stream manage request event.
*
* @param    InstancePtr is the callback reference to the HDMI RX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Rx_Hdcp_StreamManageRequest_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	/* Update the stream type in HDCP configurations */
	recvInst->HdcpConfig.StreamType =
    		XV_HdmiRxSs1_HdcpGetContentStreamType(recvInst->HdmiRxSs);

	xdbg_xv_rx_print(ANSI_COLOR_HIGH_WHITE" "ANSI_COLOR_BG_BLUE"%s, %d "
    			"Steam-Manage-Req (Stream = %d)"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__, recvInst->HdcpConfig.StreamType);

	/* Callback to propagate the content stream to the determine
	 * content stream for the downstream topology.
	 * To be handled by the user. */
	if (recvInst->HdcpSetContentStreamTypeCb != NULL) {
		recvInst->HdcpSetContentStreamTypeCb(
			recvInst->HdcpSetContentStreamTypeCbRef);
	}
}

/*****************************************************************************/
/**
* This function implements the callback for HDCP authentication event.
*
* @param    InstancePtr is the callback reference to the HDMI RX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Rx_Hdcp_Authenticated_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	int HdcpProtocol;
	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	xdbg_xv_rx_print(ANSI_COLOR_HIGH_WHITE" "ANSI_COLOR_BG_BLUE"%s, %d "
			"Hdcp-Authenticated"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__);

	/* xil_printf message */
	HdcpProtocol = XV_HdmiRxSs1_HdcpGetProtocol(recvInst->HdmiRxSs);
	switch (HdcpProtocol) {
	case XV_HDMIRXSS1_HDCP_22:
		xdbg_printf(XDBG_DEBUG_GENERAL,"HDCP 2.2 upstream "
				"authenticated\r\n");
		break;

	case XV_HDMIRXSS1_HDCP_14:
		xdbg_printf(XDBG_DEBUG_GENERAL,"HDCP 1.4 upstream "
				"authenticated\r\n");
		break;

	default:
		break;
	}

	recvInst->HdcpConfig.StreamType =
		XV_HdmiRxSs1_HdcpGetContentStreamType(recvInst->HdmiRxSs);
	if (recvInst->HdcpForceBlankCb != NULL) {
		recvInst->HdcpForceBlankCb(recvInst->HdcpForceBlankCbRef);
	}
}

/*****************************************************************************/
/**
* This function implements the callback for HDCP un-authentication event.
*
* @param    InstancePtr is the callback reference to the HDMI RX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Rx_Hdcp_Unauthenticated_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	xdbg_xv_rx_print(ANSI_COLOR_HIGH_WHITE" "ANSI_COLOR_BG_BLUE"%s, %d "
			"Hdcp-Unauthenticated"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__);

	/* Reset the stream type to type 0 stream. */
	recvInst->HdcpConfig.StreamType = XV_HDMIRXSS1_HDCP_STREAMTYPE_0;

	if (recvInst->HdcpForceBlankCb != NULL) {
		recvInst->HdcpForceBlankCb(recvInst->HdcpForceBlankCbRef);
	}
}

/*****************************************************************************/
/**
* This function implements the callback for HDCP encyption update event.
*
* @param    InstancePtr is the callback reference to the HDMI RX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Rx_Hdcp_EncryptionUpdate_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	xdbg_xv_rx_print(ANSI_COLOR_HIGH_WHITE" "ANSI_COLOR_BG_BLUE"%s, %d "
			"Hdcp-Encryption-Update"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__);

	recvInst->HdcpConfig.StreamType =
		XV_HdmiRxSs1_HdcpGetContentStreamType(recvInst->HdmiRxSs);
	if (recvInst->HdcpForceBlankCb != NULL) {
		recvInst->HdcpForceBlankCb(recvInst->HdcpForceBlankCbRef);
	}
}
#endif

/*****************************************************************************/
/**
* This function implements the callback for the HDMI RX Connect event.
*
* @param    InstancePtr is the callback reference to the HDMI RX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Rx_HdmiRx_Connect_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)recvInst->HdmiRxSs;

	if (HdmiRxSs1Ptr->IsStreamConnected == (FALSE)) {
		xdbg_xv_rx_print(ANSI_COLOR_WHITE" "ANSI_COLOR_BG_HIGH_CYAN
			"%s, %d Disconnected"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__);

		recvInst->ErrorStats.RxBrdgOverflowCnt = 0;

		XV_Rx_HdmiRx_SendEvent(recvInst, XV_RX_HDMI_EVENT_DISCONNECTED);
		/* Alternatively, if a priority queue can be established :-
		 * XV_Rx_HdmiRx_PushEvent(recvInst, XV_RX_HDMI_EVENT_DISCONNECTED);
		 */
	} else if (HdmiRxSs1Ptr->IsStreamConnected == (TRUE)) {
		xdbg_xv_rx_print(ANSI_COLOR_WHITE" "ANSI_COLOR_BG_HIGH_CYAN
			"%s, %d Connected"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__);

		XV_Rx_HdmiRx_SendEvent(recvInst, XV_RX_HDMI_EVENT_CONNECTED);
		/* Alternatively, if a priority queue can be established :-
		 * XV_Rx_HdmiRx_PushEvent(recvInst, XV_RX_HDMI_EVENT_CONNECTED);
		 */
	}

}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI RX Bridge overflow event.
*
* @param    InstancePtr is the callback reference to the HDMI RX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Rx_HdmiRx_BrdgOverFlow_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	/* xdbg_xv_rx_print(ANSI_COLOR_WHITE" "ANSI_COLOR_BG_HIGH_CYAN
	 *		"%s, %d Bridge-Overflow"ANSI_COLOR_RESET"\r\n",
	 *		__func__, __LINE__);
	 */

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	/* Update error cont */
	recvInst->ErrorStats.RxBrdgOverflowCnt++;
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI RX Aux event.
*
* @param    InstancePtr is the callback reference to the HDMI RX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Rx_HdmiRx_Aux_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	/* xdbg_xv_rx_print(ANSI_COLOR_WHITE" "ANSI_COLOR_BG_HIGH_CYAN
	 *		"%s, %d RX AUX Event"ANSI_COLOR_RESET"\r\n",
	 *		__func__, __LINE__); */

	if (recvInst->RxAuxCb != NULL) {
		recvInst->RxAuxCb(recvInst->RxAuxCbRef);
	}
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI RX Audio event.
*
* @param    InstancePtr is the callback reference to the HDMI RX SS instance.
*
* @return	None.
*l
* @note		None.
*
******************************************************************************/
static void XV_Rx_HdmiRx_Audio_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	xdbg_xv_rx_print(ANSI_COLOR_WHITE" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
			"RX Audio Event"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__);

	if (recvInst->RxAudio != NULL) {
		recvInst->RxAudio(recvInst->RxAudioCallbackRef);
	}
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI RX Link Status event.
*
* @param    InstancePtr is the callback reference to the HDMI RX SS instance.
* @param    CallbackRef is the value passed from the event based callback.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Rx_HdmiRx_LinkStatus_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	xdbg_xv_rx_print(ANSI_COLOR_WHITE" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
			"Link Status"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	if (recvInst->StateInfo.CurrentState != XV_RX_HDMI_STATE_DISCONNECTED) {
		XV_Rx_PulsePllReset(recvInst);
	}
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI RX DDC event.
*
* @param    InstancePtr is the callback reference to the HDMI RX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Rx_HdmiRx_Ddc_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	xdbg_xv_rx_print(ANSI_COLOR_WHITE" "ANSI_COLOR_BG_HIGH_CYAN
			"%s, %d Rx Ddc Event"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__);

	/* Doing nothing here */
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI RX Stream Down event.
*
* @param    InstancePtr is the callback reference to the HDMI RX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Rx_HdmiRx_StreamDown_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	xdbg_xv_rx_print(ANSI_COLOR_WHITE" "ANSI_COLOR_BG_HIGH_CYAN
			"%s, %d Rx Stream Down"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__);

	XV_Rx_HdmiRx_SendEvent(recvInst, XV_RX_HDMI_EVENT_STREAMDOWN);
	/* Alternatively, if a priority queue can be established :-
	 * XV_Rx_HdmiRx_PushEvent(recvInst, XV_RX_HDMI_EVENT_STREAMDOWN);
	 */

}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI RX Stream
* Initialization event.
*
* @param    InstancePtr is the callback reference to the HDMI RX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Rx_HdmiRx_StreamInit_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	xdbg_xv_rx_print(ANSI_COLOR_WHITE" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
			"Rx Stream Init"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__);

	XV_Rx_HdmiRx_SendEvent(recvInst, XV_RX_HDMI_EVENT_STREAMINIT);
	/* Alternatively, if a priority queue can be established :-
	 * XV_Rx_HdmiRx_PushEvent(recvInst, XV_RX_HDMI_EVENT_STREAMINIT);
	 */
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI RX Stream Up event.
*
* @param    InstancePtr is the callback reference to the HDMI RX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Rx_HdmiRx_StreamUp_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	xdbg_xv_rx_print(ANSI_COLOR_WHITE" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
		"Rx Stream Up, State: %s"ANSI_COLOR_RESET"\r\n",
		__func__, __LINE__,
		XV_Rx_Hdmi_Rx_StatetoString(recvInst->StateInfo.CurrentState));

	XV_Rx_HdmiRx_SendEvent(recvInst, XV_RX_HDMI_EVENT_STREAMUP);
	/* Alternatively, if a priority queue can be established :-
	 * XV_Rx_HdmiRx_PushEvent(recvInst, XV_RX_HDMI_EVENT_STREAMUP);
	 */
}

/*****************************************************************************/
/**
* This function implements the callback for the HDMI RX Phy Reset event.
*
* @param    InstancePtr is the callback reference to the HDMI RX SS instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_Rx_HdmiRx_PhyReset_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	xdbg_xv_rx_print(ANSI_COLOR_WHITE" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
		"Rx Phy Reset, State: %s"ANSI_COLOR_RESET"\r\n",
		__func__, __LINE__,
		XV_Rx_Hdmi_Rx_StatetoString(recvInst->StateInfo.CurrentState));

	XV_Rx_HdmiRx_SendEvent(recvInst, XV_RX_HDMI_EVENT_PHYRESET);
}

static void XV_Rx_HdmiRx_FrlConfig_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	xdbg_xv_rx_print(ANSI_COLOR_WHITE" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
			"Rx Frl Config"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__);

	XV_Rx_HdmiRx_SendEvent(recvInst, XV_RX_HDMI_EVENT_FRLCONFIG);
	/* Alternatively, if a priority queue can be established :-
	 * XV_Rx_HdmiRx_PushEvent(recvInst, XV_RX_HDMI_EVENT_STREAMUP);
	 */
}

static void XV_Rx_HdmiRx_FrlStart_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	xdbg_xv_rx_print(ANSI_COLOR_WHITE" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
			"Rx Frl Start"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__);

	XV_Rx_HdmiRx_SendEvent(recvInst, XV_RX_HDMI_EVENT_FRLSTART);
	/* Alternatively, if a priority queue can be established :-
	 * XV_Rx_HdmiRx_PushEvent(recvInst, XV_RX_HDMI_EVENT_STREAMUP);
	 */
}

static void XV_Rx_HdmiRx_TmdsConfig_Cb(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	xdbg_xv_rx_print(ANSI_COLOR_WHITE" "ANSI_COLOR_BG_HIGH_CYAN"%s, %d "
			"Rx Tmds Config"ANSI_COLOR_RESET"\r\n",
			__func__, __LINE__);

	XV_Rx_HdmiRx_SendEvent(recvInst, XV_RX_HDMI_EVENT_TMDSCONFIG);
	/* Alternatively, if a priority queue can be established :-
	 * XV_Rx_HdmiRx_PushEvent(recvInst, XV_RX_HDMI_EVENT_STREAMUP);
	 */
}

static void XV_Rx_HdmiRX_VrrVfp_Cb (void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	if (recvInst->RxVrrVfpCb != NULL) {
		recvInst->RxVrrVfpCb(recvInst->RxVrrVfpCbRef);
	}

}

static void XV_Rx_HdmiRX_VtemPkt_Cb (void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	if (recvInst->RxVtemCb != NULL) {
		recvInst->RxVtemCb(recvInst->RxVtemCbRef);
	}


}



static void XV_Rx_HdmiRX_DynHdrPkt_Cb (void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef);

	XV_Rx *recvInst = (XV_Rx *)CallbackRef;

	if (recvInst->RxDynHdrCb != NULL) {
		recvInst->RxDynHdrCb(recvInst->RxDynHdrCbRef);
	}


}


/******************* XV RX STATE MACHINE IMPLEMENTATION **********************/

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
static void XV_Rx_HdmiRx_ProcessPendingEvents(XV_Rx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	XV_Rx_Hdmi_Events EventPending;

	for (int i = 0 ; i < XV_RX_HDMI_NUM_EVENTS ; i++) {
		if ((InstancePtr->StateInfo.HdmiRxPendingEvents &&
		     ((u32)0x1 << (u32)i)) != 0) {
			EventPending = XHdmi_Rx_EventsPriorityQueue[i];
			/* Process the pending event. */
			XV_Rx_HdmiRx_ProcessEvents(InstancePtr, EventPending);
			/* Clear the pending event. */
			InstancePtr->StateInfo.HdmiRxPendingEvents &= (u16)(~(0x1 << i));
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
void XV_Rx_HdmiRx_PushEvent(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event)
{
	Xil_AssertVoid(InstancePtr);
	Xil_AssertVoid(Event < XV_RX_HDMI_NUM_EVENTS);

	for (int i = 0 ; i < XV_RX_HDMI_NUM_EVENTS ; i++) {
		if (Event == XHdmi_Rx_EventsPriorityQueue[i]) {
			InstancePtr->StateInfo.HdmiRxPendingEvents |= (0x1 << i);
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
static void XV_Rx_HdmiRx_SendEvent(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event)
{
	Xil_AssertVoid(InstancePtr);
	Xil_AssertVoid(Event < XV_RX_HDMI_NUM_EVENTS);

	XV_Rx_HdmiRx_ProcessEvents(InstancePtr, Event);
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
static void XV_Rx_HdmiRx_ProcessEvents(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event)
{
	Xil_AssertVoid(InstancePtr);

	XV_Rx_Hdmi_State CurrentState =
			InstancePtr->StateInfo.CurrentState;
	XV_Rx_Hdmi_State NextState = CurrentState;

	switch (CurrentState) {
	case XV_RX_HDMI_STATE_DISCONNECTED:
		XV_Rx_HdmiRx_StateDisconnected(InstancePtr, Event, &NextState);
		break;

	case XV_RX_HDMI_STATE_CONNECTED:
		XV_Rx_HdmiRx_StateConnected(InstancePtr, Event, &NextState);
		break;

	case XV_RX_HDMI_STATE_NOSTREAM:
		XV_Rx_HdmiRx_StateNoStream(InstancePtr, Event, &NextState);
		break;

	case XV_RX_HDMI_STATE_STREAMINITIALIZED:
		XV_Rx_HdmiRx_StateStreamInitialized(InstancePtr, Event, &NextState);
		break;

	case XV_RX_HDMI_STATE_STREAMON:
		XV_Rx_HdmiRx_StateStreamOn(InstancePtr, Event, &NextState);
		break;

	case XV_RX_HDMI_STATE_STREAMOFF:
		XV_Rx_HdmiRx_StateStreamOff(InstancePtr, Event, &NextState);
		break;

	case XV_RX_HDMI_STATE_PHYRESET:
		XV_Rx_HdmiRx_StatePhyReset(InstancePtr, Event, &NextState);
		break;

	case XV_RX_HDMI_STATE_FRLCONFIG:
		XV_Rx_HdmiRx_StateFrlConfig(InstancePtr, Event, &NextState);
		break;

	case XV_RX_HDMI_STATE_FRLSTART:
		XV_Rx_HdmiRx_StateFrlStart(InstancePtr, Event, &NextState);
		break;

	case XV_RX_HDMI_STATE_TMDSCONFIG:
		XV_Rx_HdmiRx_StateTmdsConfig(InstancePtr, Event, &NextState);
		break;

	default:
		break;
	}

	if ( (NextState != CurrentState) | (NextState == XV_RX_HDMI_STATE_STREAMON)) {
		xdbg_xv_rx_statemachine_print("%s,%d: Event %s : < "
				"State %s -> State %s >\r\n", __func__,
				__LINE__, XV_Rx_Hdmi_Rx_EventtoString(Event),
				XV_Rx_Hdmi_Rx_StatetoString(
					InstancePtr->StateInfo.CurrentState),
				XV_Rx_Hdmi_Rx_StatetoString(NextState));

		InstancePtr->StateInfo.PreviousState = CurrentState;
		XV_Rx_HdmiRx_StateEnter(InstancePtr, NextState);
		InstancePtr->StateInfo.CurrentState = NextState;

		if ((InstancePtr->StateInfo.CurrentState ==
		     XV_RX_HDMI_STATE_STREAMINITIALIZED) &&
		    InstancePtr->StreamInitFail == TRUE) {
			InstancePtr->StateInfo.CurrentState =
						XV_RX_HDMI_STATE_STREAMOFF;
		}
	}

}

static void XV_Rx_HdmiRx_StateEnter(XV_Rx *InstancePtr, XV_Rx_Hdmi_State State)
{
	Xil_AssertVoid(InstancePtr);

	switch (State) {
	case XV_RX_HDMI_STATE_DISCONNECTED:
		XV_Rx_HdmiRx_EnterStateDisconnected(InstancePtr);
		break;

	case XV_RX_HDMI_STATE_CONNECTED:
		XV_Rx_HdmiRx_EnterStateConnected(InstancePtr);
		break;

	case XV_RX_HDMI_STATE_NOSTREAM:
		XV_Rx_HdmiRx_EnterStateNoStream(InstancePtr);
		break;

	case XV_RX_HDMI_STATE_STREAMINITIALIZED:
		XV_Rx_HdmiRx_EnterStateStreamInitialized(InstancePtr);
		break;

	case XV_RX_HDMI_STATE_STREAMON:
		XV_Rx_HdmiRx_EnterStateStreamOn(InstancePtr);
		break;

	case XV_RX_HDMI_STATE_STREAMOFF:
		XV_Rx_HdmiRx_EnterStateStreamOff(InstancePtr);
		break;

	case XV_RX_HDMI_STATE_PHYRESET:
		XV_Rx_HdmiRx_EnterStatePhyReset(InstancePtr);
		break;

	case XV_RX_HDMI_STATE_FRLCONFIG:
		XV_Rx_HdmiRx_EnterStateFrlConfig(InstancePtr);
		break;

	case XV_RX_HDMI_STATE_FRLSTART:
		XV_Rx_HdmiRx_EnterStateFrlStart(InstancePtr);
		break;

	case XV_RX_HDMI_STATE_TMDSCONFIG:
		XV_Rx_HdmiRx_EnterStateTmdsConfig(InstancePtr);
		break;

	default:
		break;
	}

}

static void XV_Rx_HdmiRx_StateDisconnected(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	/* Transition from state disconnected to connected, is only
	 * allowed when a connected event is received.
	 *
	 * All other events - stream init, stream up and
	 * stream down are ignored, if the hdmi rx is in
	 * the disconnected state.
	 */
	switch (Event) {
	case XV_RX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_RX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_RX_HDMI_EVENT_CONNECTED:
		*NextStatePtr = XV_RX_HDMI_STATE_CONNECTED;
		break;

	case XV_RX_HDMI_EVENT_STREAMINIT:
		break;

	case XV_RX_HDMI_EVENT_STREAMUP:
		break;

	case XV_RX_HDMI_EVENT_STREAMDOWN:
		break;

	case XV_RX_HDMI_EVENT_PHYRESET:
		break;

	case XV_RX_HDMI_EVENT_FRLCONFIG:
		break;

	case XV_RX_HDMI_EVENT_FRLSTART:
		break;

	case XV_RX_HDMI_EVENT_TMDSCONFIG:

	default:
		break;
	}
}

static void XV_Rx_HdmiRx_EnterStateDisconnected(XV_Rx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	XHdmiphy1 *XV_Rx_Hdmiphy1Ptr = InstancePtr->VidPhy;

	/* Clear the GT RX TMDS clock ratio. */
	XV_Rx_Hdmiphy1Ptr->HdmiRxTmdsClockRatio = 0;

	XHdmiphy1_IBufDsEnable(XV_Rx_Hdmiphy1Ptr, 0, XHDMIPHY1_DIR_RX, (FALSE));

	if (InstancePtr->RxCableConnectionChange != NULL) {
		InstancePtr->RxCableConnectionChange(
			InstancePtr->RxCableConnectionChangeCallbackRef);
	}

#ifdef USE_HDCP_HDMI_RX
	InstancePtr->HdcpConfig.UpstreamInstanceConnected = FALSE;
	InstancePtr->HdcpConfig.UpstreamInstanceStreamUp = FALSE;
#endif
}

static void XV_Rx_HdmiRx_StateConnected(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_RX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_RX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_RX_HDMI_EVENT_CONNECTED:

		break;

	case XV_RX_HDMI_EVENT_STREAMINIT:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMINITIALIZED;
		break;

	case XV_RX_HDMI_EVENT_STREAMUP:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMON;
		break;

	case XV_RX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMOFF;
		break;

	case XV_RX_HDMI_EVENT_PHYRESET:
		*NextStatePtr = XV_RX_HDMI_STATE_PHYRESET;
		break;

	case XV_RX_HDMI_EVENT_FRLCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_RX_HDMI_EVENT_FRLSTART:
		*NextStatePtr = XV_RX_HDMI_STATE_FRLSTART;
		break;

	case XV_RX_HDMI_EVENT_TMDSCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_TMDSCONFIG;
		break;

	default:
		break;
	}
}

static void XV_Rx_HdmiRx_EnterStateConnected(XV_Rx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	XHdmiphy1 *XV_Rx_Hdmiphy1Ptr = InstancePtr->VidPhy;

	XHdmiphy1_IBufDsEnable(XV_Rx_Hdmiphy1Ptr, 0, XHDMIPHY1_DIR_RX, (TRUE));

	if (InstancePtr->RxCableConnectionChange != NULL) {
		InstancePtr->RxCableConnectionChange(
			InstancePtr->RxCableConnectionChangeCallbackRef);
	}

#ifdef USE_HDCP_HDMI_RX
	InstancePtr->HdcpConfig.UpstreamInstanceConnected = TRUE;
#endif
}

static void XV_Rx_HdmiRx_StateNoStream(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_RX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_RX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_RX_HDMI_EVENT_CONNECTED:

		break;

	case XV_RX_HDMI_EVENT_STREAMINIT:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMINITIALIZED;
		break;

	case XV_RX_HDMI_EVENT_STREAMUP:

		break;

	case XV_RX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMOFF;
		break;

	case XV_RX_HDMI_EVENT_PHYRESET:
		*NextStatePtr = XV_RX_HDMI_STATE_PHYRESET;
		break;

	case XV_RX_HDMI_EVENT_FRLCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_RX_HDMI_EVENT_FRLSTART:
		*NextStatePtr = XV_RX_HDMI_STATE_FRLSTART;
		break;

	case XV_RX_HDMI_EVENT_TMDSCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_TMDSCONFIG;
		break;

	default:
		break;
	}
}

static void XV_Rx_HdmiRx_EnterStateNoStream(XV_Rx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	/* Doing nothing here. */
}

static void XV_Rx_HdmiRx_StateStreamInitialized(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_RX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_RX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_RX_HDMI_EVENT_CONNECTED:

		break;

	case XV_RX_HDMI_EVENT_STREAMINIT:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMINITIALIZED;
		break;

	case XV_RX_HDMI_EVENT_STREAMUP:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMON;
		break;

	case XV_RX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMOFF;
		break;

	case XV_RX_HDMI_EVENT_PHYRESET:
		*NextStatePtr = XV_RX_HDMI_STATE_PHYRESET;
		break;

	case XV_RX_HDMI_EVENT_FRLCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_RX_HDMI_EVENT_FRLSTART:
		*NextStatePtr = XV_RX_HDMI_STATE_FRLSTART;
		break;

	case XV_RX_HDMI_EVENT_TMDSCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_TMDSCONFIG;
		break;

	default:
		break;
	}
}

static void XV_Rx_HdmiRx_EnterStateStreamInitialized(XV_Rx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	u32 Status = XST_SUCCESS;

	/* HDCP operations here if necessary. */
	Status = XV_Rx_HdmiRx_StreamInit_CfgMmcm(InstancePtr);
	if (Status != XST_SUCCESS) {
		xdbg_xv_rx_print("%s, %d : mmcm configuration in STREAM "
				"INITIALIZATION failed.\r\n", __func__, __LINE__);
		InstancePtr->StreamInitFail = TRUE;
	} else {
		InstancePtr->StreamInitFail = FALSE;
	}
}


static void XV_Rx_HdmiRx_StateStreamOn(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_RX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_RX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_RX_HDMI_EVENT_CONNECTED:

		break;

	case XV_RX_HDMI_EVENT_STREAMINIT:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMINITIALIZED;
		break;

	case XV_RX_HDMI_EVENT_STREAMUP:
        *NextStatePtr = XV_RX_HDMI_STATE_STREAMON;
		break;

	case XV_RX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMOFF;
		break;

	case XV_RX_HDMI_EVENT_PHYRESET:
		*NextStatePtr = XV_RX_HDMI_STATE_PHYRESET;
		break;

	case XV_RX_HDMI_EVENT_FRLCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_RX_HDMI_EVENT_FRLSTART:
		break;

	case XV_RX_HDMI_EVENT_TMDSCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_TMDSCONFIG;
		break;

	default:
		break;
	}
}

static void XV_Rx_HdmiRx_EnterStateStreamOn(XV_Rx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);
	if (InstancePtr->HdmiRxSs->HdmiRx1Ptr->Stream.IsFrl != TRUE) {
		/* Enable RX clock forwarding */
		XHdmiphy1_Clkout1OBufTdsEnable(InstancePtr->VidPhy,
					       XHDMIPHY1_DIR_RX, (TRUE));
	}

	if (InstancePtr->RxStreamOn != NULL) {
		InstancePtr->RxStreamOn(InstancePtr->RxStreamOnCallbackRef);
	}
#ifdef USE_HDCP_HDMI_RX
	/* Hdcp Stream Up. */
	InstancePtr->HdcpConfig.UpstreamInstanceStreamUp = TRUE;
#endif
}

static void XV_Rx_HdmiRx_StateStreamOff(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_RX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_RX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_RX_HDMI_EVENT_CONNECTED:

		break;

	case XV_RX_HDMI_EVENT_STREAMINIT:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMINITIALIZED;
		break;

	case XV_RX_HDMI_EVENT_STREAMUP:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMON;
		break;

	case XV_RX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMOFF;
		break;

	case XV_RX_HDMI_EVENT_PHYRESET:
		*NextStatePtr = XV_RX_HDMI_STATE_PHYRESET;
		break;

	case XV_RX_HDMI_EVENT_FRLCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_RX_HDMI_EVENT_FRLSTART:
		break;

	case XV_RX_HDMI_EVENT_TMDSCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_TMDSCONFIG;
		break;

	default:
		break;
	}
}

static void XV_Rx_HdmiRx_EnterStateStreamOff(XV_Rx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	/* Here for robustness we should get ready
	 * to receive the stream again.
	 */

	if (InstancePtr->HdmiRxSs->HdmiRx1Ptr->Stream.IsFrl != TRUE) {
		/* Power down the MMCM. (without holding) */
		XHdmiphy1_MmcmPowerDown(InstancePtr->VidPhy, 0,
					XHDMIPHY1_DIR_RX, FALSE);

		/* Disable RX clock forwarding */
		XHdmiphy1_Clkout1OBufTdsEnable(InstancePtr->VidPhy,
					       XHDMIPHY1_DIR_RX, (FALSE));
	}

	InstancePtr->ErrorStats.RxBrdgOverflowCnt = 0;

	/* Stream Down */
	if (InstancePtr->RxStreamOff != NULL) {
		InstancePtr->RxStreamOff(InstancePtr->RxStreamOffCallbackRef);
	}

#ifdef USE_HDCP_HDMI_RX
	/* Hdcp Stream Down */
	InstancePtr->HdcpConfig.UpstreamInstanceStreamUp = FALSE;
#endif
}

static void XV_Rx_HdmiRx_StatePhyReset(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_RX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_RX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_RX_HDMI_EVENT_CONNECTED:
		break;

	case XV_RX_HDMI_EVENT_STREAMINIT:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMINITIALIZED;
		break;

	case XV_RX_HDMI_EVENT_STREAMUP:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMON;
		break;

	case XV_RX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMOFF;
		break;

	case XV_RX_HDMI_EVENT_PHYRESET:
		XV_Rx_HdmiRx_EnterStatePhyReset(InstancePtr);
		break;

	case XV_RX_HDMI_EVENT_FRLCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_RX_HDMI_EVENT_FRLSTART:
		*NextStatePtr = XV_RX_HDMI_STATE_FRLSTART;
		break;

	case XV_RX_HDMI_EVENT_TMDSCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_TMDSCONFIG;
		break;

	default:
		break;
	}
}

static void XV_Rx_HdmiRx_EnterStatePhyReset(XV_Rx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	xdbg_xv_rx_print("%s: Hdmi Rx : PhyReset ...\r\n", __func__);

	XHdmiphy1_ResetGtTxRx (InstancePtr->VidPhy,
			0,
			XHDMIPHY1_CHANNEL_ID_CHA,
			XHDMIPHY1_DIR_RX,
			FALSE);

}

static void XV_Rx_HdmiRx_StateFrlConfig(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_RX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_RX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_RX_HDMI_EVENT_CONNECTED:

		break;

	case XV_RX_HDMI_EVENT_STREAMINIT:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMINITIALIZED;
		break;

	case XV_RX_HDMI_EVENT_STREAMUP:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMON;
		break;

	case XV_RX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMOFF;
		break;

	case XV_RX_HDMI_EVENT_PHYRESET:
		*NextStatePtr = XV_RX_HDMI_STATE_PHYRESET;
		break;

	case XV_RX_HDMI_EVENT_FRLCONFIG:
		XV_Rx_HdmiRx_EnterStateFrlConfig(InstancePtr);
		break;

	case XV_RX_HDMI_EVENT_FRLSTART:
		*NextStatePtr = XV_RX_HDMI_STATE_FRLSTART;
		break;

	case XV_RX_HDMI_EVENT_TMDSCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_TMDSCONFIG;
		break;

	default:
		break;
	}
}

static void XV_Rx_HdmiRx_EnterStateFrlConfig(XV_Rx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	xdbg_xv_rx_print("%s: Hdmi Rx : FrlConfig ...\r\n", __func__);
	u64 LineRate = (u64)(HdmiRxSs.HdmiRx1Ptr->Stream.Frl.LineRate) * (u64)(1e9);
	u8 NChannels = HdmiRxSs.HdmiRx1Ptr->Stream.Frl.Lanes;

	InstancePtr->RxClkSrcConfig(InstancePtr->RxClkSrcConfigCallbackRef);

	if (InstancePtr->VidPhy->Config.RxRefClkSel !=
			InstancePtr->VidPhy->Config.RxFrlRefClkSel){
		XHdmiphy1_IBufDsEnable(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX, (TRUE));
	}
#ifdef XPS_BOARD_VCU118
	u8 rate = HdmiRxSs.HdmiRx1Ptr->Stream.Frl.LineRate;

	if (rate != 12) {
		//LPM
		XHdmiphy1_SetRxLpm(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CHA, XHDMIPHY1_DIR_RX, 1);

	} else {
		//DFE
		XHdmiphy1_SetRxLpm(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CHA, XHDMIPHY1_DIR_RX, 0);

	}
#endif
	XHdmiphy1_Hdmi21Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX,
			       LineRate, NChannels);

	if (InstancePtr->RxClkSrcSelCb != NULL) {
		InstancePtr->RxClkSrcSelCb(InstancePtr->RxClkSrcSelCallbackRef);
	}
}

static void XV_Rx_HdmiRx_StateFrlStart(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_RX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_RX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_RX_HDMI_EVENT_CONNECTED:

		break;

	case XV_RX_HDMI_EVENT_STREAMINIT:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMINITIALIZED;
		break;

	case XV_RX_HDMI_EVENT_STREAMUP:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMON;
		break;

	case XV_RX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMOFF;
		break;

	case XV_RX_HDMI_EVENT_PHYRESET:
		*NextStatePtr = XV_RX_HDMI_STATE_PHYRESET;
		break;

	case XV_RX_HDMI_EVENT_FRLCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_RX_HDMI_EVENT_FRLSTART:
		break;

	case XV_RX_HDMI_EVENT_TMDSCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_TMDSCONFIG;
		break;

	default:
		break;
	}
}

static void XV_Rx_HdmiRx_EnterStateFrlStart(XV_Rx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	xdbg_xv_rx_print("%s: Hdmi Rx : Frl Start ...\r\n", __func__);
}

static void XV_Rx_HdmiRx_StateTmdsConfig(XV_Rx *InstancePtr,
					XV_Rx_Hdmi_Events Event,
					XV_Rx_Hdmi_State *NextStatePtr)
{
	Xil_AssertVoid(InstancePtr);

	switch (Event) {
	case XV_RX_HDMI_EVENT_DISCONNECTED:
		*NextStatePtr = XV_RX_HDMI_STATE_DISCONNECTED;
		break;

	case XV_RX_HDMI_EVENT_CONNECTED:

		break;

	case XV_RX_HDMI_EVENT_STREAMINIT:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMINITIALIZED;
		break;

	case XV_RX_HDMI_EVENT_STREAMUP:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMON;
		break;

	case XV_RX_HDMI_EVENT_STREAMDOWN:
		*NextStatePtr = XV_RX_HDMI_STATE_STREAMOFF;
		break;

	case XV_RX_HDMI_EVENT_PHYRESET:
		*NextStatePtr = XV_RX_HDMI_STATE_PHYRESET;
		break;

	case XV_RX_HDMI_EVENT_FRLCONFIG:
		*NextStatePtr = XV_RX_HDMI_STATE_FRLCONFIG;
		break;

	case XV_RX_HDMI_EVENT_FRLSTART:
		*NextStatePtr = XV_RX_HDMI_STATE_FRLSTART;
		break;

	case XV_RX_HDMI_EVENT_TMDSCONFIG:
		XV_Rx_HdmiRx_EnterStateTmdsConfig(InstancePtr);
		break;

	default:
		break;
	}
}

static void XV_Rx_HdmiRx_EnterStateTmdsConfig(XV_Rx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	InstancePtr->RxClkSrcConfig(InstancePtr->RxClkSrcConfigCallbackRef);

	XHdmiphy1_Hdmi20Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX);

	if (InstancePtr->RxClkSrcSelCb != NULL) {
		InstancePtr->RxClkSrcSelCb(InstancePtr->RxClkSrcSelCallbackRef);
	}

	XV_HdmiRx1_SetFrlVClkVckeRatio(HdmiRxSs.HdmiRx1Ptr, 0);
}

const char *XV_Rx_Hdmi_Rx_StatetoString(XV_Rx_Hdmi_State State)
{
	const char *stateNameString;

	switch (State) {
	case XV_RX_HDMI_STATE_DISCONNECTED:
		stateNameString = "dis-connected";
		break;

	case XV_RX_HDMI_STATE_CONNECTED:
		stateNameString = "connected";
		break;

	case XV_RX_HDMI_STATE_NOSTREAM:
		stateNameString = "no-stream";
		break;

	case XV_RX_HDMI_STATE_STREAMINITIALIZED:
		stateNameString = "stream-initialized";
		break;

	case XV_RX_HDMI_STATE_STREAMON:
		stateNameString = "stream-on";
		break;

	case XV_RX_HDMI_STATE_STREAMOFF:
		stateNameString = "stream-off";
		break;

	case XV_RX_HDMI_STATE_FRLCONFIG:
		stateNameString = "frl-config";
		break;

	case XV_RX_HDMI_STATE_FRLSTART:
		stateNameString = "frl-start";
		break;

	case XV_RX_HDMI_STATE_TMDSCONFIG:
		stateNameString = "tmds-config";
		break;

	default:
		stateNameString = "unknown";
		break;
	}

	return stateNameString;
}

const char *XV_Rx_Hdmi_Rx_EventtoString(XV_Rx_Hdmi_Events Event)
{
	const char *eventNameString;

	switch (Event) {
	case XV_RX_HDMI_EVENT_DISCONNECTED:
		eventNameString = "dis-connected";
		break;

	case XV_RX_HDMI_EVENT_CONNECTED:
		eventNameString = "connected";
		break;

	case XV_RX_HDMI_EVENT_STREAMINIT:
		eventNameString = "stream-initialize";
		break;

	case XV_RX_HDMI_EVENT_STREAMUP:
		eventNameString = "stream-up";
		break;

	case XV_RX_HDMI_EVENT_STREAMDOWN:
		eventNameString = "stream-down";
		break;

	case XV_RX_HDMI_EVENT_POLL:
		eventNameString = "poll";
		break;

	case XV_RX_HDMI_EVENT_FRLCONFIG:
		eventNameString = "frl-config";
		break;

	case XV_RX_HDMI_EVENT_FRLSTART:
		eventNameString = "frl-start";
		break;

	case XV_RX_HDMI_EVENT_TMDSCONFIG:
		eventNameString = "tmds-config";
		break;

	default:
		eventNameString = "unknown";
		break;
	}

	return eventNameString;
}
#endif
