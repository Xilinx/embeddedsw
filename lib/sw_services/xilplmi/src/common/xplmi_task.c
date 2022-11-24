/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_task.c
*
* This file contains code Task handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/06/2019 Initial release
* 1.01  kc   07/16/2019 Added PERF macro to print task times
* 1.02  kc   02/17/2020 Task dispatcher updated with round robin from FCFS
*       bsv  04/04/2020 Code clean up
* 1.03  kc   07/28/2020 WDT support added to set PLM live status
*       bm   10/14/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
* 1.04  kc   11/30/2020 Disable interrupts while updating shared data
* 1.05  skd  03/12/2021 Added provision to skip scheduling a task if it is
*                       already present in queue
*       ma   03/24/2021 Reduced minimum digits of time stamp decimals to 3
*       skd  03/31/2021 Adding non periodic tasks even if a task
*                       with the same handler exists, to ensure no
*                       interrupt task handlers get missed
*       bm   04/03/2021 Move task creation out of interrupt context
*       bsv  04/08/2021 Moved Task Time prints to DEBUG_DETAILED to reduce
*                       logs on console
* 1.06  td   07/08/2021 Fix doxygen warnings
*       ma   07/12/2021 Minor updates to task related code
*       ma   08/05/2021 Add separate task for each IPI channel
* 1.07  bm   02/04/2022 Fix race condition in task dispatch loop
*       bsv  03/05/2022 Fix exception while deleting two consecutive tasks of
*                       same priority
*       bsv  03/11/2022 Restore race condition fix that got disturbed by
*                       previous patch
* 1.08  ng   11/11/2022 Updated doxygen comments
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_task.h"
#include "xplmi_debug.h"
#include "xplmi_wdt.h"
#include "mb_interface.h"
#include "xplmi_proc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static struct metal_list TaskQueue[XPLMI_TASK_PRIORITIES];

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief	This function creates the task and initializes its fields with
 * the user parameters.
 *
 * @param	Priority Priority of the task
 * @param	Handler function pointer to the task handler
 * @param	PrivData Private Data to be passed with task handler
 *
 * @return	Pointer to the task node structure
 *
 *****************************************************************************/
XPlmi_TaskNode* XPlmi_TaskCreate(TaskPriority_t Priority,
	int (*Handler)(void *Arg), void *PrivData)
{
	XPlmi_TaskNode *Task = NULL;

	if(Handler == NULL) {
		goto END;
	}

	/* Get a free task node */
	Task = XPlmi_GetTaskInstance(NULL, NULL, XPLMI_INVALID_INTR_ID);
	if (Task == NULL) {
		XPlmi_Printf(DEBUG_GENERAL, "Task creation failed \n\r");
		goto END;
	}
	Task->Priority = Priority;
	Task->Delay = 0U;
	metal_list_init(&Task->TaskNode);
	Task->Handler = Handler;
	Task->PrivData = PrivData;
	Task->State = (u8)0x0U;

END:
	return Task;
}

/*****************************************************************************/
/**
 * @brief	This function returns the instance of the task with matching
 * handler and private data or with matching interrupt id
 *
 * @param	Handler is pointer to the task handler
 * @param	PrivData is argument to be passed to the task handler
 * @param	IntrId is the interrupt id associated with the task
 *
 * @return	Instance of the task in case of a match, NULL otherwise
 *
 *****************************************************************************/
XPlmi_TaskNode* XPlmi_GetTaskInstance(int (*Handler)(void *Arg),
		const void *PrivData, const u32 IntrId)
{
	XPlmi_TaskNode *Task = NULL;
	static XPlmi_TaskNode Tasks[XPLMI_TASK_MAX];
	u8 Index;

	for (Index = 0U; Index < XPLMI_TASK_MAX; Index++) {
		Task = &Tasks[Index];
		if (IntrId != XPLMI_INVALID_INTR_ID) {
			/* Return task whose interrupt id is matching */
			if ((Task->Handler != NULL) &&
			    (Task->IntrId == IntrId)) {
				break;
			}
		}
		else {
			/* Assign free task node */
			if ((Task->Handler == Handler) &&
			    (Task->PrivData == PrivData)) {
				break;
			}
		}
		Task = NULL;
	}

	return Task;
}

/*****************************************************************************/
/**
 * @brief	This function adds the task to the task queue so that it can be
 * triggered based on its priority.
 *
 * @param	Task Pointer to the task node
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_TaskTriggerNow(XPlmi_TaskNode *Task)
{
	Xil_AssertVoid(Task->Handler != NULL);
	if (metal_list_is_empty(&Task->TaskNode) != (int)FALSE) {
		metal_list_add_tail(&TaskQueue[Task->Priority],
			&Task->TaskNode);
	}
}

/*****************************************************************************/
/**
 * @brief	This function initializes the task queues list.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_TaskInit(void)
{
	u32 Index;

	XPlmi_Printf(DEBUG_DETAILED, "%s\n\r", __func__);
	/* Initialize the list pointers */
	for (Index = 0U; Index < XPLMI_TASK_PRIORITIES; Index++) {
		metal_list_init(&TaskQueue[Index]);
	}
}

/*****************************************************************************/
/**
 * @brief	This function will be checking for tasks in the queue based on the
 * priority. After calling every task handlers, next high priority task will
 * be called.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_TaskDispatchLoop(void)
{
	int Status = XST_FAILURE;
	struct metal_list *Node[XPLMI_TASK_PRIORITIES];
	XPlmi_TaskNode *Task;
	u32 Index;
#ifdef PLM_DEBUG_DETAILED
	u64 TaskStartTime;
	XPlmi_PerfTime PerfTime = {0U};
#endif

	XPlmi_Printf(DEBUG_DETAILED, "%s\n\r", __func__);
	for (Index = 0U; Index < XPLMI_TASK_PRIORITIES; Index++) {
		Node[Index] = &TaskQueue[Index];
	}

	while (TRUE) {
		Task = NULL;
		XPlmi_SetPlmLiveStatus();

		microblaze_disable_interrupts();
		/**
		 * Perform Priority based task handling
		 */
		for (Index = 0U; Index < XPLMI_TASK_PRIORITIES; Index++) {
			/**
			 * If no pending tasks are present, go to sleep
			 */
			if (metal_list_is_empty(&TaskQueue[Index]) != (int)FALSE) {
				XPlmi_Printf(DEBUG_DETAILED,
				 "No pending tasks in Priority%d Queue\n\r",
				 Index);
				continue;
			} else {
				/* Skip the first element as it
				 * is not proper task
				 */
				if ((metal_list_is_empty(Node[Index]) != (int)FALSE) ||
					(Node[Index] == &TaskQueue[Index])) {
					Node[Index] = TaskQueue[Index].next;
				}
				/**
				 * - Get the next task in round robin
				 */
				Task = metal_container_of(Node[Index],
					XPlmi_TaskNode, TaskNode);
				Node[Index] = Node[Index]->next;
				break;
			}
		}
		if (Task != NULL) {
#ifdef PLM_DEBUG_DETAILED
			/* Call the task handler */
			TaskStartTime = XPlmi_GetTimerValue();
#endif
			Xil_AssertVoid(Task->Handler != NULL);
			metal_list_del(&Task->TaskNode);
			microblaze_enable_interrupts();
			Status = Task->Handler(Task->PrivData);
#ifdef PLM_DEBUG_DETAILED
			XPlmi_MeasurePerfTime(TaskStartTime, &PerfTime);
			XPlmi_Printf(DEBUG_PRINT_PERF, "%u.%03u ms: Task Time\n\r",
				(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
#endif
			if (Status != XST_SUCCESS) {
				XPlmi_ErrMgr(Status);
			}
			continue;
		}

		/**
		 * Goto sleep when all queues are empty
		 */
		XPlmi_Printf(DEBUG_DETAILED,
			"No pending tasks..Going to sleep\n\r");
		mb_sleep();
		microblaze_enable_interrupts();
	}
}
