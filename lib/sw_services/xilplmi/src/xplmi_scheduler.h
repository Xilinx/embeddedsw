/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_scheduler.h
*
* This header file contains declarations related to scheduler.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rm   06/02/2019 Initial release
* 1.01  kc   02/10/2020 Updated scheduler to add/remove tasks
*       kc   02/17/2020 Added configurable priority for scheduler tasks
*       bsv  04/04/2020 Code clean up
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_SCHEDULER_H
#define XPLMI_SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xplmi_task.h"

/************************** Constant Definitions *****************************/
#define XPLMI_SCHED_MAX_TASK		(10U)

/* Values for TaskPtr->Status */
#define XPLMI_TASK_STATUS_TRIGGERED	(0x5AFEC0C0)
#define XPLMI_TASK_STATUS_DISABLED	(0x00000000)

#define PMC_PMC_MB_IO_IRQ_ACK			(0xF028003CU)
#define PMC_PMC_MB_IO_IRQ_ACK_SHIFT		(0x5U)
#define PMC_PMC_MB_IO_IRQ_ACK_WIDTH		(0x1U)
#define PMC_PMC_MB_IO_IRQ_ACK_MASK		(0X0000020U)

typedef int (*XPlmi_Callback_t)(void *Data);

struct XPlmi_Task_t{
	u32 Interval;
	u32 OwnerId;
	XPlmi_Callback_t CustomerFunc;
	TaskPriority_t Priority;
};

typedef struct {
	struct XPlmi_Task_t TaskList[XPLMI_SCHED_MAX_TASK];
	u32 TaskCount;
	u32 Tick;
} XPlmi_Scheduler_t ;

int XPlmi_SchedulerInit(void);
void XPlmi_SchedulerHandler(void *Data);
int XPlmi_SchedulerAddTask(u32 OwnerId, XPlmi_Callback_t CallbackFn,
		u32 MilliSeconds, TaskPriority_t Priority);
int XPlmi_SchedulerRemoveTask(u32 OwnerId, XPlmi_Callback_t CallbackFn,
		u32 MilliSeconds);

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_SCHEDULER_H_ */
