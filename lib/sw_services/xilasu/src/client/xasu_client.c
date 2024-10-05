/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_client.c
 *
 * This file contains the ASU client initialization and generic queue management functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  06/03/24 Initial release
 *       ma   07/17/24 Update P0, P1 Queue addresses and set IsCmdPresent to TRUE before triggering
 *                     the IPI interrupt to ASU
 *       ss   08/13/24 Changed XAsu_ClientInit function prototype and Initialized mailbox in
 *                     XAsu_ClientInit() API
 *       ss   09/19/24 Added XAsu_CheckAsufwPrsntBit() API
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_client_info Client APIs AND Error Codes
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_client.h"
#include "xasu_status.h"

/************************************ Constant Definitions ***************************************/
#define XASU_QUEUE_BUFFER_FULL          0xFFU       /**< To indicate queue full state */
#define XASU_CLIENT_READY               0xFFFFFFFFU /**< To indicate Client is ready */
#define XASU_TARGET_IPI_INT_MASK        1U          /**< ASU IPI interrupt mask */

#define ASU_GLOBAL_BASEADDR             (0xEBF80000U) /**< ASU GLOBAL register base address */
#define ASU_GLOBAL_GLOBAL_CNTRL         (ASU_GLOBAL_BASEADDR + 0x00000000U) /**< ASU GLOBAL CNTRL
                                                                             register address */

#define ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK       0x10U          /**< ASU FW Present mask
                                                                              value */
#define XASU_ASUFW_BIT_CHECK_TIMEOUT_VALUE	0xFFFFFU	/**< ASUFW check timoeout value */

/************************************** Type Definitions *****************************************/
/**
 * This typedef contains all the parameters required to manage the client library
 * Also it holds the shared memory queue index details
 */
typedef struct {
	XMailbox *MailboxPtr;
	XAsu_ChannelMemory *ChannelMemoryPtr;
	XAsu_QueueInfo P0Queue;
	XAsu_QueueInfo P1Queue;
	u32 IsReady;
} XAsu_Client;
/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static XAsu_Client *XAsu_GetClientInstance(void);
static u32 XAsu_IsChannelQueueFull(XAsu_QueueInfo *QueueInfo);
static s32 XAsu_SendIpi(void);
static void XAsu_DoorBellToClient(void *CallBackRef);
static s32 XAsu_CheckAsufwPrsntBit(void);

/************************************ Variable Definitions ***************************************/
static XAsu_CommChannelInfo *CommChannelInfo = (XAsu_CommChannelInfo *)(UINTPTR)
	XASU_RTCA_COMM_CHANNEL_INFO_ADDR; /** All IPI channels information received from user
						configuration */
static XMailbox MailboxInstance;        /**< Variable to Mailbox instance */
static volatile u32 RecvDone = FALSE;	/**< Done flag */

/*************************************************************************************************/
/**
 * @brief	This function initializes the client instance.
 *
 * @param	BaseAddress	Base address of the IPI channel assigned to APU/RPU/PL.
 *
 * @return
 * 		- XST_SUCCESS, On successful initialization.
 * 		- XST_FAILURE, On failure.
 *,
 *************************************************************************************************/
s32 XAsu_ClientInit(u32 BaseAddress)
{
	s32 Status = XST_FAILURE;
	XAsu_Client *ClientInstancePtr = XAsu_GetClientInstance();
	u32 ChannelIdx;

	/* If already initialized returns success as no initialization is needed */
	if (ClientInstancePtr->IsReady == XASU_CLIENT_READY) {
		Status = XST_SUCCESS;
		goto END;
	}

	/** Check for ASUFW present bit. */
	Status = XAsu_CheckAsufwPrsntBit();
	if (Status != XST_SUCCESS) {
		Status = XASU_ASUFW_NOT_PRESENT;
		goto END;
	}

	/** Initialize mailbox instance. */
	Status = (s32)XMailbox_Initialize(&MailboxInstance, BaseAddress);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Map the IPI shared memory of the channel */
	for (ChannelIdx = 0U; ChannelIdx < CommChannelInfo->NumOfIpiChannels; ++ChannelIdx) {
		if (MailboxInstance.Agent.IpiInst.Config.BitMask ==
		    CommChannelInfo->Channel[ChannelIdx].IpiBitMask) {
			break;
		}
	}

	if (ChannelIdx == CommChannelInfo->NumOfIpiChannels) {
		Status = XASU_IPI_CONFIG_NOT_FOUND;
		goto END;
	}

	/* Assign channel shared memory */
	ClientInstancePtr->ChannelMemoryPtr = XASU_CHANNEL_MEMORY_BASEADDR +
					(XASU_CHANNEL_MEMORY_OFFSET * ChannelIdx);

	ClientInstancePtr->MailboxPtr = &MailboxInstance;
	ClientInstancePtr->P0Queue.ChannelQueue = &ClientInstancePtr->ChannelMemoryPtr->P0ChannelQueue;
	ClientInstancePtr->P1Queue.ChannelQueue = &ClientInstancePtr->ChannelMemoryPtr->P1ChannelQueue;
	ClientInstancePtr->P0Queue.NextFreeIndex = 0U;
	ClientInstancePtr->P1Queue.NextFreeIndex = 0U;

	ClientInstancePtr->IsReady = XASU_CLIENT_READY;

	Status = XMailbox_SetCallBack(ClientInstancePtr->MailboxPtr, XMAILBOX_RECV_HANDLER,
				      XAsu_DoorBellToClient, ClientInstancePtr->MailboxPtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function updates the queue buffer status to notify the request is present and
 * 		generates a door bell to ASU.
 *
 * @param	QueueInfo	 Pointer to the XAsu_QueueInfo structure.
 *
 * @return
 * 	- XST_SUCCESS upon successful update.
 * 	- XST_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
s32 XAsu_UpdateQueueBufferNSendIpi(XAsu_QueueInfo *QueueInfo)
{
	s32 Status = XST_FAILURE;
	XAsu_ChannelQueueBuf *QueueBufPtr;

	/** Validate input parameters. */
	if (QueueInfo == NULL) {
		goto END;
	}
	/* Get Queue memory */
	QueueBufPtr = XAsu_GetChannelQueueBuf(QueueInfo);
	if (QueueBufPtr == NULL) {
		goto END;
	}

	QueueBufPtr->RespBufStatus = 0x0U;
	QueueBufPtr->ReqBufStatus = XASU_COMMAND_IS_PRESENT;

	if (QueueInfo->NextFreeIndex == (XASU_MAX_BUFFERS - 1U)) {
		/* TODO to point to zero index upon free */
		QueueInfo->NextFreeIndex = 0U;
	} else {
		QueueInfo->NextFreeIndex++;
	}

	/** Set IsCmdPresent to TRUE to indicate the command is present in the queue. */
	QueueInfo->ChannelQueue->IsCmdPresent = TRUE;

	/** Place an IPI request to ASU. */
	Status = XAsu_SendIpi();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	while (!RecvDone);

	RecvDone = FALSE;
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the pointer of the next free queue buffer of the requested
 * 		priority queue.
 *
 * @param	QueueInfo	 Pointer to the XAsu_QueueInfo structure.
 *
 * @return
 * 	- Pointer to XAsufw_ChannelQueueBuf.
 * 	- Otherwise, returns NULL.
 *
 *************************************************************************************************/
XAsu_ChannelQueueBuf *XAsu_GetChannelQueueBuf(XAsu_QueueInfo *QueueInfo)
{
	XAsu_ChannelQueueBuf *QueueBuf = NULL;

	if (QueueInfo == NULL) {
		goto END;
	}

	/* Check if Queue is full */
	if (XAsu_IsChannelQueueFull(QueueInfo) != TRUE) {
		QueueBuf =  &QueueInfo->ChannelQueue->ChannelQueueBufs[QueueInfo->NextFreeIndex];
	}

END:
	return QueueBuf;

}

/*************************************************************************************************/
/**
 * @brief	This function returns either requested queue is full or not.
 *
 * @param	QueueInfo	 Pointer to the XAsu_QueueInfo structure.
 *
 * @return
 * 	- TRUE - if queue is full.
 * 	- Otherwise, returns FALSE.
 *
 *************************************************************************************************/
static u32 XAsu_IsChannelQueueFull(XAsu_QueueInfo *QueueInfo)
{
	return (QueueInfo->NextFreeIndex <= (XASU_MAX_BUFFERS - 1U)) ? FALSE : TRUE;
}

/*************************************************************************************************/
/**
 * @brief 	This function returns the pointer to the XAsu_QueueInfo structure of the provided
 * 		priority.
 *
 * @param	QueuePriority	Priority of the queue.
 *
 * @return
 * 	- Pointer to XAsu_QueueInfo.
 * 	- NULL if invalid input.
 *
 *************************************************************************************************/
XAsu_QueueInfo *XAsu_GetQueueInfo(u32 QueuePriority)
{
	XAsu_Client *ClientInstancePtr = XAsu_GetClientInstance();
	XAsu_QueueInfo *QueueInfo = NULL;

	if (QueuePriority == XASU_PRIORITY_HIGH) {
		QueueInfo = &ClientInstancePtr->P0Queue;
	} else if (QueuePriority == XASU_PRIORITY_LOW) {
		QueueInfo = &ClientInstancePtr->P1Queue;
	} else {
		QueueInfo = NULL;
	}

	return QueueInfo;
}

/*************************************************************************************************/
/**
 * @brief	This function sends an IPI request to ASU.
 *
 * @return
 * 	- XST_SUCCESS - If the IPI is sent successfully.
 * 	- XST_FAILURE - If there is a failure.
 *
 *************************************************************************************************/
static s32 XAsu_SendIpi(void)
{
	s32 Status = XST_FAILURE;
	XAsu_Client *ClientInstancePtr = XAsu_GetClientInstance();

	if (ClientInstancePtr->IsReady != XASU_CLIENT_READY) {
		goto END;
	}

	Status = (s32)XMailbox_Send(ClientInstancePtr->MailboxPtr,
				    XASU_TARGET_IPI_INT_MASK, FALSE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns an instance pointer of ASU client interface.
 *
 *
 * @return
 * 	- It returns pointer to the XAsu_Client.
 *
 *************************************************************************************************/
static XAsu_Client *XAsu_GetClientInstance(void)
{
	static XAsu_Client ClientInstance = {0U};

	return &ClientInstance;
}

/****************************************************************************/
/**
 * @brief	This function polls for the response from ASUFW.
 *
 * @param	CallBackRef	Call back reference pointer.
 *
 ****************************************************************************/
static void XAsu_DoorBellToClient(void *CallBackRef)
{
	(void)CallBackRef;
	RecvDone = TRUE;
}

/*************************************************************************************************/
/**
 * @brief	This function returns ASUFW application present status.
 *
 * @return
 * 	- XST_SUCCESS - upon success.
 * 	- XST_FAILURE - If there is a failure.
 *
 *************************************************************************************************/
static s32 XAsu_CheckAsufwPrsntBit(void)
{
	s32 Status = XST_FAILURE;
	s32 Timeout = 0U;

	for (Timeout = 0U; Timeout != XASU_ASUFW_BIT_CHECK_TIMEOUT_VALUE; Timeout++) {
		if ((Xil_In32(ASU_GLOBAL_GLOBAL_CNTRL) & ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK)
			== ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK) {
				Status = XST_SUCCESS;
				goto END;
		}
		usleep(1U);
	}

END:
	return Status;
}
/** @} */
