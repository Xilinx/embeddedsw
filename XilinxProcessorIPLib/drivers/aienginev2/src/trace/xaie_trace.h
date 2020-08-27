/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_trace.h
* @{
*
* Header file for AIE trace module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad  06/16/2020  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_TRACE_H
#define XAIE_TRACE_H

/***************************** Include Files *********************************/
#include "xaie_events.h"

/**************************** Type Definitions *******************************/
/* This enum captures various states of a trace module */
typedef enum {
	XAIE_TRACE_IDLE,
	XAIE_TRACE_RUNNING,
	XAIE_TRACE_OVERRUN,
} XAie_TraceState;

/* This enum captures various trace modes */
typedef enum {
	XAIE_TRACE_EVENT_TIME,
	XAIE_TRACE_EVENT_PC,
	XAIE_TRACE_INST_EXEC,
} XAie_TraceMode;

/************************** Function Prototypes  *****************************/
AieRC XAie_TraceEvent(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events Event, u8 SlotId);
AieRC XAie_TraceStartEvent(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events StartEvent);
AieRC XAie_TraceStopEvent(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events StopEvent);
AieRC XAie_TracePktConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Packet Pkt);
AieRC XAie_TraceModeConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_TraceMode Mode);
AieRC XAie_TraceGetState(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_TraceState *State);
AieRC XAie_TraceGetMode(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_TraceMode *Mode);
AieRC XAie_TraceControlConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events StartEvent,
		XAie_Events StopEvent, XAie_TraceMode Mode);
AieRC XAie_TraceControlConfigReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module);
AieRC XAie_TracePktConfigReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module);
AieRC XAie_TraceEventList(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events *Events, u8 *SlotId,
		u8 NumEvents);
AieRC XAie_TraceEventReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 SlotId);

#endif		/* end of protection macro */
