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
 *       vns  09/30/24 Added support for asynchronous communication
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
#include "sleep.h"
#include "xasu_def.h"

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
	u8 P0NextFreeIndex;
	u8 P1NextFreeIndex;
	u32 IsReady;
} XAsu_Client;

/**
 * This structure holds the call back reference of the requests to respond upon completion
 */
typedef struct {
	XAsuClient_ResponseHandler CallBackFuncPtr;  /**< Call Back function pointer */
	void *CallBackRefPtr;   /**< Call Back reference pointer */
	u8 *RespBufferPtr;		/**< Buffer to store the response data */
	u32 Size;				/**< Size of the response buffer */
} XAsu_RefToCallBack;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static XAsu_Client *XAsu_GetClientInstance(void);
static s32 XAsu_SendIpi(void);
static void XAsu_DoorBellToClient(void *CallBackRef);
static s32 XAsu_CheckAsufwPrsntBit(void);
static u8 XAsu_GenerateUniqueId(void);
static u8 XAsu_GetFreeIndex(u8 Priority);

/************************************ Variable Definitions ***************************************/
static XAsu_CommChannelInfo *CommChannelInfo = (XAsu_CommChannelInfo *)(UINTPTR)
	XASU_RTCA_COMM_CHANNEL_INFO_ADDR; /** All IPI channels information received from user
						configuration */
static XMailbox MailboxInstance;        /**< Variable to Mailbox instance */
static XAsu_RefToCallBack AsuCallBackRef[XASU_UNIQUE_ID_MAX]; /**< Entry of callback info */

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

	/* Validate instance pointer */
	if (ClientInstancePtr == NULL) {
		goto END;
	}

	/** If already initialized returns success as no initialization is needed */
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
	ClientInstancePtr->ChannelMemoryPtr = (XAsu_ChannelMemory *)(UINTPTR)(XASU_CHANNEL_MEMORY_BASEADDR +
					(XASU_CHANNEL_MEMORY_OFFSET * ChannelIdx));

	ClientInstancePtr->MailboxPtr = &MailboxInstance;
	ClientInstancePtr->P0NextFreeIndex = 0U;
	ClientInstancePtr->P1NextFreeIndex = 0U;

	ClientInstancePtr->IsReady = XASU_CLIENT_READY;

	Status = XMailbox_SetCallBack(ClientInstancePtr->MailboxPtr, XMAILBOX_RECV_HANDLER,
				      XAsu_DoorBellToClient, ClientInstancePtr->MailboxPtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the input client parameters.
 *
 * @param	ClientParamPtr	 	Pointer to the XAsu_ClientParams instance.
 *
 * @return
 * 	- XST_SUCCESS upon successful validation.
 * 	- XASU_INVALID_ARGUMENT, upon invalid arguments.
 *	- XASU_INVALID_CLIENT_PARAM if ClientParamPtr is NULL
 *	- XASU_INVALID_CALL_BACK_REF if CallBackFuncPtr is NULL
 *	- XASU_INVALID_PRIORITY if invalid priority is selected
 *
 *************************************************************************************************/
s32 XAsu_ValidateClientParameters(XAsu_ClientParams *ClientParamPtr)
{
    s32 Status = XASU_INVALID_ARGUMENT;

    if (ClientParamPtr == NULL) {
        Status = XASU_INVALID_CLIENT_PARAM;
        goto END;
    }

    if (ClientParamPtr->CallBackFuncPtr == NULL) {
		Status = XASU_INVALID_CALL_BACK_REF;
		goto END;
	}

    if ((ClientParamPtr->Priority != XASU_PRIORITY_HIGH) &&
	    (ClientParamPtr->Priority != XASU_PRIORITY_LOW)) {
		Status = XASU_INVALID_PRIORITY;
		goto END;
	}

    Status = XST_SUCCESS;

END:
    return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function updates the queue buffer status to notify the request is present and
 * 		generates a door bell to ASU.
 *
 * @param	ClientParam	 	Pointer to the XAsu_ClientParams instance.
 * @param	ReqBuffer		Pointer to the XAsu_ChannelQueueBuf's Request buffer data to be filled.
 * @param	Size			Size of the request buffer in bytes.
 * @param	Header			Header of the request buffer to be filled.
 *
 * @return
 * 	- XST_SUCCESS upon successful update.
 * 	- XST_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
s32 XAsu_UpdateQueueBufferNSendIpi(XAsu_ClientParams *ClientParam, void *ReqBuffer, u32 Size,
				   u32 Header)
{
	s32 Status = XST_FAILURE;
	XAsu_Client *ClientInstancePtr = XAsu_GetClientInstance();
	u8 FreeIndex = 0U;
	XAsu_ChannelQueueBuf *QueueBufPtr = NULL;
	XAsu_ChannelQueue *ChannelQPtr = NULL;

	/** Validate input parameters. */
	if ((ClientParam == NULL) || (Header == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
	}
	if (ClientInstancePtr->IsReady != XASU_CLIENT_READY) {
		Status = XASU_CLIENT_NOT_INITIALIZED;
	}

	/** Get free queue buffer index of selected priority */
	FreeIndex = XAsu_GetFreeIndex(ClientParam->Priority);
	if (FreeIndex == XASU_MAX_BUFFERS) {
		Status = XASU_QUEUE_FULL;
		goto END;
	}

	if (ClientParam->Priority == XASU_PRIORITY_HIGH) {
		QueueBufPtr = &ClientInstancePtr->ChannelMemoryPtr->P0ChannelQueue.
						ChannelQueueBufs[FreeIndex];
		ChannelQPtr = &ClientInstancePtr->ChannelMemoryPtr->P0ChannelQueue;
    }
	else {
		QueueBufPtr = &ClientInstancePtr->ChannelMemoryPtr->P1ChannelQueue.
						ChannelQueueBufs[FreeIndex];
		ChannelQPtr = &ClientInstancePtr->ChannelMemoryPtr->P1ChannelQueue;
	}

	QueueBufPtr->ReqBuf.Header = Header;
	if ((ReqBuffer != NULL) && (Size != 0x0U)) {
		Status = Xil_SecureMemCpy((void *)QueueBufPtr->ReqBuf.Arg,
					  sizeof(QueueBufPtr->ReqBuf.Arg), ReqBuffer, Size);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	QueueBufPtr->RespBufStatus = 0x0U;
	QueueBufPtr->ReqBufStatus = XASU_COMMAND_IS_PRESENT;

	/** Set IsCmdPresent to TRUE to indicate the command is present in the queue. */
	ChannelQPtr->IsCmdPresent = TRUE;

	/** Place an IPI request to ASU. */
	Status = XAsu_SendIpi();
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function registers call back parameters across the generated unique ID per request
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds
 *              				the client input arguments.
 * @param	RespBufferPtr		Buffer to hold the response from response buffer, if no data is
 *								part of response buffer for the request the value shall be NULL.
 * @param	Size				Size of the data to be filled in response buffer.
 *
 * @return
 * 			- Unique ID used for registration (valid values are 0 to (XASU_UNIQUE_ID_MAX - 1))
 *          - XASU_UNIQUE_ID_MAX - upon unique ID unavailability
 *
 *************************************************************************************************/
u8 XAsu_RegCallBackNGetUniqueId(XAsu_ClientParams *ClientParamPtr, u8 *RespBufferPtr, u32 Size)
{
	u8 UniqueId = XAsu_GenerateUniqueId();

	if (UniqueId == XASU_UNIQUE_ID_MAX) {
		goto END;
	}

	/* Store the details for reference */
	AsuCallBackRef[UniqueId].CallBackFuncPtr = ClientParamPtr->CallBackFuncPtr;
	AsuCallBackRef[UniqueId].CallBackRefPtr = ClientParamPtr->CallBackRefPtr;
	AsuCallBackRef[UniqueId].RespBufferPtr = RespBufferPtr;
	AsuCallBackRef[UniqueId].Size = Size;
END:
	return UniqueId;
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
	XAsu_Client *ClientInstancePtr = XAsu_GetClientInstance();

	return((s32)XMailbox_Send(ClientInstancePtr->MailboxPtr,
				    XASU_TARGET_IPI_INT_MASK, FALSE));
}

/*************************************************************************************************/
/**
 * @brief	This function returns an instance pointer of ASU client interface.
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
	u8 BufferIdx;
	XAsu_Client *ClientInstancePtr = XAsu_GetClientInstance();
	XAsu_ChannelQueue *ChannelQueue = NULL;
	u8 UniqueId;
	u8 Priority = XASU_PRIORITY_HIGH;

	(void)CallBackRef;

	ChannelQueue = &ClientInstancePtr->ChannelMemoryPtr->P0ChannelQueue;
	do {
		for (BufferIdx = 0U; BufferIdx < XASU_MAX_BUFFERS; ++BufferIdx) {
			if (ChannelQueue->ChannelQueueBufs[BufferIdx].RespBufStatus == XASU_RESPONSE_IS_PRESENT) {
				/** Get UniqueID */
				UniqueId = XAsu_GetUniqueId(ChannelQueue->ChannelQueueBufs[BufferIdx].RespBuf.Header);
				/** Copy the response buffer data if any */
				if (AsuCallBackRef[UniqueId].RespBufferPtr != NULL) {
					memcpy(AsuCallBackRef[UniqueId].RespBufferPtr,
					       &ChannelQueue->ChannelQueueBufs[BufferIdx].RespBuf.Arg[1],
					       AsuCallBackRef[UniqueId].Size);
				}
				/** Call back to notify the completion */
				if (AsuCallBackRef[UniqueId].CallBackFuncPtr != NULL) {
					AsuCallBackRef[UniqueId].CallBackFuncPtr(AsuCallBackRef[UniqueId].CallBackRefPtr,
						ChannelQueue->ChannelQueueBufs[BufferIdx].RespBuf.Arg[0]);
					/** Clear the contents upon completion */
					AsuCallBackRef[UniqueId].CallBackFuncPtr = NULL;
					AsuCallBackRef[UniqueId].CallBackRefPtr = NULL;
					(void)memset(&ChannelQueue->ChannelQueueBufs[BufferIdx], 0, sizeof(XAsu_ChannelQueueBuf));
				}
			}
		}
		ChannelQueue = &ClientInstancePtr->ChannelMemoryPtr->P1ChannelQueue;
		Priority++;
	} while (Priority <= XASU_PRIORITY_LOW);
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

/*************************************************************************************************/
/**
 * @brief	This function generates an unique ID.
 *
 * @return	Unique ID
 *			- 0 to (XASU_UNIQUE_ID_MAX-1) if AsuCallBackRef array has any empty index.
 *			- XASU_UNIQUE_ID_MAX upon no free space is left
 *
 *************************************************************************************************/
static u8 XAsu_GenerateUniqueId(void)
{
	static u8 UniqueId = XASU_UNIQUE_ID_MAX;
	u8 TempId = UniqueId;

	do {
		if (UniqueId < (XASU_UNIQUE_ID_MAX - 1U)) {
			UniqueId++;
		}
		else {
			UniqueId = 0U;
		}
		/** Validate if the assigned unique ID is free */
		if ((AsuCallBackRef[UniqueId].CallBackFuncPtr == NULL) &&
		    (AsuCallBackRef[UniqueId].CallBackRefPtr == NULL)) {
			break;
		}
	} while (UniqueId != TempId);

	if (UniqueId == TempId) {
		UniqueId = XASU_UNIQUE_ID_MAX;
	}

	return UniqueId;
}

/*************************************************************************************************/
/**
 * @brief	This function gets the free index of selected priority queue buffer.
 *
 * @param	Priority	Select the queue priority.
 *
 * @return	Free Index of the Channel queue buffer of the selected priority
 *			- 0 to 7
 *			- XASU_MAX_BUFFERS if no index is free
 *
 *************************************************************************************************/
static u8 XAsu_GetFreeIndex(u8 Priority)
{
	XAsu_Client *ClientInstancePtr = XAsu_GetClientInstance();
	XAsu_ChannelQueue *ChannelQPtr = NULL;
	u8 *FreeIndexPtr = NULL;
	u8 TempIndex;

	/** Get next free index */
	if (Priority == XASU_PRIORITY_HIGH) {
		FreeIndexPtr = &ClientInstancePtr->P0NextFreeIndex;
		ChannelQPtr = &ClientInstancePtr->ChannelMemoryPtr->P0ChannelQueue;
	}
	else {
		FreeIndexPtr = &ClientInstancePtr->P1NextFreeIndex;
		ChannelQPtr = &ClientInstancePtr->ChannelMemoryPtr->P1ChannelQueue;
	}
	if (*FreeIndexPtr == XASU_MAX_BUFFERS) {
		*FreeIndexPtr = 0U;
	}
	TempIndex = *FreeIndexPtr;

	while ((ChannelQPtr->ChannelQueueBufs[*FreeIndexPtr].ReqBufStatus != 0U) ||
	       (ChannelQPtr->ChannelQueueBufs[*FreeIndexPtr].RespBufStatus != 0U)) {
		*FreeIndexPtr = (*FreeIndexPtr + 1) % XASU_MAX_BUFFERS;
		if (TempIndex == *FreeIndexPtr) {
			*FreeIndexPtr = XASU_MAX_BUFFERS;
			break;
		}
	}

	return *FreeIndexPtr;
}
/** @} */
