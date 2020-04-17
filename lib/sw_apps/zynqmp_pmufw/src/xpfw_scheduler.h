/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
	u32 PitBaseAddr;
	u32 Tick;
	u32 Enabled;
} XPfw_Scheduler_t ;

void XPfw_SchedulerTickHandler(XPfw_Scheduler_t *SchedPtr);
XStatus XPfw_SchedulerInit(XPfw_Scheduler_t *SchedPtr, u32 PitBaseAddr);
XStatus XPfw_SchedulerStart(XPfw_Scheduler_t *SchedPtr);
XStatus XPfw_SchedulerStop(XPfw_Scheduler_t *SchedPtr);
void XPfw_SchedulerProcess(XPfw_Scheduler_t *SchedPtr);
XStatus XPfw_SchedulerAddTask(XPfw_Scheduler_t *SchedPtr, u32 OwnerId,u32 MilliSeconds, XPfw_Callback_t CallbackFn);
XStatus XPfw_SchedulerRemoveTask(XPfw_Scheduler_t *SchedPtr, u32 OwnerId, u32 MilliSeconds, XPfw_Callback_t CallbackFn);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_SCHEDULER_H_ */
