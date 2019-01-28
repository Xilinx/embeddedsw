/******************************************************************************
* Copyright (C) 2018 Xilinx, Inc. All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_task.c
*
* This file contains the task related code for PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  kc   07/12/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_task.h"
#include "xplm_startup.h"

/************************** Constant Definitions *****************************/
#define XPLM_MB_MSR_IE_MASK			(0x2U)
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
struct metal_list EvQueue[XPLM_TASK_PRIORITIES];
/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief This function overwrites the metal function that posts the event
 * to the queue. In this case, we adding the task to the priority
 * queue based on the prioroty assigned.
 *
 * @param	Event Pointer to the metal event structure that was
 *		registered.
 *
 * @retur	metal events defines as present in event.h
 *****************************************************************************/
int _metal_event_post(struct metal_event *Event)
{
	XPlmi_Printf(DEBUG_DETAILED, "%s\n\r", __func__);
	metal_list_add_tail(&EvQueue[Event->priority], &Event->node);
	return 0;
}

/*****************************************************************************/
/**
 * This function registers the startup events for PMC CDO processing and PDI
 * loading
 *
 * @param	None
 *
 * @return	Status as defined in xplm_status.h
 *****************************************************************************/
int XPlm_TaskInit(void )
{
	u32 Index;
	int Status;

	XPlmi_Printf(DEBUG_DETAILED, "%s\n\r", __func__);

	/* Initialize the list pointers */
	for (Index = 0; Index < XPLM_TASK_PRIORITIES; Index++)
	{
		metal_list_init(&EvQueue[Index]);
	}

	/* Register events */
	Status = XPlm_AddStartUpTasks();

	return Status;
}

/*****************************************************************************/
/**
 * This function will be checking for tasks in the queue based on the
 * priority. After calling every task handlers, next high priority task will
 * be called.
 *
 * @param	None
 *
 * @return	None
 *****************************************************************************/
void XPlm_TaskDispatchLoop(void )
{
	u32 Index;
	struct metal_list *Node;
	struct metal_event *Event;
	int Status;
	u64 TaskStartTime;

	XPlmi_Printf(DEBUG_DETAILED, "%s\n\r", __func__);

	while (1)
	{
		/** Priority based task handling */
		for (Index=0; Index<XPLM_TASK_PRIORITIES; Index++)
		{
			/** if no pending tasks are present, go to sleep */
			if (metal_list_is_empty(&EvQueue[Index]))
			{
				XPlmi_Printf(DEBUG_DETAILED, "No pending tasks in"
					   " Priority%d Queue\n\r", Index);
				continue;
			}

			/** Start time of the task  */
			TaskStartTime = XPlm_GetTimerValue();

			/** Call the task handler */
			Node = EvQueue[Index].next;
			Event = metal_container_of(Node,
						   struct metal_event, node);
			Status = Event->hd.func(Event, Event->hd.arg);
			/** delete the task that is handled */
			if (Status == METAL_EVENT_HANDLED)
			{
				metal_event_pop(Event);
			}

			/** Start time of the task  */
			XPlm_MeasurePerfTime(TaskStartTime);
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Task Time \n\r");
		}
#if 0
		/**
		 * TODO Need to check what is the better way
		 * to check all the Queues are empty
		 */
		XPlmi_Printf(DEBUG_INFO, "No pending Events..Going to sleep\n\r");
		mb_sleep();
#endif
		/*
		 * FIXME: Currently, IE bit is not being set when
		 * microblaze_enable_interrupts() API is called. Remove this code
		 * once issue is root caused.
		 */
		if ( (mfmsr() & XPLM_MB_MSR_IE_MASK) != XPLM_MB_MSR_IE_MASK) {
			XPlmi_Printf(DEBUG_INFO, "IE bit in MSR is not set. "
					"Setting the same to receive interrupts\n\r");
			mtmsr(mfmsr() | XPLM_MB_MSR_IE_MASK);
		}
	}

}
