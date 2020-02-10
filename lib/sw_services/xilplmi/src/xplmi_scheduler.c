/******************************************************************************
*
* Copyright (C) 2019-2020 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
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
* 1.01  kc   01/20/2020 Added APIs for removing the task
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_scheduler.h"
#include "xplmi_task.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLMI_SCHED_TICK	(10U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static XPlmi_Scheduler_t Sched;

/*****************************************************************************/

/******************************************************************************/
/**
*
* The function checks the specified task is active or not, returns
* corresponding status of the task.
* @param    scheduler pointer
* @param    task index.
*
* @return	TRUE or FALSE based on the task active status.
****************************************************************************/
int XPlmi_IsTaskActive(XPlmi_Scheduler_t *SchedPtr, u32 TaskListIndex)
{
	int ReturnVal = FALSE;

	/* Periodic */
	if ((0U != SchedPtr->TaskList[TaskListIndex].Interval)
		&& (NULL != SchedPtr->TaskList[TaskListIndex].CustomerFunc)
		&& (0U == (SchedPtr->Tick
			% SchedPtr->TaskList[TaskListIndex].Interval))) {
		ReturnVal = TRUE;
	} else if ((0U == SchedPtr->TaskList[TaskListIndex].Interval)
		&& (NULL != SchedPtr->TaskList[TaskListIndex].CustomerFunc)) {
		/* Non-Periodic */
		ReturnVal = TRUE;
	} else {
		/** Do Nothing */
	}

	return ReturnVal;
}

/******************************************************************************/
/**
*
* The function checks the specified task is periodic or not, returns
* corresponding periodicity status
* @param    scheduler pointer
* @param    task index.
*
* @return	TRUE or FALSE based on the task peridocity status
****************************************************************************/
int XPlmi_IsTaskNonPeriodic(XPlmi_Scheduler_t *SchedPtr, u32 TaskListIndex)
{
	int ReturnVal = FALSE;

	if ((0U == SchedPtr->TaskList[TaskListIndex].Interval)
		&& (NULL != SchedPtr->TaskList[TaskListIndex].CustomerFunc)) {
		ReturnVal = TRUE;
	} else {
		/** Do Nothing */
	}

	return ReturnVal;
}

/******************************************************************************/
/**
*
* The function initializes scheduler and returns the initialization status
* @param    None.
*
* @return	XST_SUCCESS if Scheduler initializes without any errors
****************************************************************************/
int XPlmi_SchedulerInit(void)
{
	u32 Idx;
	int Status = XST_FAILURE;

	/* Disable all the tasks */
	for (Idx = 0U; Idx < XPLMI_SCHED_MAX_TASK; Idx++) {
		Sched.TaskList[Idx].Interval = 0U;
		Sched.TaskList[Idx].CustomerFunc = NULL;
		Sched.TaskList[Idx].Status = XPLMI_TASK_STATUS_DISABLED;
	}

	Sched.Enabled = FALSE;
	Sched.PitBaseAddr = 0x0;
	Sched.Tick = 0U;

	/* Successfully completed init */
	Status = XST_SUCCESS;
	return Status;
}

/******************************************************************************/
/**
*
* The function starts the scheduler and updates start status.
* @param    schduler pointer
*
* @return	XST_SUCCESS if scheduler is started successfully
****************************************************************************/
int XPlmi_SchedulerStart(XPlmi_Scheduler_t *SchedPtr)
{
	int Status = XST_FAILURE;

	if (SchedPtr == NULL) {
		goto END;
	} else {
		SchedPtr->Enabled = TRUE;
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/******************************************************************************/
/**
*
* The function stops the scheduler and updates stop status.
* @param    scheduler pointer
*
* @return	XST_SUCCESS if schedulaer is stopped successfully
****************************************************************************/
int XPlmi_SchedulerStop(XPlmi_Scheduler_t *SchedPtr)
{
	SchedPtr->Enabled = FALSE;
	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* The function is scheduler handler and it is called at regular intervals
* based on configured interval. Scheduler handler check and adds the user
* periodic task to PLM task queue
* @param	Data - Not used currently. Added as a part of generic interrupt
*               handler
* @return	None
****************************************************************************/
void XPlmi_SchedulerHandler(void *Data)
{
	u32 Idx;
	int Status = XST_FAILURE;
	XPlmi_TaskNode *Task;
	(void)Data;

	Sched.Tick++;
	XPlmi_UtilRMW(PMC_PMC_MB_IO_IRQ_ACK, PMC_PMC_MB_IO_IRQ_ACK, 0x20);
	for (Idx = 0U; Idx < XPLMI_SCHED_MAX_TASK; Idx++) {
		/* Check if the task is triggered and has a valid Callback */
		if (XPlmi_IsTaskActive(&Sched, Idx) == TRUE) {
			/* Add the Task to the PLM Task Queue */
			Task = XPlmi_TaskCreate(XPLM_TASK_PRIORITY_1,
					Sched.TaskList[Idx].CustomerFunc, 0U);
			if (Task == NULL) {
				Status = XPLMI_UPDATE_STATUS(XPLM_ERR_TASK_CREATE, 0x0);
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
	END:
	return;
}

/******************************************************************************/
/**
*
* The function adds user periodic task to scheduler queue.The user shall
* call this funtion to register their scheduler task
* @param	OwnerId Id of the owner, used while removing the task.
* @param	CallbackFn callback function that should be called
* @param	MilliSeconds Periodicity of the task. If Zero, task is added
*               once. Value should be in multiples of 10ms.
*
* @return	XST_SUCCESS if scheduler task is registered properly
****************************************************************************/
int XPlmi_SchedulerAddTask(u32 OwnerId, XPlmi_Callback_t CallbackFn,
			   u32 MilliSeconds)
{
	u32 Idx;
	int Status = XST_FAILURE;

	/* Get the Next Free Task Index */
	for (Idx = 0U; Idx < XPLMI_SCHED_MAX_TASK; Idx++) {
		if (NULL == Sched.TaskList[Idx].CustomerFunc) {
			break;
		}
	}

	/* Check if we have reached Max Task limit */
	if (XPLMI_SCHED_MAX_TASK == Idx) {
		goto END;
	}

	/* Add Interval as a factor of TICK_MILLISECONDS */
	Sched.TaskList[Idx].Interval = MilliSeconds/XPLMI_SCHED_TICK;
	Sched.TaskList[Idx].OwnerId = OwnerId;
	Sched.TaskList[Idx].CustomerFunc = CallbackFn;
	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
*
* The function removes scheduler task from scheduler queue. The funtion called
* by the user for deregistering the scheduler task.
* @param	OwnerId Id of the owner, removed only if matches the ownerid
*               while adding the task.
* @param	MilliSeconds Periodicity of the task given while adding.
* @param	CallbackFn callback function that is given while adding.
* @return	XST_SUCCESS if handlers are deregistered properly
****************************************************************************/
int XPlmi_SchedulerRemoveTask(u32 OwnerId, XPlmi_Callback_t CallbackFn,
			      u32 MilliSeconds)
{
	u32 Idx;
	u32 TaskCount = 0U;

	/*Find the Task Index */
	for (Idx = 0U; Idx < XPLMI_SCHED_MAX_TASK; Idx++) {
		if ((CallbackFn == Sched.TaskList[Idx].CustomerFunc) &&
		    (Sched.TaskList[Idx].OwnerId == OwnerId) &&
		    ((Sched.TaskList[Idx].Interval ==
		                (MilliSeconds/XPLMI_SCHED_TICK)) ||
				(0U == MilliSeconds))) {
			Sched.TaskList[Idx].Interval = 0U;
			Sched.TaskList[Idx].OwnerId = 0U;
			Sched.TaskList[Idx].CustomerFunc = NULL;
			TaskCount++;
		}
	}

	XPlmi_Printf(DEBUG_DETAILED, "%s: Removed %lu tasks\r\n",
			__func__, TaskCount);

	return ((TaskCount > 0U) ? XST_SUCCESS : XST_FAILURE);
}
