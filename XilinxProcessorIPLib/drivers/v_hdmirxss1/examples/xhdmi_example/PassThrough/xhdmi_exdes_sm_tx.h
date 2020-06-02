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
#ifndef _XHDMI_EXDES_SM_TX_H_
#define _XHDMI_EXDES_SM_TX_H_

/***************************** Include Files *********************************/
#include "xparameters.h"
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#include <string.h>
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xv_hdmitxss1.h"
#include "xhdmiphy1.h"
#include "xvidc.h"
#include "xvidc_edid_ext.h"
#if defined (ARMR5) || (__aarch64__) || (__arm__)
#include "xscugic.h"
#else
#include "xintc.h"
#endif
#include "xv_hdmitx1.h"

/************************** Constant Definitions ****************************/

/* Define constants to print colors */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_HIGH_YELLOW  "\x1b[93m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define ANSI_COLOR_BG_MAGENTA   "\x1b[45m"
#define ANSI_COLOR_BG_HIGH_CYAN "\x1b[46;1m"
#define ANSI_COLOR_BG_BLUE      "\x1b[44m"
#define ANSI_COLOR_BG_RED       "\x1b[41m"
#define ANSI_COLOR_BG_RESET     "\x1b[0m"

#if defined (XPAR_V_HDMI_TX_SS_HDCP_1_4_DEVICE_ID) || \
	defined (XPAR_XHDCP22_TX_NUM_INSTANCES)

/* If HDCP 1.4 or HDCP 2.2 is in the system
 * then use the HDCP abstraction layer */
#define USE_HDCP_HDMI_TX

#endif

/**************************** Type Definitions ******************************/
/**
 * This typedef enumerated the events on the Xilinx HDMI Receiver
 * state machine.
 */
typedef enum {
	XV_TX_HDMI_EVENT_DISCONNECTED = 0,
	XV_TX_HDMI_EVENT_CONNECTED,
	XV_TX_HDMI_EVENT_BRDGUNLOCKED,
	XV_TX_HDMI_EVENT_BRDGUNDERFLOW,
	XV_TX_HDMI_EVENT_BRDGOVERFLOW,
	XV_TX_HDMI_EVENT_VSYNC,
	XV_TX_HDMI_EVENT_STREAMUP,
	XV_TX_HDMI_EVENT_STREAMDOWN,
	XV_TX_HDMI_EVENT_FRLCONFIG,
	XV_TX_HDMI_EVENT_FRLFFE,
	XV_TX_HDMI_EVENT_FRLSTART,
	XV_TX_HDMI_EVENT_FRLSTOP,
	XV_TX_HDMI_EVENT_TMDSCONFIG,
	XV_TX_HDMI_EVENT_POLL,
	XV_TX_HDMI_NUM_EVENTS
} XV_Tx_Hdmi_Events;

/**
 * This typedef enumerates the Xilinx HDMI Receiver states
 */
typedef enum {
	XV_TX_HDMI_STATE_DISCONNECTED = 0,
	XV_TX_HDMI_STATE_CONNECTED,
	XV_TX_HDMI_STATE_BRDG_UNLOCKED,
	XV_TX_HDMI_STATE_VSYNC,
	XV_TX_HDMI_STATE_STREAMON,
	XV_TX_HDMI_STATE_STREAMOFF,
	XV_TX_HDMI_STATE_FRLCONFIG,
	XV_TX_HDMI_STATE_FRLFFE,
	XV_TX_HDMI_STATE_FRLSTART,
	XV_TX_HDMI_STATE_FRLSTOP,
	XV_TX_HDMI_STATE_TMDSCONFIG,
	XV_TX_HDMI_NUM_STATE
} XV_Tx_Hdmi_State;

/**
 * This typedef enumerates the source for CKE configurations
 * for HDMI 2.1 TX FRL.
 */
typedef enum {
	XV_TX_HDMI_FRL_CKE_SRC_INTERNAL,
	XV_TX_HDMI_FRL_CKE_SRC_EXTERNAL
} XV_TX_Hdmi_FrlCkeSrc;

/**
 * This typedef defines the Edid information needed to
 * initiate FRL training.
 */
typedef struct {
	XV_VidC_Supp IsSCDCPresent;
	u8           MaxFrlRateSupp;
} XV_Tx_Hdmi_FrlEdidInfo;

/**
 * This typedef defines the state and event related information
 * for the Xilinx Video Receiver Hdmi state machine.
 */
typedef struct {
	u16 HdmiTxPendingEvents;
	XV_Tx_Hdmi_State PreviousState;
	XV_Tx_Hdmi_State CurrentState;
} XV_Tx_Hdmi_StateInfo;

/**
 * This typedef defines the current state and configuration of the
 * Xilinx Video transmitter.
 */
typedef struct {
	u64 LineRate;
	XV_HdmiTx1_AudioFormatType AudioFormat;
	u8  AudioChannels;
	u8  IsFrl;
	u8  IsHdmi;
	u8  SinkEdidParsed;
	u8  Vsync0_after_steamOn;
	XV_TX_Hdmi_FrlCkeSrc FrlCkeSrc;
	XV_Tx_Hdmi_FrlEdidInfo FrlEdidInfo;
} XV_Tx_Hdmi_ConfigInfo;

/**
 *
 */
typedef struct {
	u32 TxBrdgOverflowCnt;
	u32 TxBrdgUnderflowCnt;
} XV_Tx_Hdmi_ErrorStats;

/**
 * This typedef tracks the HDCP status of the transmitter state machine.
 */
typedef struct {
	u8  DownstreamInstanceBinded;
	u8  DownstreamInstanceConnected;
	u8  DownstreamInstanceStreamUp;
	u8  IsReady;
	u8  IsRepeater;
} XV_Tx_Hdcp_Config;

/**
 * This typedef defines the callbacks to differentiate the types
 * of the transmitter state machine.
 */
typedef enum {
	XV_TX_TRIG_HANDLER_CONNECTION_CHANGE,
	XV_TX_TRIG_HANDLER_SETUP_TXTMDSREFCLK,
	XV_TX_TRIG_HANDLER_SETUP_AUDVID,
	XV_TX_TRIG_HANDLER_STREAM_ON,
	XV_TX_TRIG_HANDLER_ENABLE_CABLE_DRIVERS,
	XV_TX_TRIG_HANDLER_VSYNC_RECV,
	XV_TX_TRIG_HANDLER_STREAM_OFF,
	XV_TX_TRIG_HANDLER_READYTOSTARTTX,
	XV_TX_TRIG_HANDLER_SETUP_TXFRLREFCLK,
	XV_TX_TRIG_HANDLER_FRL_FFE_CONFIG_DEVICE,
	XV_TX_TRIG_HANDLER_FRL_CONFIG_DEVICE_SETUP,
	XV_TX_TRIG_HANDLER_HDCP_FORCE_BLANKING,
	XV_TX_GET_FRL_CLOCK,
} XV_Tx_Trigger_CallbackHandler;

/**
 * This typedef defines all the interrupt vector IDs
 * required for the Hdmi Tx Ss core to be connected
 * to the interrupt controller
 */
typedef struct {
	u32 IntrVecId_HdmiTxSs;
	u32 IntrVecId_Hdcp14;
	u32 IntrVecId_Hdcp14Timer;
	u32 IntrVecId_Hdcp22Timer;
	u32 IntrVecId_VPhy;
} XV_Tx_IntrVecId;

/**
 * Define a function pointer for user enabled printing.
 */
typedef void (*XV_Tx_Debug_Printf)(const char *fmt, ...);

extern XV_Tx_Debug_Printf XV_Tx_DebugPrintf;
extern XV_Tx_Debug_Printf XV_TxDebugTxSMPrintf;
extern XV_Tx_Debug_Printf XV_TxDebugTxNewStreamSetupPrintf;

#define xdbg_xv_tx_print \
	if (XV_Tx_DebugPrintf != NULL) XV_Tx_DebugPrintf
#define xdbg_xv_tx_statemachine_print \
	if (XV_TxDebugTxSMPrintf != NULL) XV_TxDebugTxSMPrintf
#define xdbg_xv_tx_new_stream_setup_print \
	if (XV_TxDebugTxNewStreamSetupPrintf != NULL) \
		XV_TxDebugTxNewStreamSetupPrintf

/* Pointer to HDCP Repeater hooks. */
typedef void (*XV_Tx_HdcpRepeaterEvent_Callback)(void *InstancePtr, void *Event);

/* Pointer to Video Stream hooks. */
typedef void (*XV_Tx_VideoEvent_Callback)(void *InstancePtr, void *Event);

/* Pointer to Transmitter State Machine callback hooks. */
typedef void (*XV_Tx_SM_Callback)(void *InstancePtr);

/* Pointer to Transmitter State Machine callback hooks. */
typedef void (*XV_Tx_SM_Info_Callback)(void *InstancePtr, u8 Data);

typedef enum {
	XV_TX_HDMITXSS_STREAM_UP,
	XV_TX_HDMITXSS_STREAM_DOWN
} XV_Tx_HdmiTxSs_StreamState;

/**
 * This typedef defines the XV_Tx instance.
 */
typedef struct {
	XV_HdmiTxSs1 *HdmiTxSs;

	XV_Tx_Hdmi_StateInfo StateInfo;
	XV_Tx_Hdmi_ConfigInfo ConfigInfo;
	XV_Tx_Hdmi_ErrorStats ErrorStats;
	XV_Tx_HdmiTxSs_StreamState XV_Tx_StreamState;
#ifdef USE_HDCP_HDMI_TX
	XV_Tx_Hdcp_Config HdcpConfig;
#endif
	XHdmiphy1      *VidPhy;	/**< Video Phy reference. */
	u8 ExclHdcpAuthReqFlag; /**< Flag for exclusion of HDCP
			       *  authentication requests to hdcp drivers,
			       *  in order to keep the processor from
			       *  being over-used by constant polling. */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	XScuGic   *Intc; /**< Interrupt Controller reference */
#else
	XIntc     *Intc; /**< Interrupt Controller reference */
#endif

	XV_Tx_SM_Callback TxCableConnectionChange;
	void *TxCableConnectionChangeCallbackRef;
	XV_Tx_SM_Callback SetupTxTmdsRefClkCb;
	void *SetupTxTmdsRefClkCbRef;
	XV_Tx_SM_Callback SetupTxFrlRefClkCb;
	void *SetupTxFrlRefClkCbRef;
	XV_Tx_SM_Callback TxStreamOff;
	void *TxStreamOffCallbackRef;
	XV_Tx_SM_Callback SetupAudVidSrc;
	void *SetupAudVidSrcCallbackRef;
	XV_Tx_SM_Callback TxStreamOn;
	void *TxStreamOnCallbackRef;
	XV_Tx_SM_Callback EnableCableDriversCb;
	void *EnableCableDriversCbRef;
	XV_Tx_SM_Callback TxVidSyncRecv;
	void *TxVidSyncRecvCallbackRef;
	XV_Tx_SM_Callback TxReadyToStart;
	void *TxReadyToStartCallbackRef;
	XV_Tx_SM_Callback TxFrlConfigDeviceSetupCb;
	void *TxFrlConfigDeviceSetupCbRef;
	XV_Tx_SM_Callback TxFrlFfeConfigDeviceCb;
	void *TxFrlFfeConfigDeviceCbRef;
	XV_Tx_SM_Callback TxTmdsConfig;
	void *TxTmdsConfigCallbackRef;
	XV_Tx_SM_Callback TxLogTimeStamp;
	void *TxLogTimeStampCallbackRef;
	XV_Tx_SM_Callback GetTxFrlClkCb;
	void *GetTxFrlClkCbRef;
#ifdef USE_HDCP_HDMI_TX
	XV_Tx_SM_Callback HdcpForceBlankCb;
	void *HdcpForceBlankCbRef;
#endif
} XV_Tx;

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/

u32 XV_Tx_Hdmi_Initialize(XV_Tx *InstancePtr, u32 HdmiTxSsDevId,
		u32 VPhyDevId, XV_Tx_IntrVecId IntrVecIds);

void XV_Tx_SetDebugPrints(XV_Tx_Debug_Printf PrintFunc);
void XV_Tx_SetDebugStateMachinePrints(XV_Tx_Debug_Printf PrintFunc);
void XV_Tx_SetDebugTxNewStreamSetupPrints(XV_Tx_Debug_Printf PrintFunc);

u32 XV_Tx_SetTriggerCallbacks(XV_Tx *InstancePtr,
		XV_Tx_Trigger_CallbackHandler Handler,
		void *Callback, void *CallbackRef);

void XV_Tx_Hdcp_Poll(XV_Tx *InstancePtr);
void XV_Tx_SetHdcpAuthReqExclusion(XV_Tx *InstancePtr, u8 Set);
u8 XV_Tx_GetHdcpAuthReqExclusion(XV_Tx *InstancePtr);
#ifdef USE_HDCP_HDMI_TX
void XV_Tx_Hdcp_Authenticate(XV_Tx *InstancePtr);
void XV_Tx_Hdcp_SetCapability(XV_Tx *InstancePtr, int Protocol);
#endif

u8 XV_Tx_IsConnected(XV_Tx *InstancePtr);
void XV_Tx_SetAudioFormatAndChannels(XV_Tx *InstancePtr,
				     XV_HdmiTx1_AudioFormatType AudioFormat,
				     u8 AudioChannels);
void XV_Tx_SetVic(XV_Tx *InstancePtr, u8 Vic);
void XV_Tx_SetHdmiFrlMode(XV_Tx *InstancePtr);
void XV_Tx_SetHdmiTmdsMode(XV_Tx *InstancePtr);
void XV_Tx_SetDviMode(XV_Tx *InstancePtr);
void XV_Tx_SetLineRate(XV_Tx *InstancePtr, u64 LineRate);
u64 XV_Tx_GetLineRate(XV_Tx *InstancePtr);
void XV_Tx_SetEdidParsingDone(XV_Tx *InstancePtr, u8 Set);
u32 XV_Tx_StartFrlTraining(XV_Tx *InstancePtr,
			   XV_VidC_Supp IsSCDCPresent,
			   u8 MaxFrlRate);
void XV_Tx_SetFrlEdidInfo(XV_Tx *InstancePtr, XV_VidC_Supp IsSCDCPresent,
			  u8 MaxFrlRateSupp);
void XV_Tx_SetFrlIntVidCkeGen(XV_Tx *InstancePtr);
void XV_Tx_SetFrlExtVidCkeGen(XV_Tx *InstancePtr);
void XV_Tx_SetUseInternalACR(XV_Tx *InstancePtr);
void XV_Tx_SetUseExternalACR(XV_Tx *InstancePtr);
void XV_Tx_SetAudSamplingFreq(XV_Tx *InstancePtr,
		XHdmiC_SamplingFrequencyVal AudSampleFreqVal);
void XV_Tx_SetFRLCkeSrcToExternal(XV_Tx *InstancePtr, u32 Value);

u32 XV_Tx_VideoSetupAndStart(XV_Tx *InstancePtr,
			     XVidC_VideoStream *HdmiTxSsVidStreamPtr);
u32 XV_Tx_GetNVal(XV_Tx *InstancePtr,
		  XHdmiC_SamplingFrequencyVal SampFreqVal);

#endif
#endif /* _XHDMI_EXDES_SM_TX_H_ */
