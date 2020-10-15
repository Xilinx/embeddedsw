/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_scheduler.c
*
* This file contains code related to scheduler.
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
* 1.02  kc   07/28/2020 Wdt handler added for every scheduler tick
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       bm   10/14/2020 Code clean up
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_scheduler.h"
#include "xplmi_debug.h"
#include "xplmi_wdt.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLMI_SCHED_TICK	(10U)

/************************** Function Prototypes ******************************/
static u8 XPlmi_IsTaskNonPeriodic(XPlmi_Scheduler_t *SchedPtr,
	u32 TaskListIndex);

/************************** Variable Definitions *****************************/
static XPlmi_Scheduler_t Sched;

/*****************************************************************************/

/******************************************************************************/
/**
* @brief	The function checks the specified task is active or not, returns
* corresponding status of the task
*
* @param    	SchedPtr is Scheduler pointer
* @param    	TaskListIndex is Task index
*
* @return	TRUE or FALSE based on the task active status
*
****************************************************************************/
static u8 XPlmi_IsTaskActive(XPlmi_Scheduler_t *SchedPtr, u32 TaskListIndex)
{
	u8 ReturnVal = (u8)FALSE;

	if (XPlmi_IsTaskNonPeriodic(SchedPtr, TaskListIndex) == (u8)TRUE) {
		ReturnVal = (u8)TRUE;
	} else {
		if ((0U != SchedPtr->TaskList[TaskListIndex].Interval)
		&& (NULL != SchedPtr->TaskList[TaskListIndex].CustomerFunc)
		&& (0U == (SchedPtr->Tick
		% SchedPtr->TaskList[TaskListIndex].Interval))) {
			/* Periodic */
			ReturnVal = (u8)TRUE;
		}
	}

	return ReturnVal;
}

/******************************************************************************/
/**
* @brief	The function checks the specified task is periodic or not, returns
* corresponding periodicity status.
*
* @param    Scheduler pointer
* @param    Task index
*
* @return	TRUE or FALSE based on the task peridocity status
*
****************************************************************************/
static u8 XPlmi_IsTaskNonPeriodic(XPlmi_Scheduler_t *SchedPtr, u32 TaskListIndex)
{
	u8 ReturnVal = (u8)FALSE;

	if ((0U == SchedPtr->TaskList[TaskListIndex].Interval)
		&& (NULL != SchedPtr->TaskList[TaskListIndex].CustomerFunc)) {
		ReturnVal = (u8)TRUE;
	}

	return ReturnVal;
}

/******************************************************************************/
/**
* @brief	The function initializes scheduler and returns the
* initialization status.
*
* @param    None.
*
* @return	XST_SUCCESS
*
****************************************************************************/
int XPlmi_SchedulerInit(void)
{
	int Status = XST_FAILURE;
	u32 Idx;

	/* Disable all the tasks */
	for (Idx = 0U; Idx < XPLMI_SCHED_MAX_TASK; Idx++) {
		Sched.TaskList[Idx].Interval = 0U;
		Sched.TaskList[Idx].CustomerFunc = NULL;
	}

	Sched.Tick = 0U;

	/* Successfully completed init */
	Status = XST_SUCCESS;
	return Status;
}

/******************************************************************************/
/**
* @brief	The function is scheduler handler and it is called at regular
* intervals based on configured interval. Scheduler handler checks and adds the
* user periodic task to PLM task queue.
*
* @param	Data - Not used currently. Added as a part of generic interrupt
*               handler
*
* @return	None
*
****************************************************************************/
void XPlmi_SchedulerHandler(void *Data)
{
	int Status = XST_FAILURE;
	u32 Idx;
	XPlmi_TaskNode *Task;
	(void)Data;

	Sched.Tick++;
	XPlmi_UtilRMW(PMC_PMC_MB_IO_IRQ_ACK, PMC_PMC_MB_IO_IRQ_ACK, 0x20U);
	for (Idx = 0U; Idx < XPLMI_SCHED_MAX_TASK; Idx++) {
		/* Check if the task is triggered and has a valid Callback */
		if (XPlmi_IsTaskActive(&Sched, Idx) == (u8)TRUE) {
			/* Add the Task to the PLM Task Queue */
			Task = XPlmi_TaskCreate(Sched.TaskList[Idx].Priority,
					Sched.TaskList[Idx].CustomerFunc, NULL);
			if (Task == NULL) {
				Status = XPlmi_UpdateStatus(XPLM_ERR_TASK_CREATE, 0x0);
				XPlmi_Printf(DEBUG_GENERAL, "Task Creation Err:0x%x\n\r", Status);
				goto END;
			}
			XPlmi_TaskTriggerNow(Task);
			/* Remove the task from scheduler if it is non-periodic*/
			if (Sched.TaskList[Idx].Interval == 0U) {
				Sched.TaskList[Idx].OwnerId = 0U;
				Sched.TaskList[Idx].CustomerFunc = NULL;
			}
		}
	}

	XPlmi_WdtHandler();

END:
	return;
}

/******************************************************************************/
/**
* @brief	The function adds user periodic task to scheduler queue. The user
* shall call this function to register their scheduler task.
*
* @param	OwnerId Id of the owner, used while removing the task.
* @param	CallbackFn callback function that should be called
* @param	MilliSeconds Periodicity of the task. If Zero, task is added
*               once. Value should be in multiples of 10ms.
* @param	Priority is the priority of the task.
*
* @return	XST_SUCCESS if scheduler task is registered properly
*
****************************************************************************/
int XPlmi_SchedulerAddTask(u32 OwnerId, XPlmi_Callback_t CallbackFn,
			   u32 MilliSeconds, TaskPriority_t Priority)
{
	int Status = XST_FAILURE;
	u32 Idx;

	/* Get the Next Free Task Index */
	for (Idx = 0U; Idx < XPLMI_SCHED_MAX_TASK; Idx++) {
		if (NULL == Sched.TaskList[Idx].CustomerFunc) {
			/* Add Interval as a factor of TICK_MILLISECONDS */
			Sched.TaskList[Idx].Interval = MilliSeconds / XPLMI_SCHED_TICK;
			Sched.TaskList[Idx].OwnerId = OwnerId;
			Sched.TaskList[Idx].CustomerFunc = CallbackFn;
			Sched.TaskList[Idx].Priority = Priority;
			Status = XST_SUCCESS;
			break;
		}
	}

	return Status;
}

/******************************************************************************/
/**
* @brief	The function removes scheduler task from scheduler queue.
* The function called by the user for deregistering the scheduler task.
*
* @param	OwnerId Id of the owner, removed only if matches the ownerid
*               while adding the task.
* @param	CallbackFn callback function that is given while adding.
* @param	MilliSeconds Periodicity of the task given while adding.
*
* @return	XST_SUCCESS on success and error code on failure
*
****************************************************************************/
int XPlmi_SchedulerRemoveTask(u32 OwnerId, XPlmi_Callback_t CallbackFn,
		u32 MilliSeconds)
{
	int Status = XST_FAILURE;
	u32 Idx;
	u32 TaskCount = 0U;

	/* Find the Task Index */
	for (Idx = 0U; Idx < XPLMI_SCHED_MAX_TASK; Idx++) {
		if ((CallbackFn == Sched.TaskList[Idx].CustomerFunc) &&
			(Sched.TaskList[Idx].OwnerId == OwnerId) &&
			((Sched.TaskList[Idx].Interval ==
				(MilliSeconds / XPLMI_SCHED_TICK)) ||
				(0U == MilliSeconds))) {
			Sched.TaskList[Idx].Interval = 0U;
			Sched.TaskList[Idx].OwnerId = 0U;
			Sched.TaskList[Idx].CustomerFunc = NULL;
			TaskCount++;
		}
	}

	XPlmi_Printf(DEBUG_DETAILED, "%s: Removed %u tasks\r\n",
			__func__, TaskCount);
	if (TaskCount > 0U) {
		Status = XST_SUCCESS;
	}

	return Status;
}
