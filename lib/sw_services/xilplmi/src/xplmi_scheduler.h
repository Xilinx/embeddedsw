/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/



#ifndef XPLMI_SCHEDULER_H
#define XPLMI_SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"

/************************** Constant Definitions *****************************/
#define XPLMI_SCHED_MAX_TASK		(10U)

/* Values for TaskPtr->Status */
#define XPLMI_TASK_STATUS_TRIGGERED	(0x5AFEC0C0U)
#define XPLMI_TASK_STATUS_DISABLED	(0x00000000U)

#define PMC_PMC_MB_IO_IRQ_ACK			(0xF028003CU)
#define PMC_PMC_MB_IO_IRQ_ACK_SHIFT		(0x5U)
#define PMC_PMC_MB_IO_IRQ_ACK_WIDTH		(0x1U)
#define PMC_PMC_MB_IO_IRQ_ACK_MASK		(0X0000020U)

typedef int (*XPlmi_Callback_t)(void *);

struct XPlmi_Task_t{
	u32 Interval;
	u32 OwnerId;
	int Status;
	XPlmi_Callback_t CustomerFunc;
	u32 Priority;
};

typedef struct {
	struct XPlmi_Task_t TaskList[XPLMI_SCHED_MAX_TASK];
	u32 TaskCount;
	u32 PitBaseAddr;
	u32 Tick;
	u32 Enabled;
} XPlmi_Scheduler_t ;

int XPlmi_SchedulerInit(void);
int XPlmi_SchedulerStart(XPlmi_Scheduler_t *SchedPtr);
int XPlmi_SchedulerStop(XPlmi_Scheduler_t *SchedPtr);
void XPlmi_SchedulerHandler(void *Data);
int XPlmi_SchedulerAddTask(u32 OwnerId, XPlmi_Callback_t CallbackFn,
		u32 MilliSeconds, u32 Priority);
int XPlmi_SchedulerRemoveTask(u32 OwnerId, XPlmi_Callback_t CallbackFn,
		u32 MilliSeconds);

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_SCHEDULER_H_ */
