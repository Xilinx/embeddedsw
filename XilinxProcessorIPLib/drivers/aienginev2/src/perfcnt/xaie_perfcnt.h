/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_perfcnt.h
* @{
*
* Header file for performance counter implementations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- ------   -------- -----------------------------------------------------
* 1.0   Dishita  11/21/2019  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIEPERFCNT_H
#define XAIEPERFCNT_H

/***************************** Include Files *********************************/
#include "xaie_events.h"
#include "xaie_helper.h"
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaiegbl_defs.h"

/************************** Function Prototypes  *****************************/
AieRC XAie_PerfCounterGet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter, u32 *CounterVal);
AieRC XAie_PerfCounterControlSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter, XAie_Events StartEvent,
		XAie_Events StopEvent);
AieRC XAie_PerfCounterResetControlSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter, XAie_Events ResetEvent);
AieRC XAie_PerfCounterSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter, u32 CounterVal);
AieRC XAie_PerfCounterEventValueSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter, u32 EventVal);
AieRC XAie_PerfCounterControlReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter);
AieRC XAie_PerfCounterResetControlReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter);
AieRC XAie_PerfCounterReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter);
AieRC XAie_PerfCounterEventValueReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter);
AieRC XAie_PerfCounterGetControlConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 Counter, XAie_Events *StartEvent,
		XAie_Events *StopEvent, XAie_Events *ResetEvent);
#endif		/* end of protection macro */
