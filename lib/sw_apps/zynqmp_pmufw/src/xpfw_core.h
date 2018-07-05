/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#ifndef XPFW_CORE_H_
#define XPFW_CORE_H_


#include "xpfw_module.h"
#include "xpfw_scheduler.h"

#define XPFW_MAX_MOD_COUNT 32U



typedef struct {
	XPfw_Module_t ModList[XPFW_MAX_MOD_COUNT];
	XPfw_Scheduler_t Scheduler;
	u8 ModCount;
	u32 IsReady;
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


#endif /* XPFW_CORE_H_ */
