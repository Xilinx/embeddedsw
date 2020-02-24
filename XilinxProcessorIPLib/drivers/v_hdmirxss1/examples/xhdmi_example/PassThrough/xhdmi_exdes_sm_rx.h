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
#ifndef _XHDMI_EXDES_SM_RX_H_
#define _XHDMI_EXDES_SM_RX_H_

/***************************** Include Files *********************************/
#include <string.h>
#include "xparameters.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#include "xv_hdmirxss1.h"
#include "xhdmiphy1.h"
#include "xvidc.h"
#if defined (ARMR5) || (__aarch64__) || (__arm__)
#include "xscugic.h"
#else
#include "xintc.h"
#endif

/************************** Constant Definitions ****************************/
/* Define constants to print colors */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_HIGH_WHITE   "\x1b[97m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define ANSI_COLOR_BG_MAGENTA   "\x1b[45m"
#define ANSI_COLOR_BG_HIGH_CYAN "\x1b[46;1m"
#define ANSI_COLOR_BG_BLUE      "\x1b[44m"
#define ANSI_COLOR_BG_RED       "\x1b[41m"
#define ANSI_COLOR_BG_RESET     "\x1b[0m"

#if defined (XPAR_V_HDMI_RX_SS_HDCP_1_4_DEVICE_ID) || \
	defined (XPAR_XHDCP22_RX_NUM_INSTANCES)

/* If HDCP 1.4 or HDCP 2.2 is in the system
 * then use the HDCP abstraction layer */
#define USE_HDCP_HDMI_RX

#endif

/**************************** Type Definitions ******************************/

/**
 * This typedef enumerated the events on the Xilinx HDMI Receiver
 * state machine.
 */
typedef enum {
        XV_RX_HDMI_EVENT_DISCONNECTED = 0,
        XV_RX_HDMI_EVENT_CONNECTED,
        XV_RX_HDMI_EVENT_STREAMINIT,
        XV_RX_HDMI_EVENT_STREAMUP,
        XV_RX_HDMI_EVENT_STREAMDOWN,
        XV_RX_HDMI_EVENT_POLL,
	XV_RX_HDMI_EVENT_PHYRESET,
        XV_RX_HDMI_EVENT_FRLCONFIG,
        XV_RX_HDMI_EVENT_FRLSTART,
        XV_RX_HDMI_EVENT_TMDSCONFIG,
        XV_RX_HDMI_NUM_EVENTS
} XV_Rx_Hdmi_Events;

/**
 * This typedef enumerates the Xilinx HDMI Receiver states
 */
typedef enum {
        XV_RX_HDMI_STATE_DISCONNECTED = 0,
        XV_RX_HDMI_STATE_CONNECTED,
        XV_RX_HDMI_STATE_NOSTREAM,
        XV_RX_HDMI_STATE_STREAMINITIALIZED,
        XV_RX_HDMI_STATE_STREAMON,
        XV_RX_HDMI_STATE_STREAMOFF,
	XV_RX_HDMI_STATE_PHYRESET,
        XV_RX_HDMI_STATE_FRLCONFIG,
        XV_RX_HDMI_STATE_FRLSTART,
        XV_RX_HDMI_STATE_TMDSCONFIG,
        XV_RX_HDMI_NUM_STATE
} XV_Rx_Hdmi_State;

/**
 * This typedef defines the state and event related information
 * for the Xilinx Video Receiver Hdmi state machine.
 */
typedef struct {
        u16 HdmiRxPendingEvents;
        XV_Rx_Hdmi_State PreviousState;
        XV_Rx_Hdmi_State CurrentState;
} XV_Rx_HdmiRx_StateInfo;

/**
 * This typedef defines a counter to keep track of the bridge
 * error callbacks received from the RX SS interrupts.
 */
typedef struct {
	u32 RxBrdgOverflowCnt;
	/* u32 RxBrdgUnderflowCnt; */
} XV_Rx_HdmiRx_ErrorStats;

/**
 * This typedef tracks the HDCP status of the receiver state machine.
 */
typedef struct {
	u8  UpstreamInstanceBinded;
	u8  UpstreamInstanceConnected;
	u8  UpstreamInstanceStreamUp;
	u8  IsReady;
	u8  IsRepeater;
	u8  StreamType;
} XV_Rx_Hdcp_Config;

/**
 * This typedef defines the callbacks to differentiate the types
 * of the receiver state machine.
 */
typedef enum {
	XV_RX_TRIG_HANDLER_CONNECTION_CHANGE,
	XV_RX_TRIG_HANDLER_STREAM_OFF,
	XV_RX_TRIG_HANDLER_STREAM_ON,
	XV_RX_TRIG_HANDLER_AUDIOCONFIG,
	XV_RX_TRIG_HANDLER_AUXEVENT,
	XV_RX_TRIG_HANDLER_LINKSTATUS,
	XV_RX_TRIG_HANDLER_HDCP_FORCE_BLANKING,
	XV_RX_TRIG_HANDLER_HDCP_SET_CONTENTSTREAMTYPE,
	XV_RX_TRIG_HANDLER_CLKSRC_CONFIG,
	XV_RX_TRIG_HANDLER_CLKSRC_SEL,
} XV_Rx_Trigger_CallbackHandler;

typedef struct {
	u32 IntrVecId_HdmiRxSs;
	u32 IntrVecId_Hdcp14;
	u32 IntrVecId_Hdcp14Timer;
	u32 IntrVecId_Hdcp22Timer;
	u32 IntrVecId_VPhy;
} XV_Rx_IntrVecId;

/**
 * Define a function pointer for user enabled printing.
 */
typedef void (*XV_Rx_Debug_Printf)(const char *fmt, ...);

extern XV_Rx_Debug_Printf XV_Rx_DebugPrintf;
extern XV_Rx_Debug_Printf XV_RxDebugRxSMPrintf;

#define xdbg_xv_rx_print \
	if (XV_Rx_DebugPrintf != NULL) XV_Rx_DebugPrintf
#define xdbg_xv_rx_statemachine_print \
	if(XV_RxDebugRxSMPrintf != NULL) XV_RxDebugRxSMPrintf

/* Pointer to HDCP Repeater hooks. */
typedef void (*XV_Rx_HdcpRepeaterEvent_Callback)(void *InstancePtr, u8 Event);

/* Pointer to Video Stream hooks. */
typedef void (*XV_Rx_VideoEvent_Callback)(void *InstancePtr, u8 Event);

/* Pointer to Receiver State Machine callback hooks. */
typedef void (*XV_Rx_SM_Callback)(void *InstancePtr);

/* Pointer to Receiver State Machine callback hooks with two arguments. */
typedef void (*XV_Rx_SM_Info_Callback)(void *InstancePtr, u8 Data);

/**
 * This typedef enumerates the types of HDCP configurations that can be
 * supported by the Xilinx Video Receiver system.
 */
typedef enum {
	XV_RX_HDCP_DISABLED,
	XV_RX_HDCP_MONO_INTF,
	XV_RX_HDCP_REPEATER
} XV_Rx_hdcp_config;


/**
 * This typedef defines the XV_Rx instance.
 */
typedef struct {
	XV_HdmiRxSs1 *HdmiRxSs;

	XHdmiphy1      *VidPhy;	/**< Video Phy reference. */

	XV_Rx_HdmiRx_StateInfo StateInfo;
	XV_Rx_HdmiRx_ErrorStats ErrorStats;
#ifdef USE_HDCP_HDMI_RX
	XV_Rx_Hdcp_Config HdcpConfig;
#endif
	u8 StreamInitFail;

#if defined (ARMR5) || (__aarch64__) || (__arm__)
	XScuGic   *Intc; /**< Interrupt Controller reference */
#else
	XIntc     *Intc; /**< Interrupt Controller reference */
#endif

	XV_Rx_SM_Callback RxCableConnectionChange;
	void *RxCableConnectionChangeCallbackRef;
	XV_Rx_SM_Callback RxStreamOff;
	void *RxStreamOffCallbackRef;
	XV_Rx_SM_Callback RxStreamOn;
	void *RxStreamOnCallbackRef;
	XV_Rx_SM_Callback RxReportSysInfo;
	void *RxReportSysInfoCallbackRef;
	XV_Rx_SM_Callback RxAudio;
	void *RxAudioCallbackRef;
	XV_Rx_SM_Callback RxAuxCb;
	void *RxAuxCbRef;
	XV_Rx_SM_Callback RxClkSrcConfig;
	void *RxClkSrcConfigCallbackRef;
	XV_Rx_SM_Callback RxClkSrcSelCb;
	void *RxClkSrcSelCallbackRef;
#ifdef USE_HDCP_HDMI_RX
	XV_Rx_SM_Callback HdcpSetContentStreamTypeCb;
	void *HdcpSetContentStreamTypeCbRef;

	XV_Rx_SM_Callback HdcpForceBlankCb;
	void *HdcpForceBlankCbRef;
#endif

	u8 Edid[256];

	u32 AcrNVal;
	u32 AcrCtsVal;
} XV_Rx;

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
/* VPhy structure */
extern XHdmiphy1 Hdmiphy1;
/* HDMI RX SS structure */
extern XV_HdmiRxSs1 HdmiRxSs;

/************************** Function Prototypes *****************************/

u32 XV_Rx_Hdmi_Initialize(XV_Rx *InstancePtr, u32 HdmiRxSsDevId,
			  u32 VPhyDevId, XV_Rx_IntrVecId IntrVecIds);

void XV_Rx_SetDebugPrints(XV_Rx_Debug_Printf PrintFunc);
void XV_Rx_SetDebugStateMachinePrints(XV_Rx_Debug_Printf PrintFunc);

u32 XV_Rx_SetTriggerCallbacks(XV_Rx *InstancePtr,
			      XV_Rx_Trigger_CallbackHandler Handler,
			      void *Callback, void *CallbackRef);

void XV_Rx_Hdcp_Poll(XV_Rx *InstancePtr);
#ifdef USE_HDCP_HDMI_RX
u8 XV_Rx_Hdcp_GetStreamType(XV_Rx *InstancePtr);
void XV_Rx_Hdcp_SetCapability(XV_Rx *InstancePtr, int Protocol);
#endif

void XV_Rx_SetHpd(XV_Rx *InstancePtr, u8 Hpd);
u8 XV_Rx_IsStreamOn(XV_Rx *InstancePtr);
u8 XV_Rx_IsConnected(XV_Rx *InstancePtr);
u32 XV_Rx_PulsePllReset(XV_Rx *InstancePtr);
void XV_Rx_TmdsClkEnable(XV_Rx *InstancePtr, u32 Set);
u64 XV_Rx_GetLineRate(XV_Rx *InstancePtr);
XHdmiC_SamplingFrequencyVal XV_Rx_GetFrlAudSampFreq(XV_Rx *InstancePtr);
XHdmiC_SamplingFrequencyVal XV_Rx_GetTmdsAudSampFreq(XV_Rx *InstancePtr);
u32 XV_Rx_AcrNValDiffCheck(XV_Rx *InstancePtr);

#endif /* _XHDMI_EXDES_SM_RX_H_ */
