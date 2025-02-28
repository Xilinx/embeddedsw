/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_queuescheduler.c
 *
 * This file contains the shared memory code for IPI communication in ASUFW. This file provides
 * functionality for initializing IPI shared memory, validating the user configuration data,
 * creating queue level tasks and scheduling them when the commands are received from clients.
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
 * 1.1   ma   12/12/24 Added support for DMA non-blocking wait
 *       ma   02/19/25 Updated handling of same priority queue requests in round robin scheduling
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_queuescheduler.h"
#include "xasufw_memory.h"
#include "xasufw_cmd.h"
#include "xasufw_debug.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xil_util.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_SHARED_MEMORY_ADDRESS	0xEBE41000U /**< Reserved address in ASU DATA RAM  for
							channel buffers shared memory */
#define XASUFW_SHARED_MEMORY_SIZE	0x8000U /**< 32KB size for shared memory */

#define XASUFW_MAX_PRIORITIES_SUPPORTED	16U /**< Maximum queue priorities supported */

/**
 * Queue Unique ID or Queue Task private data related defines. Queue Unique ID is created with
 * the below data:
 *  Bits[31:16] - IPI channel bit mask.
 *  Bits[15:12] - Channel index (0 to 7).
 *  Bits[11:8] - Queue index (P0 or P1).
 *
 * ReqId uniquely identifies each request present in all the queues. ReqId is created in ASUFW
 * with below data:
 *  Bits[31:8] - Queue Unique ID.
 *  Bits[7:0] - Unique ID received in the command header from client.
 */
#define XASUFW_P0_QUEUE				0x0U /**< P0 Queue */
#define XASUFW_P1_QUEUE				0x1U /**< P1 Queue */
#define XASUFW_CHANNELINDEX_MASK	0xF000U /**< Channel index mask in Queue UniqueID */
#define XASUFW_CHANNELINDEX_SHIFT	12U /**< Channel index shift in Queue UniqueID */
#define XASUFW_QUEUEINDEX_MASK		0xF00U /**< P0/P1 Queue index mask in Queue UniqueID */
#define XASUFW_QUEUEINDEX_SHIFT		8U /**< Queue index shift in Queue UniqueID */

/**
 * When there are pending requests present in the queue, ASUFW triggers the task with the delay
 * of 100ms.
 */
#define XASUFW_QUEUE_TASK_DELAY_TIME	100U /**< Queue task delay time. */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
static inline void XAsufw_InterruptRemoteProc(u32 IpiMask);

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_QueueTaskHandler(void *Arg);

/************************************ Variable Definitions ***************************************/
/* All channel's shared memory where the commands are received */
static XAsufw_SharedMemory *SharedMemory = (XAsufw_SharedMemory *)(UINTPTR)
	XASUFW_SHARED_MEMORY_ADDRESS;

/* All IPI channels information received from user configuration */
static XAsu_CommChannelInfo *CommChannelInfo = (XAsu_CommChannelInfo *)(UINTPTR)
	XASU_RTCA_COMM_CHANNEL_INFO_ADDR;

/* All channel's task related information */
static XAsufw_ChannelTasks CommChannelTasks = { 0U };

/*************************************************************************************************/
/**
 * @brief	This function triggers the IPI interrupt to the sender.
 *
 * @param	IpiMask		IPI Mask of the remote processor.
 *
 *************************************************************************************************/
static inline void XAsufw_InterruptRemoteProc(u32 IpiMask)
{
	XAsufw_WriteReg(IPI_ASU_TRIG, IpiMask);
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
static s32 XAsufw_QueueTaskHandler(void *Arg)
{
	s32 Status = XASUFW_FAILURE;
	u32 ChannelIndex = ((u32)Arg & XASUFW_CHANNELINDEX_MASK) >> XASUFW_CHANNELINDEX_SHIFT;
	u32 PxQueue = ((u32)Arg & XASUFW_QUEUEINDEX_MASK) >> XASUFW_QUEUEINDEX_SHIFT;
	u32 BufferIdx;
	XAsu_ChannelQueue *ChannelQueue;
	XAsu_ChannelQueueBuf *QueueBuf;
	u32 ReqId = 0x0U;
	XTask_TaskNode *Task = NULL;
	u32 IpiMask = 0U;

	/** Check which queue (P0/P1) has new command from client. */
	if (PxQueue == XASUFW_P0_QUEUE) {
		XAsufw_Printf(DEBUG_GENERAL, "Running P0 task of channel %d\r\n", ChannelIndex);
		ChannelQueue = &SharedMemory->ChannelMemory[ChannelIndex].P0ChannelQueue;
		BufferIdx = CommChannelTasks.Channel[ChannelIndex].P0QueueBufIdx;
	} else {
		XAsufw_Printf(DEBUG_GENERAL, "Running P1 task of channel %d\r\n", ChannelIndex);
		ChannelQueue = &SharedMemory->ChannelMemory[ChannelIndex].P1ChannelQueue;
		BufferIdx = CommChannelTasks.Channel[ChannelIndex].P1QueueBufIdx;
	}

	/** If no pending requests, exit the queue task. */
	if (ChannelQueue->ReqSent == ChannelQueue->ReqServed) {
		Status = XASUFW_SUCCESS;
		goto END;
	}

	/**
	 * Check all buffers for any command is present and validate and allocate resource if valid
	 * and update response after command is executed.
	 */
	for (; BufferIdx < XASU_MAX_BUFFERS; ++BufferIdx) {
		if ((ChannelQueue->ChannelQueueBufs[BufferIdx].ReqBufStatus == XASU_COMMAND_IS_PRESENT) ||
			(ChannelQueue->ChannelQueueBufs[BufferIdx].ReqBufStatus ==
				XASU_COMMAND_DMA_WAIT_COMPLETE)) {
			XAsufw_Printf(DEBUG_GENERAL, "Command is present. Channel: %d, Priority Queue: %d, "
				      "BufferIdx: %d\r\n", ChannelIndex, PxQueue, BufferIdx);
			QueueBuf = &ChannelQueue->ChannelQueueBufs[BufferIdx];
			/** Create a Request ID with Queue task private data and Req Unique ID from client. */
			ReqId = ((QueueBuf->ReqBuf.Header & XASU_UNIQUE_REQ_ID_MASK) >>
						XASU_UNIQUE_REQ_ID_SHIFT) | (u32)Arg;
			/**
			 * If the queue buffer's request buffer status is XASU_COMMAND_IS_PRESENT, validate
			 * the command, check if the required resources are available and allocate the
			 * resources.
			*/
			if (ChannelQueue->ChannelQueueBufs[BufferIdx].ReqBufStatus ==
				XASU_COMMAND_IS_PRESENT) {
				Status = XAsufw_ValidateCommand(&QueueBuf->ReqBuf);
				if (XASUFW_SUCCESS == Status) {
					XAsufw_Printf(DEBUG_GENERAL, "Validate command successful\r\n");
					Status = XAsufw_CheckAndAllocateResources(&QueueBuf->ReqBuf, ReqId);
				} else {
					XAsufw_Printf(DEBUG_GENERAL, "Validate command failed\r\n");
					/**
					 * TODO: Need to enhance this code to write the response only when
					 * invalid command is received or the access permissions fail.
					 * Currently, XAsufw_ValidateCommand only checks for invalid command.
					 *
					 * Update command status in the request queue and the response in
					 * response queue.
					 */
					QueueBuf->ReqBufStatus = XASU_COMMAND_EXECUTION_COMPLETE;
					XAsufw_CommandResponseHandler(&QueueBuf->ReqBuf, Status);
				}
			}

			/**
			 * If it is a new request, and if the command validation and resource allocation are
			 * successful, or if the command's non-blocking DMA operation is completed, the queue
			 * handler should be called.
			 */
			if (((ChannelQueue->ChannelQueueBufs[BufferIdx].ReqBufStatus ==
					XASU_COMMAND_IS_PRESENT) && (Status == XASUFW_SUCCESS)) ||
				(ChannelQueue->ChannelQueueBufs[BufferIdx].ReqBufStatus ==
					XASU_COMMAND_DMA_WAIT_COMPLETE)) {
				Status = XAsufw_CommandQueueHandler(QueueBuf, ReqId);
			}

			/** Increment the requests served by ASUFW if the command execution is complete. */
			if (ChannelQueue->ChannelQueueBufs[BufferIdx].ReqBufStatus ==
					XASU_COMMAND_EXECUTION_COMPLETE) {
				IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
				ChannelQueue->ReqServed++;
				/** Trigger interrupt to the sender as the response is ready. */
				XAsufw_InterruptRemoteProc(IpiMask);
			}
			break;
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

	/** If the requests served is not same as requests sent, trigger the queue task with delay. */
	if (ChannelQueue->ReqSent != ChannelQueue->ReqServed) {
		Task = XTask_GetInstance(Arg);
		if (Task != NULL) {
			XAsufw_Printf(DEBUG_DETAILED, "Pending requests are present in the queue.\r\n"
					"Triggering the queue task with delay.\r\n");
			Status = XTask_TriggerAfterDelay(Task, XASUFW_QUEUE_TASK_DELAY_TIME);
		}
	}

END:
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
		if ((SharedMemory->ChannelMemory[ChannelIdx].P0ChannelQueue.IsCmdPresent == XASU_TRUE) &&
			(CommChannelTasks.Channel[ChannelIdx].P0QueueTask != NULL)) {
			XTask_TriggerNow(CommChannelTasks.Channel[ChannelIdx].P0QueueTask);
			SharedMemory->ChannelMemory[ChannelIdx].P0ChannelQueue.IsCmdPresent = XASU_FALSE;
		}

		if ((SharedMemory->ChannelMemory[ChannelIdx].P1ChannelQueue.IsCmdPresent == XASU_TRUE) &&
			(CommChannelTasks.Channel[ChannelIdx].P1QueueTask != NULL)) {
			XTask_TriggerNow(CommChannelTasks.Channel[ChannelIdx].P1QueueTask);
			SharedMemory->ChannelMemory[ChannelIdx].P1ChannelQueue.IsCmdPresent = XASU_FALSE;
		}
	}
}

/*************************************************************************************************/
/**
 * @brief	This function initializes the IPI shared memory, validates the communication channel
 * configuration received as part of ASU CDO, creates queue level tasks based on the priority set
 * by the user and enables corresponding IPI channel interrupts.
 *
 *************************************************************************************************/
void XAsufw_ChannelConfigInit(void)
{
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
				      "from user configuration.\r\nIPI Bit Mask: 0x%x, "
				      "P0 Queue Priority: %d, P1 Queue Priority: %d\r\n",
				      CommChannelInfo->Channel[ChannelIndex].IpiBitMask,
				      CommChannelInfo->Channel[ChannelIndex].P0QueuePriority,
				      CommChannelInfo->Channel[ChannelIndex].P1QueuePriority);
			break;
		}
		/**
		 * Create P0 Queue Task of the channel corresponding to ChannelIndex. Task PrivData is
		 * nothing but Queue Unique ID.
		 */
		PrivData = (ChannelIndex << XASUFW_CHANNELINDEX_SHIFT) |
					(XASUFW_P0_QUEUE << XASUFW_QUEUEINDEX_SHIFT) |
					(CommChannelInfo->Channel[ChannelIndex]. IpiBitMask << XASUFW_IPI_BITMASK_SHIFT);
		CommChannelTasks.Channel[ChannelIndex].P0QueueTask = XTask_Create(
					CommChannelInfo->Channel[ChannelIndex].P0QueuePriority, XAsufw_QueueTaskHandler,
					(void *)PrivData, 0x0U);
		SharedMemory->ChannelMemory[ChannelIndex].P0ChannelQueue.IsCmdPresent = XASU_FALSE;
		CommChannelTasks.Channel[ChannelIndex].P0QueueBufIdx = 0U;

		/**
		 * Create P1 Queue Task of the channel corresponding to ChannelIndex. Task PrivData is
		 * nothing but Queue Unique ID.
		 */
		PrivData = (ChannelIndex << XASUFW_CHANNELINDEX_SHIFT) |
					(XASUFW_P1_QUEUE << XASUFW_QUEUEINDEX_SHIFT) |
					(CommChannelInfo->Channel[ChannelIndex]. IpiBitMask << XASUFW_IPI_BITMASK_SHIFT);
		CommChannelTasks.Channel[ChannelIndex].P1QueueTask = XTask_Create(
					CommChannelInfo->Channel[ChannelIndex].P1QueuePriority, XAsufw_QueueTaskHandler,
					(void *)PrivData, 0x0U);
		SharedMemory->ChannelMemory[ChannelIndex].P1ChannelQueue.IsCmdPresent = XASU_FALSE;
		CommChannelTasks.Channel[ChannelIndex].P1QueueBufIdx = 0U;

		/* Enable IPI interrupt from the channel */
		XAsufw_EnableIpiInterrupt(CommChannelInfo->Channel[ChannelIndex].IpiBitMask);
	}
}
/** @} */
