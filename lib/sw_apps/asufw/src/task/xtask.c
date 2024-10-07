/**************************************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtask.c
 *
 * This file contains task related code.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   10/11/23 Initial release
 * 1.1   ma   01/04/24 Added code to select command to be executed based on priority
 * 1.2   ma   02/02/24 Update task functionality to support periodic tasks
 *       ma   03/16/24 Added error codes at required places
 *       ma   04/30/24 Included dependent header file
 *       ma   07/08/24 Add task based approach at queue level
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xtask_apis Task Related APIs
* @{
*/
/*************************************** Include Files *******************************************/
#include "xtask.h"
#include "xasufw_status.h"
#include "xil_printf.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
static XTask_TaskNode TaskList[XTASK_MAX];
static XLinkList TaskQueue[XTASK_PRIORITIES];
static u32 NextDispatchTime;
/** Current time in us */
u32 TaskTimeNow = 0U;

/*************************************************************************************************/
/**
 * @brief	This function is called during boot up which will initialize the task queues.
 *
 *************************************************************************************************/
void XTask_Init(void)
{
	u32 Priority;

	for (Priority = 0U; Priority < XTASK_PRIORITIES; Priority++) {
		XLinkList_Init(&TaskQueue[Priority]);
	}
}

/*************************************************************************************************/
/**
 * @brief	This function creates the task and initializes its fields with the user parameters.
 *
 * @param	Priority	Priority of the task.
 * @param	TaskHandler	Function pointer to the task handler.
 * @param	PrivData	Private data to be passed with task handler.
 * @param	Interval	Task interval if the task need to run periodically.
 * 				For non-periodic tasks interval need to be 0.
 *
 * @return
 * 	- Returns pointer to the task node structure if the task is created successfully.
 * 	- Returns NULL if the task creation fails.
 *
 *************************************************************************************************/
XTask_TaskNode *XTask_Create(u32 Priority, XTask_Handler_t TaskHandler, void *PrivData,
			     u32 Interval)
{
	XTask_TaskNode *Task = NULL;
	u32 Idx;

	if ((Priority >= XTASK_PRIORITIES) || (TaskHandler == NULL)) {
		goto END;
	}

	/** Find the empty slot for task in TaskList. */
	for (Idx = 0U; Idx < XTASK_MAX; Idx++) {
		if (TaskList[Idx].TaskHandler == NULL) {
			break;
		}
	}

	/** If the maximum allowed tasks are already created, do not create a new task. */
	if (Idx >= XTASK_MAX) {
		xil_printf("Task create failed: too many tasks created\n");
		goto END;
	}

	Task = &TaskList[Idx];
	Task->Priority = Priority;
	Task->Delay = 0U;
	XLinkList_Init(&Task->TaskNode);
	Task->TaskHandler = TaskHandler;
	Task->PrivData = PrivData;
	Task->Interval = Interval;

	/** Update task delay and next dispatch time. */
	if (Interval > 0U) {
		(void)XTask_TriggerAfterDelay(Task, Interval);
	}

END:
	return Task;
}

/*************************************************************************************************/
/**
 * @brief	This function deletes the given task from the task queue.
 *
 * @param	Task	Task to be deleted from task list.
 *
 *************************************************************************************************/
void XTask_Delete(XTask_TaskNode *Task)
{
	/** If the list is not empty, remove the task from the task queue. */
	if (XLinkList_IsEmpty(&Task->TaskNode) != (u8)TRUE) {
		XLinkList_RemoveItem(&Task->TaskNode);
	}

	/** Reset task structure members with default values. */
	Task->Delay = 0U;
	Task->TaskHandler = NULL;
	Task->PrivData = NULL;
	Task->Interval = 0U;
}

/*************************************************************************************************/
/**
 * @brief	This function adds the task to the task queue so that it can be triggered based
 * 		on its priority. This function is called when the task needs to be executed.
 *
 * @param	Task	Task pointer to be executed in the task list.
 *
 *************************************************************************************************/
void XTask_TriggerNow(XTask_TaskNode *Task)
{
	/* Trigger the task only if the given task handler is valid */
	if (Task->TaskHandler != NULL) {
		/* Add the task to the TaskQueue only if it is not already added */
		if (XLinkList_IsEmpty(&Task->TaskNode) == (u8)TRUE) {
			XLinkList_AddItemLast(&Task->TaskNode, &TaskQueue[Task->Priority]);
		}
	}
}

/*************************************************************************************************/
/**
 * @brief	This function is called when the task need to be executed after the given delay. It
 * 		will update the Delay field of the given task structure with the given delay
 * 		value. When this function is called, it will update the NextDispatchTime variable
 * 		with given input Delay value if it is greater than task delay value.
 *
 * @param	Task	Pointer to the task node structure.
 * @param	Delay	Delay after which the task needs to be triggered.
 *
 * @return
 * 	- Returns XASUFW_SCCESS on successful execution of the function.
 * 	- XASUFW_TASK_INVALID_HANDLER, if task handler is NULL.
 *
 *************************************************************************************************/
s32 XTask_TriggerAfterDelay(XTask_TaskNode *Task, u32 Delay)
{
	s32 Status = XASUFW_FAILURE;

	/* TODO: Validate Delay with max value */
	if (Task->TaskHandler == NULL) {
		Status = XASUFW_TASK_INVALID_HANDLER;
		goto END;
	}

	Task->Delay = Delay;

	/**
	 * Make given task as next delayed task to be executed, if the tasks delay period is less than
	 * the delay period of next delayed task execution
	 */
	if ((NextDispatchTime == 0U) || (NextDispatchTime > Task->Delay)) {
		NextDispatchTime = Delay;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function updates the given task's event structure with the input event to
 * 		trigger the task when the particular event occurs.
 *
 * @param	Task	Pointer to the task node structure.
 * @param	Event	Pointer to the event node structure.
 *
 * @return
 * 	- Returns XASUFW_SCCESS on successful execution of the function.
 * 	- XASUFW_TASK_INVALID_HANDLER, if task handler is NULL.
 *
 *************************************************************************************************/
s32 XTask_TriggerOnEvent(const XTask_TaskNode *Task, XTask_TaskEvent *Event)
{
	s32 Status = XASUFW_FAILURE;
	u32 Idx = Task - TaskList;

	if (Task->TaskHandler == NULL) {
		Status = XASUFW_TASK_INVALID_HANDLER;
		goto END;
	}

	/* Update given task event structure for the given task */
	Event->Tasks[Idx / XTASK_NUM_BITS_IN_U32] |=
		((u32)(1U) << (Idx & (XTASK_NUM_BITS_IN_U32 - 1U)));

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is the notify function for the given event. This function needs to be
 * 		called on the occurrence of the event after which the dependent tasks needs to be
 * 		triggered.
 *
 *
 * @param	Event	Pointer to the event node structure.
 *
 *************************************************************************************************/
void XTask_EventNotify(XTask_TaskEvent *Event)
{
	u32 Idx;
	u32 TaskEventBitVal;

	/**
	 * Parse through the task event structure to check if the given event has occured or not.
	 * If the event has occurred, update the event structure and trigger the task to be exected.
	 */
	for (Idx = 0U; Idx < XTASK_MAX; Idx++) {
		TaskEventBitVal = 1U << (Idx & (XTASK_NUM_BITS_IN_U32 - 1U));
		if ((Event->Tasks[Idx / XTASK_NUM_BITS_IN_U32] & TaskEventBitVal) != 0U) {
			Event->Tasks[Idx / XTASK_NUM_BITS_IN_U32] &= ~TaskEventBitVal;
			XTask_TriggerNow(&TaskList[Idx]);
		}
	}
}

/*************************************************************************************************/
/**
 * @brief	This function returns the Delay of the given task node structure.
 *
 * @param	Task	Pointer to the task node structure.
 *
 * @return
 * 	- Returns delay of the given task.
 *
 *************************************************************************************************/
u32 XTask_DelayTime(const XTask_TaskNode *Task)
{
	return Task->Delay;
}

/*************************************************************************************************/
/**
 * @brief   This function is the task dispatch loop which is called after the initialization is
 * done and it does the following functionality
 *          1. It will check for both delayed tasks and the tasks in the task queue in infinite
 *              loop.
 *          2. For delayed tasks, it will update task delay and the next dispatch time as current
 *              time is updated whenever scheduler handler is called.
 *          3. If the task delay is completed, it will trigger that task so that it is added to the
 *              task queue to be executed next.
 *          4. And, this function will check the task queues according to the priority from highest
 *              to lowest and executes them in a loop.
 *          5. It will remove the task from the task queue before calling its handler.
 *          6. If no tasks are present in the queue, the processor will enter into sleep mode and
 *              wakes up when any interrupt occurs and repeats the above steps.
 *
 *************************************************************************************************/
void XTask_DispatchLoop(void)
{
	s32 Status = XASUFW_FAILURE;
	u32 LastDispatchTime = TaskTimeNow;
	u32 Idx;
	XTask_TaskNode *Task;
	const XLinkList *HTask;
	XLinkList *LTask;
	u32 Priority;
	u32 DeltaTime;

	xil_printf("In task dispatch loop\r\n");

	/**
	 * This function should never return
	 * This loop checks for delayed tasks to be triggered and triggers them once the delay time is
	 * completed. And also executes tasks in the task queue based on priority.
	 */
	for (;;) {
		/* Update DeltaTime and LastDispatchTime every time while entering this loop */
		DeltaTime = TaskTimeNow - LastDispatchTime;
		LastDispatchTime += DeltaTime;

		/* TODO: Dispatch if any pending interrupts are present */

		/* Check delayed tasks to be triggered */
		if ((NextDispatchTime != 0U) && (DeltaTime != 0U)) {
			NextDispatchTime = 0U;

			for (Idx = 0U; Idx < XTASK_MAX; Idx++) {
				Task = &TaskList[Idx];

				if (Task->Delay > 0U) {
					/* Update the task delay based on DeltaTime */
					if (Task->Delay > DeltaTime) {
						Task->Delay -= DeltaTime;
					} else {
						Task->Delay = 0U;
					}

					if (Task->Delay > 0U) {
						/* Update NextDispatchTime if the task delay is greater than 0 */
						if ((NextDispatchTime == 0U) || (NextDispatchTime > Task->Delay)) {
							NextDispatchTime = Task->Delay;
						}
					} else {
						/* Trigger the task if the task delay is complete */
						XTask_TriggerNow(Task);
						/* Update delay for periodic tasks */
						if (Task->Interval > 0U) {
							(void)XTask_TriggerAfterDelay(Task, Task->Interval);
						}
					}
				}
			}
		}

		/* Parse through the task queue priority list and get the next task to be executed */
		Task = NULL;
		for (Priority = 0U; Priority < XTASK_PRIORITIES; Priority++) {
			HTask = &TaskQueue[Priority];
			LTask = HTask->Next;
			if (HTask != LTask) {
				Task = XLinkList_ContainerOf(LTask, XTask_TaskNode, TaskNode);
				break;
			}
		}

		/* Remove the task from the task queue and call the handler */
		if (Task != NULL) {
			XLinkList_RemoveItem(&Task->TaskNode);
			Status = Task->TaskHandler(Task->PrivData);
			if (XASUFW_SUCCESS != Status) {
				xil_printf("Task execution failed with error: 0x%x\r\n", Status);
			}
			continue;
		}

		/* Nothing to do */
		/* TODO: Wait for interrupts */
	}
}
/** @} */
