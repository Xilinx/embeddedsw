/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.04  ma   07/12/2021 Minor updates to task related code
*       bsv  07/16/2021 Fix doxygen warnings
*       ma   08/05/2021 Add separate task for each IPI channel
*       bsv  08/15/2021 Replaced enums with macros
* 1.05  bsv  03/05/2022 Fix exception while deleting two consecutive tasks of
*                       same priority
* 1.06  bm   01/03/2023 Create Secure Lockdown as a Critical Priority Task
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

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/
#define XPLMI_TASK_MAX			(72U)
#define XPLMI_TASK_PRIORITIES		(3U)
#define XPLMI_INVALID_INTR_ID		(0xFFFFFFFFU)


#define XPLMI_SCHED_TASK_MISSED				(0x1U)

#define XPLM_TASK_PRIORITY_CRITICAL	(0U)
#define XPLM_TASK_PRIORITY_0		(1U)
#define XPLM_TASK_PRIORITY_1		(2U)
#define TaskPriority_t u8

/**************************** Type Definitions *******************************/
typedef struct XPlmi_TaskNode XPlmi_TaskNode;

struct XPlmi_TaskNode {
    u8 Priority;
    u8 State;
    u32 IntrId;
    u32 Delay;
    struct metal_list TaskNode;
    int (*Handler)(void * PrivData);
    void * PrivData;
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
XPlmi_TaskNode* XPlmi_GetTaskInstance(int (*Handler)(void *Arg),
	const void *PrivData, const u32 IntrId);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* end of XPLMI_TASK_H */
