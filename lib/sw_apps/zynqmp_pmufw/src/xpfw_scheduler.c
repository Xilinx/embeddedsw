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

#include "xpfw_scheduler.h"

/**
 * PMU PIT Clock Frequency and Tick Calculation
 */
#define PMU_PIT_CLK_FREQ	XPFW_CFG_PMU_CLK_FREQ
#define TICK_MILLISECONDS	10U
#define COUNT_PER_TICK ((PMU_PIT_CLK_FREQ / 1000U)* TICK_MILLISECONDS )

/**
 * Microblaze IOModule PIT Register Offsets
 * Used internally in this file
 */
#define PIT_PRELOAD_OFFSET	0U
#define PIT_COUNTER_OFFSET	4U
#define PIT_CONTROL_OFFSET	8U

static u32 is_task_active(XPfw_Scheduler_t *SchedPtr, u32 TaskListIndex)
{
	u32 ReturnVal;

	/* Periodic */
	if ((0U != SchedPtr->TaskList[TaskListIndex].Interval)
		&& (NULL != SchedPtr->TaskList[TaskListIndex].Callback)
		&& (0U == (SchedPtr->Tick
				% SchedPtr->TaskList[TaskListIndex].Interval))) {
		ReturnVal = TRUE;
	} else if ((0U == SchedPtr->TaskList[TaskListIndex].Interval)
			&& (NULL != SchedPtr->TaskList[TaskListIndex].Callback)) {
		/* Non-Periodic */
		ReturnVal = TRUE;
	} else {
		ReturnVal = FALSE;
	}

	return ReturnVal;
}

static u32 is_task_non_periodic(XPfw_Scheduler_t *SchedPtr, u32 TaskListIndex)
{
	u32 ReturnVal;

	if ((0U == SchedPtr->TaskList[TaskListIndex].Interval)
		&& (NULL != SchedPtr->TaskList[TaskListIndex].Callback)) {
		ReturnVal = TRUE;
	} else {
		ReturnVal = FALSE;
	}

	return ReturnVal;
}

XStatus XPfw_SchedulerInit(XPfw_Scheduler_t *SchedPtr, u32 PitBaseAddr)
{
	u32 Idx;
	XStatus Status;

	if (SchedPtr == NULL) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Disable all the tasks */
	for (Idx = 0U; Idx < XPFW_SCHED_MAX_TASK; Idx++) {
		SchedPtr->TaskList[Idx].Interval = 0U;
		SchedPtr->TaskList[Idx].Callback = NULL;
		SchedPtr->TaskList[Idx].Status = XPFW_TASK_STATUS_DISABLED;
	}

	SchedPtr->Enabled = FALSE;
	SchedPtr->PitBaseAddr = PitBaseAddr;
	SchedPtr->Tick = 0U;
	XPfw_Write32(SchedPtr->PitBaseAddr + PIT_CONTROL_OFFSET, 0U);

	/* Successfully completed init */
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPfw_SchedulerStart(XPfw_Scheduler_t *SchedPtr)
{
	XStatus Status;

	if (SchedPtr == NULL) {
		Status = XST_FAILURE;
		goto done;
	}

	SchedPtr->Enabled = TRUE;

	XPfw_Write32(SchedPtr->PitBaseAddr + PIT_PRELOAD_OFFSET,
			COUNT_PER_TICK);
	XPfw_Write32(SchedPtr->PitBaseAddr + PIT_CONTROL_OFFSET, 3U);
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPfw_SchedulerStop(XPfw_Scheduler_t *SchedPtr)
{
	SchedPtr->Enabled =FALSE;

	XPfw_Write32(SchedPtr->PitBaseAddr + PIT_PRELOAD_OFFSET, 0U );
	XPfw_Write32(SchedPtr->PitBaseAddr + PIT_CONTROL_OFFSET, 0U );

	return XST_SUCCESS;
}

void XPfw_SchedulerTickHandler(XPfw_Scheduler_t *SchedPtr)
{
	u32 Idx;

	SchedPtr->Tick++;
	for (Idx = 0U; Idx < XPFW_SCHED_MAX_TASK; Idx++) {
		/* Check if it this task can be triggered */
		if (TRUE == is_task_active(SchedPtr, Idx)) {
			/* Mark the Task as TRIGGERED */
			SchedPtr->TaskList[Idx].Status = XPFW_TASK_STATUS_TRIGGERED;
		}
	}
}

XStatus XPfw_SchedulerProcess(XPfw_Scheduler_t *SchedPtr)
{
	u32 Idx;
	XStatus Status;
	u32 CallCount = 0U;

	for (Idx = 0U; Idx < XPFW_SCHED_MAX_TASK; Idx++) {
		/* Check if the task is triggered and has a valid Callback */
		if ((XPFW_TASK_STATUS_TRIGGERED == SchedPtr->TaskList[Idx].Status) &&
			(NULL != SchedPtr->TaskList[Idx].Callback)) {
			/* Execute the Task */
			SchedPtr->TaskList[Idx].Callback();
			/* Disable the executed Task */
			SchedPtr->TaskList[Idx].Status = XPFW_TASK_STATUS_DISABLED;
			CallCount++;
	                /* Remove the Non-Periodic Task */
		        if (TRUE == is_task_non_periodic(SchedPtr, Idx)) {
			        SchedPtr->TaskList[Idx].Callback = NULL;
			}
		}
	}

	if (CallCount > 0U) {
		Status = XST_SUCCESS;
	} else {
		/* Failed because none of the tasks were triggered */
		Status = XST_FAILURE;
	}

	return Status;
}

XStatus XPfw_SchedulerAddTask(XPfw_Scheduler_t *SchedPtr, u32 OwnerId,u32 MilliSeconds, XPfw_Callback_t CallbackFn)
{
	u32 Idx;
	XStatus Status;

	/* Get the Next Free Task Index */
	for (Idx=0U;Idx < XPFW_SCHED_MAX_TASK;Idx++) {
		if (NULL == SchedPtr->TaskList[Idx].Callback){
			break;
		}
	}

	/* Check if we have reached Max Task limit */
	if (XPFW_SCHED_MAX_TASK == Idx) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Add Interval as a factor of TICK_MILLISECONDS */
	SchedPtr->TaskList[Idx].Interval = MilliSeconds/TICK_MILLISECONDS;
	SchedPtr->TaskList[Idx].OwnerId = OwnerId;
	SchedPtr->TaskList[Idx].Callback = CallbackFn;
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPfw_SchedulerRemoveTask(XPfw_Scheduler_t *SchedPtr, u32 OwnerId, u32 MilliSeconds, XPfw_Callback_t CallbackFn)
{
	u32 Idx;
	u32 TaskCount = 0U;

	/*Find the Task Index */
	for (Idx = 0U; Idx < XPFW_SCHED_MAX_TASK; Idx++) {
		if ((CallbackFn == SchedPtr->TaskList[Idx].Callback) &&
		    (SchedPtr->TaskList[Idx].OwnerId == OwnerId) &&
		    ((SchedPtr->TaskList[Idx].Interval == (MilliSeconds/TICK_MILLISECONDS)) ||
				(0U == MilliSeconds))) {
			SchedPtr->TaskList[Idx].Interval = 0U;
			SchedPtr->TaskList[Idx].OwnerId = 0U;
			SchedPtr->TaskList[Idx].Callback = NULL;
			TaskCount++;
		}
	}

	XPfw_Printf(DEBUG_DETAILED,"%s: Removed %lu tasks\r\n",
			__func__, TaskCount);

	return ((TaskCount > 0U) ? XST_SUCCESS : XST_FAILURE);
}
