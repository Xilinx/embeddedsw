/**************************************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtask.h
 * @addtogroup Overview
 * @{
 *
 * This file contains declarations for xtask.c file.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   10/11/23 Initial release
 * 1.1   ma   02/02/24 Update task functionality to support periodic tasks
 *       ma   03/16/24 Added error codes at required places
 *       ma   07/08/24 Add task based approach at queue level
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XTASK_H
#define XTASK_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xlinklist.h"

/************************************ Constant Definitions ***************************************/
#define XTASK_MAX               (32U) /**< Maximum tasks allowed to be created */
#define XTASK_PRIORITIES        (16U) /**< Number of task priorities allowed */

#define XTASK_NUM_BITS_IN_U32   (32U)   /**< Maximum bits in unsigned int */

/************************************** Type Definitions *****************************************/

/** Type definition for task handler */
typedef s32 (*XTask_Handler_t)(void *Arg);

/** This structure contains task metadata */
typedef struct {
	u32 Priority; /**< Priority of the task */
	u32 Delay; /**< Task delay */
	XLinkList TaskNode; /**< Linked list for the task */
	XTask_Handler_t TaskHandler; /**< Handler for the task */
	void *PrivData; /**< Private data of the task */
	u32 Interval; /**< Task interval. For non-periodic tasks, interval should be 0 */
} XTask_TaskNode;

/** This structure contains the events list */
typedef struct {
	u32 Tasks[(XTASK_MAX + XTASK_NUM_BITS_IN_U32 - 1U) / XTASK_NUM_BITS_IN_U32]; /**< Task events */
} XTask_TaskEvent;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
void XTask_Init(void);
XTask_TaskNode *XTask_Create(u32 Priority, XTask_Handler_t TaskHandler, void *PrivData,
			     u32 Interval);
void XTask_Delete(XTask_TaskNode *Task);
void XTask_TriggerNow(XTask_TaskNode *Task);
s32 XTask_TriggerAfterDelay(XTask_TaskNode *Task, u32 Delay);
s32 XTask_TriggerOnEvent(XTask_TaskNode *Task, XTask_TaskEvent *Event);
void XTask_EventNotify(XTask_TaskEvent *Event);
u32 XTask_DelayTime(XTask_TaskNode *Task);
void XTask_DispatchLoop(void);

/************************************ Variable Definitions ***************************************/
/** Current time in us */
extern u32 TaskTimeNow;

#ifdef __cplusplus
}
#endif

#endif  /* XTASK_H */
/** @} */
