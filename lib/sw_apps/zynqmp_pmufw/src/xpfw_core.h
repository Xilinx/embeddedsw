/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPFW_CORE_H_
#define XPFW_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpfw_module.h"
#include "xpfw_scheduler.h"

#define XPFW_MAX_MOD_COUNT 32U



typedef struct {
	XPfw_Module_t ModList[XPFW_MAX_MOD_COUNT];
	XPfw_Scheduler_t Scheduler;
	u16 IsReady;
	u8 ModCount;
	u8 Mode;	/**< Mode - Safety Diagnostics Mode / Normal Mode */
} XPfw_Core_t;

XStatus XPfw_CoreInit(u32 Options);
XStatus XPfw_CoreConfigure(void);
XStatus XPfw_CoreDispatchEvent( u32 EventId);
const XPfw_Module_t *XPfw_CoreCreateMod(void);
XStatus XPfw_CoreScheduleTask(const XPfw_Module_t *ModPtr, u32 Interval, VoidFunction_t CallbackRef);
s32 XPfw_CoreRemoveTask(const XPfw_Module_t *ModPtr, u32 Interval, VoidFunction_t CallbackRef);
XStatus XPfw_CoreStopScheduler(void);
XStatus XPfw_CoreLoop(void);
void XPfw_CorePrintStats(void);
XStatus XPfw_CoreRegisterEvent(const XPfw_Module_t *ModPtr, u32 EventId);
XStatus XPfw_CoreDeRegisterEvent(const XPfw_Module_t *ModPtr, u32 EventId);

XStatus XPfw_CoreDispatchIpi(u32 IpiNum, u32 SrcMask);

void XPfw_CoreTickHandler(void);
XStatus XPfw_CoreIsReady(void);

XStatus XPfw_CoreSetCfgHandler(const XPfw_Module_t *ModPtr, XPfwModCfgInitHandler_t CfgHandler);
XStatus XPfw_CoreSetEventHandler(const XPfw_Module_t *ModPtr, XPfwModEventHandler_t EventHandlerFn);
XStatus XPfw_CoreSetIpiHandler(const XPfw_Module_t *ModPtr, XPfwModIpiHandler_t IpiHandlerFn, u16 IpiId);

void XPfw_EnableSlvErr(void);
void XPfw_Exception_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_CORE_H_ */
