/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_sharedmem.c
 *
 * This file contains the shared memory code for IPI communication in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   01/02/24 Initial release
 *       ma   03/16/24 Added error codes at required places
 *       ma   04/18/24 Moved command handling related functions to xasufw_cmd.c
 *       ma   07/08/24 Add task based approach at queue level
 *       ma   07/23/24 Updated communication channel info address with RTCA address
 *       ss   09/26/24 Fixed doxygen comments
 *       ma   09/26/24 Removed static IPI configurations from code
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_sharedmem.h"
#include "xasufw_memory.h"
#include "xasufw_cmd.h"
#include "xasufw_debug.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_SHARED_MEMORY_ADDRESS	0xEBE41000U /**< Reserved address in ASU DATA RAM  for
							channel buffers shared memory */
#define XASUFW_SHARED_MEMORY_SIZE	0x8000U /**< 32KB size for shared memory */

#define XASUFW_MAX_PRIORITIES_SUPPORTED	16U /**< Maximum queue priorities supported */

/* Queue Unique ID related defines */
#define XASUFW_P0_QUEUE			0x0U /**< P0 Queue */
#define XASUFW_P1_QUEUE			0x1U /**< P1 Queue */
#define XASUFW_CHANNELINDEX_MASK	0xFU /**< Channel index mask in Queue UniqueID */
#define XASUFW_QUEUEINDEX_MASK		0xF0U /**< P0/P1 Queue index mask in Queue UniqueID */
#define XASUFW_QUEUEINDEX_SHIFT		4U /**< Queue index shift in Queue UniqueID */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_ValidateCommChannelUserConfig(void);

/************************************ Variable Definitions ***************************************/
/* All channel's shared memory where the commands are received */
static XAsufw_SharedMemory *SharedMemory = (XAsufw_SharedMemory *)(UINTPTR)
	XASUFW_SHARED_MEMORY_ADDRESS;

/* All IPI channels information received from user configuration */
static XAsu_CommChannelInfo *CommChannelInfo = (XAsu_CommChannelInfo *)(UINTPTR)
	XASU_RTCA_COMM_CHANNEL_INFO_ADDR;

/* All channel's task related information */
static XAsufw_ChannelTasks CommChannelTasks;

/*************************************************************************************************/
/**
 * @brief	This function validates communication channel user configuration, sorts channel
 * 		queues based on their priorities given by user and enables corresponding IPI
 * 		channel interrupts.
 *
 * @return
 * 	- XASUFW_SUCCESS, if the IPI user configuration is valid.
 * 	- XASUFW_INVALID_USER_CONFIG_RECEIVED, if the IPI user configuration is invalid.
 * 	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
s32 XAsufw_SharedMemoryInit(void)
{
	s32 Status = XASUFW_FAILURE;
	u32 ChannelIndex;

	Status = XAsufw_ValidateCommChannelUserConfig();
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_INVALID_USER_CONFIG_RECEIVED;
		XFIH_GOTO(END);
	}
	XAsufw_Printf(DEBUG_GENERAL, "User config parameters validated\r\n");

	/* Enable IPI interrupts from other channels */
	for (ChannelIndex = 0x0U; ChannelIndex < CommChannelInfo->NumOfIpiChannels; ++ChannelIndex) {
		XAsufw_EnableIpiInterrupt(CommChannelInfo->Channel[ChannelIndex].IpiBitMask);
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is the task handler for the channel queues.
 *
 * @param	Arg	Task private data.
 *
 * @return
 *	- XASUFW_SUCCESS, if the IPI commands are executed successfully.
 *	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
s32 XAsufw_QueueTaskHandler(void *Arg)
{
	s32 Status = XASUFW_FAILURE;
	u32 ChannelIndex = (u32)Arg & XASUFW_CHANNELINDEX_MASK;
	u32 PxQueue = ((u32)Arg & XASUFW_QUEUEINDEX_MASK) >> XASUFW_QUEUEINDEX_SHIFT;
	u32 BufferIdx;
	XAsu_ChannelQueue *ChannelQueue;
	XAsu_ChannelQueueBuf *QueueBuf;

	/** Check which queue has task whether P0/P1 queue. */
	if (PxQueue == XASUFW_P0_QUEUE) {
		XAsufw_Printf(DEBUG_GENERAL, "Running P0 task of channel %d\r\n", ChannelIndex);
		ChannelQueue = &SharedMemory->ChannelMemory[ChannelIndex].P0ChannelQueue;
		BufferIdx = CommChannelTasks.Channel[ChannelIndex].P0QueueBufIdx;
	} else {
		XAsufw_Printf(DEBUG_GENERAL, "Running P1 task of channel %d\r\n", ChannelIndex);
		ChannelQueue = &SharedMemory->ChannelMemory[ChannelIndex].P1ChannelQueue;
		BufferIdx = CommChannelTasks.Channel[ChannelIndex].P1QueueBufIdx;
	}

	/** Check all buffers for any command is present and validate and allocate resource if valid
		and update response after command is executed. */
	for (; BufferIdx < XASU_MAX_BUFFERS; ++BufferIdx) {
		if (ChannelQueue->ChannelQueueBufs[BufferIdx].ReqBufStatus == XASU_COMMAND_IS_PRESENT) {
			XAsufw_Printf(DEBUG_GENERAL, "Command is present. Channel: %d, Priority Queue: %d, "
				      "BufferIdx: %d\r\n", ChannelIndex, PxQueue, BufferIdx);
			QueueBuf = &ChannelQueue->ChannelQueueBufs[BufferIdx];
			Status = XAsufw_ValidateCommand(&QueueBuf->ReqBuf);
			if (XASUFW_SUCCESS == Status) {
				XAsufw_Printf(DEBUG_GENERAL, "Validate command successful\r\n");
				Status = XAsufw_CheckResources(&QueueBuf->ReqBuf, (u32)Arg);
				if (XASUFW_SUCCESS == Status) {
					Status = XAsufw_CommandQueueHandler(QueueBuf, (u32)Arg);
				}
			} else {
				XAsufw_Printf(DEBUG_GENERAL, "Validate command failed\r\n");
				/*
				 * TODO: Need to enhance this code to write the response only when
				 * invalid command is received or the access permissions fail.
				 * Currently, XAsufw_ValidateCommand only checks for invalid command
				*/
				/** Update command status in the request queue and the resopnse in
					response queue. */
				QueueBuf->ReqBufStatus = XASU_COMMAND_EXECUTION_COMPLETE;
				XAsufw_CommandResponseHandler(QueueBuf, Status);
			}
		}
	}

	/** Zeroize buffer index if max buffer index reached. */
	if (BufferIdx == XASU_MAX_BUFFERS) {
		BufferIdx = 0U;
	}

	/** Update buffer index for P0/P1 Queue. */
	if (PxQueue == XASUFW_P0_QUEUE) {
		CommChannelTasks.Channel[ChannelIndex].P0QueueBufIdx = BufferIdx;
	} else {
		CommChannelTasks.Channel[ChannelIndex].P1QueueBufIdx = BufferIdx;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks if P0 or P1 queue of the given IPI mask has the new commands
 * 		and triggers the corresponding Queue Task accordingly.
 *
 * @param   IpiMask	IPI mask on which the interrupt is received.
 *
 *************************************************************************************************/
void XAsufw_TriggerQueueTask(u32 IpiMask)
{
	u32 ChannelIdx;

	/** Get channel memory index from the IPI mask. */
	for (ChannelIdx = 0U; ChannelIdx < CommChannelInfo->NumOfIpiChannels; ++ChannelIdx) {
		if (IpiMask == CommChannelInfo->Channel[ChannelIdx].IpiBitMask) {
			break;
		}
	}

	/**
	 * Check which queue of the corresponding channel memory has new commands and trigger the
	 * task
	 */
	if (ChannelIdx != CommChannelInfo->NumOfIpiChannels) {
		if (SharedMemory->ChannelMemory[ChannelIdx].P0ChannelQueue.IsCmdPresent == XASU_TRUE) {
			XTask_TriggerNow(CommChannelTasks.Channel[ChannelIdx].P0QueueTask);
			SharedMemory->ChannelMemory[ChannelIdx].P0ChannelQueue.IsCmdPresent = XASU_FALSE;
		}

		if (SharedMemory->ChannelMemory[ChannelIdx].P1ChannelQueue.IsCmdPresent == XASU_TRUE) {
			XTask_TriggerNow(CommChannelTasks.Channel[ChannelIdx].P1QueueTask);
			SharedMemory->ChannelMemory[ChannelIdx].P1ChannelQueue.IsCmdPresent = XASU_FALSE;
		}
	}
}

/*************************************************************************************************/
/**
 * @brief	This function validates the communication channel configuration received as part of
 *		LPD CDO.
 *
 * @return
 * 	- XASUFW_SUCCESS, if the IPI user configuration is valid.
 * 	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
static s32 XAsufw_ValidateCommChannelUserConfig(void)
{
	s32 Status = XASUFW_FAILURE;
	u32 ChannelIndex;
	u32 PrivData;

	/* Validate IPI channel information */
	for (ChannelIndex = 0U; ChannelIndex < CommChannelInfo->NumOfIpiChannels; ++ChannelIndex) {
		if ((CommChannelInfo->Channel[ChannelIndex].IpiBitMask <= XASUFW_IPI_PMC_MASK) ||
		    (CommChannelInfo->Channel[ChannelIndex].IpiBitMask > XASUFW_IPI_NOBUF_6_MASK) ||
		    (CommChannelInfo->Channel[ChannelIndex].P0QueuePriority >=
		     XASUFW_MAX_PRIORITIES_SUPPORTED) ||
		    (CommChannelInfo->Channel[ChannelIndex].P1QueuePriority >=
		     XASUFW_MAX_PRIORITIES_SUPPORTED)) {
			XAsufw_Printf(DEBUG_INFO, "Invalid communication channel information received "
				      "from user configurtion.\r\nIPI Bit Mask: 0x%x, "
				      "P0 Queue Priority: %d, P1 Queue Priority: %d\r\n",
				      CommChannelInfo->Channel[ChannelIndex].IpiBitMask,
				      CommChannelInfo->Channel[ChannelIndex].P0QueuePriority,
				      CommChannelInfo->Channel[ChannelIndex].P1QueuePriority);
			break;
		}
		/** Create P0 Queue Task of the channel corresponding to ChannelIndex. */
		PrivData = ChannelIndex | (XASUFW_P0_QUEUE << XASUFW_QUEUEINDEX_SHIFT) |
			   (CommChannelInfo->Channel[ChannelIndex]. IpiBitMask << XASUFW_IPI_MASK_SHIFT);
		CommChannelTasks.Channel[ChannelIndex].P0QueueTask = XTask_Create(
					CommChannelInfo->Channel[ChannelIndex].P0QueuePriority, XAsufw_QueueTaskHandler,
					(void *)PrivData, 0x0U);
		SharedMemory->ChannelMemory[ChannelIndex].P0ChannelQueue.IsCmdPresent = XASU_FALSE;
		CommChannelTasks.Channel[ChannelIndex].P0QueueBufIdx = 0U;

		/** Create P1 Queue Task of the channel corresponding to ChannelIndex. */
		PrivData = ChannelIndex | (XASUFW_P1_QUEUE << XASUFW_QUEUEINDEX_SHIFT) |
			   (CommChannelInfo->Channel[ChannelIndex]. IpiBitMask << XASUFW_IPI_MASK_SHIFT);
		CommChannelTasks.Channel[ChannelIndex].P1QueueTask = XTask_Create(
					CommChannelInfo->Channel[ChannelIndex].P1QueuePriority, XAsufw_QueueTaskHandler,
					(void *)PrivData, 0x0U);
		SharedMemory->ChannelMemory[ChannelIndex].P1ChannelQueue.IsCmdPresent = XASU_FALSE;
		CommChannelTasks.Channel[ChannelIndex].P1QueueBufIdx = 0U;
	}

	if (ChannelIndex == CommChannelInfo->NumOfIpiChannels) {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}
/** @} */
