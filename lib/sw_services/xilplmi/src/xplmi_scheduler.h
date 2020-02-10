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


#ifndef XPLMI_SCHEDULER_H
#define XPLMI_SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"


#define XPLMI_SCHED_MAX_TASK	10U

/* Values for TaskPtr->Status */
#define XPLMI_TASK_STATUS_TRIGGERED	0x5AFEC0C0U
#define XPLMI_TASK_STATUS_DISABLED	0x00000000U

#define PMC_PMC_MB_IO_IRQ_ACK			(0xF028003CU)
#define PMC_PMC_MB_IO_IRQ_ACK_SHIFT   (0x5U)
#define PMC_PMC_MB_IO_IRQ_ACK_WIDTH   (0x1U)
#define PMC_PMC_MB_IO_IRQ_ACK_MASK    (0X0000020U)

typedef int (*XPlmi_Callback_t)(void *);

struct XPlmi_Task_t{
	u32 Interval;
	u32 OwnerId;
	int Status;
	XPlmi_Callback_t CustomerFunc;
};

typedef struct {
	struct XPlmi_Task_t TaskList[XPLMI_SCHED_MAX_TASK];
	int TaskCount;
	int PitBaseAddr;
	int Tick;
	int Enabled;
} XPlmi_Scheduler_t ;

int XPlmi_SchedulerInit(void);
int XPlmi_SchedulerStart(XPlmi_Scheduler_t *SchedPtr);
int XPlmi_SchedulerStop(XPlmi_Scheduler_t *SchedPtr);
void XPlmi_SchedulerHandler(void *Data);
int XPlmi_SchedulerAddTask(u32 OwnerId, XPlmi_Callback_t CallbackFn,
			   u32 MilliSeconds);
int XPlmi_SchedulerRemoveTask(u32 OwnerId, XPlmi_Callback_t CallbackFn,
			      u32 MilliSeconds);
int XPlmi_IsTaskActive(XPlmi_Scheduler_t *SchedPtr, u32 TaskListIndex);
int XPlmi_IsTaskNonPeriodic(XPlmi_Scheduler_t *SchedPtr, u32 TaskListIndex);

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_SCHEDULER_H_ */
