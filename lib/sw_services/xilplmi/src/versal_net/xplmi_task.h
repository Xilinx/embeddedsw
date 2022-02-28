/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_task.h
*
* This is the file which contains command execution code.
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
* 1.03  skd  03/31/2021 Adding non periodic tasks even if a task
*                       with the same handler exists, to ensure no
*                       interrupt task handlers get missed
*       bm   04/03/2021 Move task creation out of interrupt context
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLMI_TASK_H
#define XPLMI_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_assert.h"
#include "mb_interface.h"
#include "xil_types.h"
#include "xstatus.h"
#include "list.h"
#include "xplmi_proc.h"

/************************** Constant Definitions *****************************/
#define XPLMI_TASK_MAX			(64U)
#define XPLMI_TASK_PRIORITIES		(2U)
#define XPLMI_INVALID_INTR_ID		(0xFFFFFFFFU)

typedef enum {
        XPLM_TASK_PRIORITY_0 = 0,
        XPLM_TASK_PRIORITY_1, /**< 1 */
} TaskPriority_t;

/**************************** Type Definitions *******************************/
typedef struct XPlmi_TaskNode XPlmi_TaskNode;

struct XPlmi_TaskNode {
    TaskPriority_t Priority;
    u32 IntrId;
    u32 Delay;
    struct metal_list TaskNode;
    int (*Handler)(void * PrivData);
    void * PrivData;
    u8 InQueue;
    u8 IsPersistent;
};

/***************** Macros (Inline Functions) Definitions *********************/
/* Compute offset of a field within a structure. */
#define metal_offset_of(structure, member)		\
	((uintptr_t) &(((structure *) 0U)->member))

/* Compute pointer to a structure given a pointer to one of its fields. */
#define metal_container_of(ptr, structure, member)	\
	(void *)((uintptr_t)(ptr) - metal_offset_of(structure, member))

/************************** Function Prototypes ******************************/
XPlmi_TaskNode * XPlmi_TaskCreate(TaskPriority_t Priority,
	int (*Handler)(void *Arg), void * PrivData);
void XPlmi_TaskTriggerNow(XPlmi_TaskNode * Task);
void XPlmi_TaskInit(void);
void XPlmi_TaskDispatchLoop(void);
void XPlmi_TaskDelete(XPlmi_TaskNode *Task);
XPlmi_TaskNode* XPlmi_GetTaskInstance(int (*Handler)(void *Arg),
	const void *PrivData, const u32 IntrId);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* end of XPLMI_TASK_H */
