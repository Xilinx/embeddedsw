/******************************************************************************
* Copyright (C) 2015 - 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
******************************************************************************/

#ifndef XPFW_SCHEDULER_H_
#define XPFW_SCHEDULER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpfw_default.h"

#define XPFW_SCHED_MAX_TASK	10U

/* Values for TaskPtr->Status */
#define XPFW_TASK_STATUS_TRIGGERED	0x5AFEC0C0U
#define XPFW_TASK_STATUS_DISABLED	0x00000000U

typedef void (*XPfw_Callback_t) (void);

struct XPfw_Task_t{
	u32 Interval;
	u32 OwnerId;
	u32 Status;
	XPfw_Callback_t Callback;
};

typedef struct {
	struct XPfw_Task_t TaskList[XPFW_SCHED_MAX_TASK];
	u32 TaskCount;
	u32 PitBaseAddr;
	u32 Tick;
	u32 Enabled;
} XPfw_Scheduler_t ;

void XPfw_SchedulerTickHandler(XPfw_Scheduler_t *SchedPtr);
XStatus XPfw_SchedulerInit(XPfw_Scheduler_t *SchedPtr, u32 PitBaseAddr);
XStatus XPfw_SchedulerStart(XPfw_Scheduler_t *SchedPtr);
XStatus XPfw_SchedulerStop(XPfw_Scheduler_t *SchedPtr);
XStatus XPfw_SchedulerProcess(XPfw_Scheduler_t *SchedPtr);
XStatus XPfw_SchedulerAddTask(XPfw_Scheduler_t *SchedPtr, u32 OwnerId,u32 MilliSeconds, XPfw_Callback_t CallbackFn);
XStatus XPfw_SchedulerRemoveTask(XPfw_Scheduler_t *SchedPtr, u32 OwnerId, u32 MilliSeconds, XPfw_Callback_t CallbackFn);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_SCHEDULER_H_ */
