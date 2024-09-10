/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_sharedmem.h
 * @addtogroup Overview
 * @{
 *
 * This file contains declarations for xasufw_sharedmem.c file in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   01/02/24 Initial release
 *       ma   04/18/24 Moved command handling related functions to xasufw_cmd.c
 *       ma   07/08/24 Add task based approach at queue level
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASUFW_SHAREDMEM_H
#define XASUFW_SHAREDMEM_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasufw_ipi.h"
#include "xasu_sharedmem.h"
#include "xtask.h"

/************************************ Constant Definitions ***************************************/
/** Queue Unique ID related defines */
#define XASUFW_IPI_MASK_SHIFT					(16U) /**< IPI mask shift in Queue UniqueID */

#define XASUFW_MAX_CHANNELS_SUPPORTED			(8U) /**< Maximum channels supported */

/************************************** Type Definitions *****************************************/
/** This is a shared memory for all channels */
typedef struct {
	XAsu_ChannelMemory ChannelMemory[XASUFW_MAX_CHANNELS_SUPPORTED]; /**< Channel memories */
} XAsufw_SharedMemory;

/** This structure contains information of the communication channel */
typedef struct {
	u16 IpiBitMask; /**< Bit mask of the IPI channel to enable interrupts */
	u8 P0QueuePriority; /**< P0 Queue Priority of the channel */
	u8 P1QueuePriority; /**< P1 Queue Priority of the channel */
	u32 Reserved; /**< Reserved */
} XAsufw_CommChannel;

/** This structure contains information of all communication channels */
typedef struct {
	u32 CommChannelInfoHeader; /**< Communication channel information header */
	u32 NumOfIpiChannels; /**< Number of IPI channels to be enabled */
	XAsufw_CommChannel Channel[XASUFW_MAX_IPI_CHANNELS]; /**< Communication channels information */
} XAsufw_CommChannelInfo;

/** This structure contains P0 and P1 queue tasks and queue task handler required info */
typedef struct {
	XTask_TaskNode *P0QueueTask; /**< P0 queue task pointer */
	u32 P0QueueBufIdx; /**< P0 queue previous buffer index */
	XTask_TaskNode *P1QueueTask; /**< P1 queue task pointer */
	u32 P1QueueBufIdx; /**< P1 queue previous buffer index */
} XAsufw_QueueTasks;

/** This structure contains information about all channels tasks */
typedef struct {
	XAsufw_QueueTasks Channel[XASUFW_MAX_IPI_CHANNELS]; /**< Queue task info of all channels */
} XAsufw_ChannelTasks;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_SharedMemoryInit(void);
void XAsufw_TriggerQueueTask(u32 IpiMask);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_SHAREDMEM_H */
/** @} */
