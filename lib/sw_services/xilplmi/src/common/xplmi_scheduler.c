/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
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
*       td   10/19/2020 MISRA C Fixes
* 1.03  skd  03/31/2021 Adding non periodic tasks even if a task
*                       with the same handler exists, to ensure no
*                       interrupt task handlers get missed
*       bm   04/03/2021 Move task creation out of interrupt context
*       bm   04/10/2021 Updated scheduler to support private data pointer and
*                       also delay in non-periodic tasks
* 1.04  td   07/08/2021 Fix doxygen warnings
*       ma   07/12/2021 Added error handler for each scheduler task to handle
*                       cases when a scheduled task does not get executed in
*                       allotted time
*       bsv  07/16/2021 Fix doxygen warnings
*       bsv  08/02/2021 Removed unnecessary initializations to reduce code size
*       bsv  08/15/2021 Removed unwanted goto statements
* 1.05  bsv  03/05/2022 Fixed exception while deleting two consecutive tasks of
*                       same priority
*       bsv  04/03/2022 Updated logic in XPlmi_SchedulerAddTask to fix subsystem
*                       restart issue
* 1.06  skg  06/20/2022 Misra-C violation Rule 10.4 fixed
*       sk   06/27/2022 Updated logic in XPlmi_SchedulerAddTask to fix task
*                       creation error
* 1.07  ng   11/11/2022 Updated doxygen comments
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
* 1.08  nb   06/28/2023 Move XPLMI_SCHED_TICK to header
*       dd   09/12/2023 MISRA-C violation Rule 13.4 fixed
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

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**
 * @}
 * @endcond
 */

/************************** Function Prototypes ******************************/
static u8 XPlmi_IsTaskNonPeriodic(const XPlmi_Scheduler_t *SchedPtr,
	u32 TaskListIndex);

/************************** Variable Definitions *****************************/
static XPlmi_Scheduler_t Sched;

/*****************************************************************************/

/******************************************************************************/
/**
* @brief	The function checks the specified task is active or not, returns
* 			corresponding status of the task
*
* @param   	SchedPtr is Scheduler pointer
* @param   	TaskListIndex is Task index
*
* @return	TRUE or FALSE based on the task active status
*
****************************************************************************/
static u8 XPlmi_IsTaskActive(XPlmi_Scheduler_t *SchedPtr, u32 TaskListIndex)
{
	u8 ReturnVal = (u8)FALSE;

	if (NULL == SchedPtr->TaskList[TaskListIndex].CustomerFunc) {
		goto END;
	}

	if (XPlmi_IsTaskNonPeriodic(SchedPtr, TaskListIndex) == (u8)TRUE) {
		if (SchedPtr->TaskList[TaskListIndex].TriggerTime <=
			SchedPtr->Tick) {
			ReturnVal = (u8)TRUE;
		}
	} else {
		if ((0U != SchedPtr->TaskList[TaskListIndex].Interval)
		&& (0U == (SchedPtr->Tick
		% SchedPtr->TaskList[TaskListIndex].Interval))) {
			/* Periodic */
			ReturnVal = (u8)TRUE;
		}
	}

END:
	return ReturnVal;
}

/******************************************************************************/
/**
* @brief	The function checks the specified task is periodic or not, returns
* 			corresponding periodicity status.
*
* @param    SchedPtr is the Scheduler pointer
* @param    TaskListIndex is the Task index
*
* @return	TRUE or FALSE based on the task peridocity status
*
****************************************************************************/
static u8 XPlmi_IsTaskNonPeriodic(const XPlmi_Scheduler_t *SchedPtr,
	u32 TaskListIndex)
{
	u8 ReturnVal = (u8)FALSE;

	if (SchedPtr->TaskList[TaskListIndex].Type == XPLMI_NON_PERIODIC_TASK) {
		ReturnVal = (u8)TRUE;
	}

	return ReturnVal;
}

/******************************************************************************/
/**
* @brief	The function initializes scheduler and returns the
* 			initialization status.
*
* @return
* 			- None
*
****************************************************************************/
void XPlmi_SchedulerInit(void)
{
	u8 Idx;

	/* Disable all the tasks */
	for (Idx = 0U; Idx < XPLMI_SCHED_MAX_TASK; Idx++) {
		Sched.TaskList[Idx].Interval = 0U;
		Sched.TaskList[Idx].CustomerFunc = NULL;
	}

	Sched.LastTimerTick = XPlmi_GetTimerValue();
	Sched.Tick = 0U;
}

/******************************************************************************/
/**
* @brief	The function is scheduler handler and it is called at regular
* 			intervals based on configured interval. Scheduler handler checks
* 			and adds the user periodic task to PLM task queue.
*
* @param	Data - Not used currently. Added as a part of generic interrupt
* 			handler
*
* @return
* 			- None
*
****************************************************************************/
void XPlmi_SchedulerHandler(void *Data)
{
	u8 Idx;
	(void)Data;
	XPlmi_TaskNode *Task = NULL;

	Sched.LastTimerTick = XPlmi_GetTimerValue();
	Sched.Tick++;
	XPlmi_UtilRMW(PMC_PMC_MB_IO_IRQ_ACK, PMC_PMC_MB_IO_IRQ_ACK, 0x20U);
	for (Idx = 0U; Idx < XPLMI_SCHED_MAX_TASK; Idx++) {
		/**
		 * - Check if the task is active and has a valid Callback
		 */
		if (XPlmi_IsTaskActive(&Sched, Idx) == (u8)TRUE) {
			Task = Sched.TaskList[Idx].Task;
			/**
			 * - Skip the task, if its already present in the queue
			 */
			if (metal_list_is_empty(&Task->TaskNode) == (int)TRUE) {
				Task->State &= (u8)(~XPLMI_SCHED_TASK_MISSED);
				XPlmi_TaskTriggerNow(Task);
			} else {
				/**
				 * - Check if a module has registered ErrorFunc for the task and
				 * the previously scheduled task is executed or not
				 */
				if ((Sched.TaskList[Idx].ErrorFunc != NULL) &&
					((Task->State & (u8)(XPLMI_SCHED_TASK_MISSED)) ==
							(u8)0x0U)) {
					/**
					 * - Update scheduler task state with task missed flag
					 */
					Task->State |= (u8)XPLMI_SCHED_TASK_MISSED;
					/**
					 * - Call the task specific ErrorFunc if
					 *   previously scheduled task is not executed
					 */
					Sched.TaskList[Idx].ErrorFunc(XPLMI_ERR_SCHED_TASK_MISSED);
				}
			}
			/**
			 * - Remove the task from scheduler if it is non-periodic
			 */
			if (Sched.TaskList[Idx].Type == XPLMI_NON_PERIODIC_TASK) {
				Sched.TaskList[Idx].OwnerId = 0U;
				Sched.TaskList[Idx].CustomerFunc = NULL;
				Sched.TaskList[Idx].ErrorFunc = NULL;
			}
		}
	}
	XPlmi_WdtHandler();

	return;
}

/******************************************************************************/
/**
* @brief	The function adds user periodic task to scheduler queue. The user
* 			shall call this function to register their scheduler task.
*
* @param	OwnerId Id of the owner, used while removing the task.
* @param	CallbackFn callback function that should be called
* @param	ErrorFunc error function to be called when task does not execute
* 			on scheduled interval
* @param	MilliSeconds For Periodic tasks, it's the Periodicity of the task.
*			For Non-Periodic tasks, it's the delay after which task has to
*			be scheduled. Value should be in multiples of 10ms
* @param	Priority is the priority of the task
* @param	Data is the pointer to the private data of the task
* @param	TaskType is the type of Task (periodic or non-periodic)
*
* @return
* 			- XST_SUCCESS if scheduler task is registered properly.
* 			- XPLMI_ERR_INVALID_TASK_TYPE on invalid task type.
* 			- XPLMI_ERR_INVALID_TASK_PERIOD on invalid task period.
* 			- XPLMI_ERR_TASK_EXISTS if task is already present.
* 			- XPLM_ERR_TASK_CREATE if failed to create the task.
*
****************************************************************************/
int XPlmi_SchedulerAddTask(u32 OwnerId, XPlmi_Callback_t CallbackFn,
		XPlmi_ErrorFunc_t ErrorFunc, u32 MilliSeconds,
		TaskPriority_t Priority, void *Data, u8 TaskType)
{
	int Status = XST_FAILURE;
	XPlmi_PerfTime ExtraTime;
	u8 Idx;
	u32 TriggerTime = 0U;
	XPlmi_TaskNode *Task = NULL;
	u8 TaskNodePresent = (u8)FALSE;

	if ((TaskType !=  XPLMI_PERIODIC_TASK) &&
		(TaskType != XPLMI_NON_PERIODIC_TASK)) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_INVALID_TASK_TYPE, 0);
		goto END;
	}

	if ((TaskType == XPLMI_PERIODIC_TASK) && (MilliSeconds == 0U)) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_INVALID_TASK_PERIOD, 0);
		goto END;
	}
    Task = XPlmi_GetTaskInstance(CallbackFn, Data, XPLMI_INVALID_INTR_ID);
	if (Task != NULL) {
		if (metal_list_is_empty(&Task->TaskNode) == (int)FALSE) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_TASK_EXISTS, 0);
			goto END;
		}
		else {
			TaskNodePresent = (u8)TRUE;
		}
	}

	/**
	 * - Get the Next Free Task Index
	 */
	for (Idx = 0U; Idx < XPLMI_SCHED_MAX_TASK; Idx++) {
		if (NULL == Sched.TaskList[Idx].CustomerFunc) {
			/**
			 * - Add Interval as a factor of TICK_MILLISECONDS
			 */
			Sched.TaskList[Idx].Interval = MilliSeconds / XPLMI_SCHED_TICK;
			Sched.TaskList[Idx].OwnerId = OwnerId;
			Sched.TaskList[Idx].CustomerFunc = CallbackFn;
			Sched.TaskList[Idx].ErrorFunc = ErrorFunc;
			Sched.TaskList[Idx].Type = TaskType;
			Sched.TaskList[Idx].Data = Data;
			/**
			 * - Create a new task if task instance not found
			 */
			if (TaskNodePresent == (u8)FALSE) {
				Task = XPlmi_TaskCreate(Priority, CallbackFn, Data);
			}

			if (Task == NULL) {
				Status = XPlmi_UpdateStatus(XPLM_ERR_TASK_CREATE, 0);
				XPlmi_Printf(DEBUG_GENERAL, "Task Creation "
						"Err:0x%x\n\r", Status);
				goto END;
			}
			Task->IntrId = XPLMI_INVALID_INTR_ID;
			Sched.TaskList[Idx].Task = Task;
			if (TaskType != XPLMI_PERIODIC_TASK) {
				microblaze_disable_interrupts();
				XPlmi_MeasurePerfTime(Sched.LastTimerTick, &ExtraTime);
				if (Sched.Tick == 0U) {
					ExtraTime.TPerfMs %= XPLMI_SCHED_TICK;
				}
				TriggerTime = Sched.Tick +
					   (((u32)ExtraTime.TPerfMs + MilliSeconds) /
					   XPLMI_SCHED_TICK);
				microblaze_enable_interrupts();
			}
			Sched.TaskList[Idx].TriggerTime = TriggerTime;
			Status = XST_SUCCESS;
			break;
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
* @brief	The function removes scheduler task from scheduler queue.
* 			The function called by the user for deregistering the scheduler
* 			task.
*
* @param	OwnerId Id of the owner, removed only if matches the ownerid
*			while adding the task.
* @param	CallbackFn callback function that is given while adding.
* @param	MilliSeconds Periodicity of the task given while adding.
* @param	Data is the pointer to the private data of the task
*
* @return
* 			- XST_SUCCESS on success and error code on failure
*
****************************************************************************/
int XPlmi_SchedulerRemoveTask(u32 OwnerId, XPlmi_Callback_t CallbackFn,
		u32 MilliSeconds, const void *Data)
{
	int Status = XST_FAILURE;
	u8 Idx;
	u32 TaskCount = 0U;

	/* Find the Task Index */
	for (Idx = 0U; Idx < XPLMI_SCHED_MAX_TASK; Idx++) {
		if ((CallbackFn == Sched.TaskList[Idx].CustomerFunc) &&
			(Sched.TaskList[Idx].OwnerId == OwnerId) &&
			(Sched.TaskList[Idx].Data == Data) &&
			((Sched.TaskList[Idx].Interval ==
				(MilliSeconds / XPLMI_SCHED_TICK)) ||
				(0U == MilliSeconds))) {
			Sched.TaskList[Idx].Interval = 0U;
			Sched.TaskList[Idx].OwnerId = 0U;
			Sched.TaskList[Idx].CustomerFunc = NULL;
			Sched.TaskList[Idx].Data = NULL;
			microblaze_disable_interrupts();
			if (metal_list_is_empty(&Sched.TaskList[Idx].Task->TaskNode) ==
				(int)FALSE) {
				metal_list_del(&Sched.TaskList[Idx].Task->TaskNode);
			}
			microblaze_enable_interrupts();
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
