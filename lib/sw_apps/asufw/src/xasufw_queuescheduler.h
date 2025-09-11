/**************************************************************************************************
* Copyright (c) 2024 - 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_queuescheduler.h
 *
 * This file contains declarations for xasufw_queuescheduler.c file in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   01/02/24 Initial release
 *       ma   04/18/24 Moved command handling related functions to xasufw_cmd.c
 *       ma   07/08/24 Add task based approach at queue level
 *       ss   09/26/24 Fixed doxygen comments
 * 1.1   ma   12/12/24 Added support for DMA non-blocking wait
 *       rmv  07/30/25 Added declaration for XAsufw_GetSubsysIdFromIpiMask() function
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_QUEUESCHEDULER_H_
#define XASUFW_QUEUESCHEDULER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasufw_ipi.h"
#include "xasu_sharedmem.h"
#include "xtask.h"

/************************************ Constant Definitions ***************************************/
/* Queue Unique ID related defines */
#define XASUFW_IPI_BITMASK_SHIFT		(16U) /**< IPI bit mask shift value in Queue Unique ID */
#define XASUFW_QUEUE_TASK_PRIVDATA_RSVD_MASK	(0xFFU) /**< Queue task PrivData reserved mask */
#define XASUFW_MAX_CHANNELS_SUPPORTED	(8U) /**< Maximum channels supported */
#define XASUFW_RESP_DATA_OFFSET			(2U) /**< Response data offset in response buffer */

#define XASUFW_INVALID_SUBSYS_ID		(0xFFFFFFFFU) /**< Invalid subsystem ID */

/** @} */
/************************************** Type Definitions *****************************************/
/** This structure is for shared memory of all channels. */
typedef struct {
	XAsu_ChannelMemory ChannelMemory[XASUFW_MAX_CHANNELS_SUPPORTED]; /**< Channel memories */
} XAsufw_SharedMemory;

/** This structure contains P0 and P1 queue tasks and queue task handler required info. */
typedef struct {
	XTask_TaskNode *P0QueueTask; /**< P0 queue task pointer */
	u32 P0QueueBufIdx; /**< P0 queue previous buffer index */
	XTask_TaskNode *P1QueueTask; /**< P1 queue task pointer */
	u32 P1QueueBufIdx; /**< P1 queue previous buffer index */
} XAsufw_QueueTasks;

/** This structure contains information about all the channel's tasks. */
typedef struct {
	XAsufw_QueueTasks Channel[XASU_MAX_IPI_CHANNELS]; /**< Queue task info of all channels */
} XAsufw_ChannelTasks;

/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/** This define calculates the structure member address from Item and structure Type */
#define XAsufw_GetRespBuf(Item, Type, Member)    \
	((Type *)(((char *)(Item) - offsetof(Type, Item)) + offsetof(Type, Member)))
/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
void XAsufw_ChannelConfigInit(void);
void XAsufw_TriggerQueueTask(u32 IpiMask);
u32 XAsufw_GetSubsysIdFromIpiMask(u32 IpiMask);
u32 XAsufw_GetIpiMask(u32 ChannelIndex);
s32 XAsufw_CheckPreemption(u32 CurrentReqId, u32 OwnerReqId);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_QUEUESCHEDULER_H_ */
/** @} */
