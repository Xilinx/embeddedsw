/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
*       td   10/19/2020 MISRA C Fixes
* 1.02  bm   04/03/2021 Move task creation out of interrupt context
*       bm   04/10/2021 Updated scheduler to support private data pointer and
*                       also delay in non-periodic tasks
* 1.03  ma   07/12/2021 Added support to register Error Handler for scheduler
*                       task
*       bsv  07/16/2021 Fix doxygen warnings
*       bsv  08/15/2021 Removed redundant element in structure
* 1.04  bm   07/06/2022 Refactor versal and versal_net code
* 1.05  nb   06/28/2023 Move XPLMI_SCHED_TICK here from .c file
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

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/
#define XPLMI_SCHED_MAX_TASK		(10U)
#define XPLMI_PERIODIC_TASK		(0U)
#define XPLMI_NON_PERIODIC_TASK		(1U)
#define XPLMI_SCHED_TICK		(10U)

typedef int (*XPlmi_Callback_t)(void *Data);
typedef void (*XPlmi_ErrorFunc_t)(int Status);

struct XPlmi_Task_t{
	u32 Interval;
	u32 OwnerId;
	u32 TriggerTime;
	XPlmi_Callback_t CustomerFunc;
	XPlmi_ErrorFunc_t ErrorFunc;
	XPlmi_TaskNode *Task;
	const void *Data;
	u8 Type;
};

typedef struct {
	struct XPlmi_Task_t TaskList[XPLMI_SCHED_MAX_TASK];
	u64 LastTimerTick;
	u32 TaskCount;
	u32 Tick;
} XPlmi_Scheduler_t ;

void XPlmi_SchedulerInit(void);
void XPlmi_SchedulerHandler(void *Data);
int XPlmi_SchedulerAddTask(u32 OwnerId, XPlmi_Callback_t CallbackFn,
	XPlmi_ErrorFunc_t ErrorFunc, u32 MilliSeconds, TaskPriority_t Priority,
	void *Data,	u8 TaskType);
int XPlmi_SchedulerRemoveTask(u32 OwnerId, XPlmi_Callback_t CallbackFn,
	u32 MilliSeconds, const void *Data);

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_SCHEDULER_H_ */
