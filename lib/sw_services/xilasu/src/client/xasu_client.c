/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
 * 1.1   ma   01/08/25 Clear only ReqBufStatus and RespBufStatus upon command completion
 *       vns  02/06/25 Fixed magic numbers
 *       ma   02/19/25 Updated handling of same priority queue requests in round robin scheduling
 *       am   03/05/25 Added performance measurement init call
 *       ma   03/14/25 Replace memcpy with Xil_SecureMemCpy to avoid arch dependencies during copy
 *       kd   07/23/25 Fixed gcc warnings
 *       am   08/08/25 Removed redundant condition before END label
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_client_info Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_client.h"
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
#define XASU_ASUFW_BIT_CHECK_TIMEOUT_VALUE	0xFFFFFU	/**< ASUFW check timeout value */

#define XASU_NO_OF_CONTEXTS				(10U)	/**< No of contexts can be saved by client */

/** @} */
/************************************** Type Definitions *****************************************/
/** This structure represents a client context, storing a unique identifier. */
typedef struct {
	u8 UniqueId;		/**< Unique identifier for the client context. */
} XAsu_ClientCtx;

/**
 * This structure contains all the parameters required to manage the client library
 * Also it holds the shared memory queue index details
 */
typedef struct {
	XMailbox *MailboxPtr;	/**< Mailbox instance pointer */
	XAsu_ChannelMemory *ChannelMemoryPtr;/**< Pointer to the XAsu_ChannelMemory */
	u8 P0NextFreeIndex;	/**< P0 free index */
	u8 P1NextFreeIndex;	/**< P1 free index */
	u32 IsReady;	/**< Client ready flag */
} XAsu_Client;

/** This structure holds the callback reference of the requests to respond upon completion */
typedef struct {
	XAsu_ClientParams *ClientParams; /**< Pointer to the XAsu_ClientParams */
	u8 *RespBufferPtr;		/**< Buffer to store the response data */
	u32 Size;				/**< Size of the response buffer to be copied */
	u8 Clear;				/**< Clear the contents after the callback */
} XAsu_RefToCallBack;

/**
 * @addtogroup xasu_client_info Client APIs
 * @{
*/
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
	XASU_RTCA_COMM_CHANNEL_INFO_ADDR; /**< All IPI channels information received from user
						configuration */
static XAsu_RefToCallBack AsuCallBackRef[XASU_UNIQUE_ID_MAX]; /**< Entry of callback info */

static XAsu_ClientCtx AsuContext[XASU_NO_OF_CONTEXTS];	/**< ASU saved context */

/*************************************************************************************************/
/**
 * @brief	This function initializes the client instance.
 *
 * @param	MailboxInstancePtr	Pointer to the Mailbox instance.
 *
 * @return
 * 		- XST_SUCCESS, On successful initialization.
 * 		- XST_FAILURE, On failure.
 *
 *************************************************************************************************/
s32 XAsu_ClientInit(XMailbox *MailboxInstancePtr)
{
	s32 Status = XST_FAILURE;
	XAsu_Client *ClientInstancePtr = XAsu_GetClientInstance();
	u32 ChannelIdx;

	/** Validate client instance pointer. */
	if (ClientInstancePtr == NULL) {
		goto END;
	}

	/** Check for ASUFW present bit. */
	Status = XAsu_CheckAsufwPrsntBit();
	if (Status != XST_SUCCESS) {
		Status = XASU_ASUFW_NOT_PRESENT;
		goto END;
	}

	/** Initialize performance measurement. */
	XAsu_PerfInit();

	/** Map the IPI shared memory of the channel. */
	for (ChannelIdx = 0U; ChannelIdx < CommChannelInfo->NumOfIpiChannels; ++ChannelIdx) {
		if (MailboxInstancePtr->Agent.IpiInst.Config.BitMask ==
		    CommChannelInfo->Channel[ChannelIdx].IpiBitMask) {
			break;
		}
	}

	if (ChannelIdx == CommChannelInfo->NumOfIpiChannels) {
		Status = XASU_IPI_CONFIG_NOT_FOUND;
		goto END;
	}

	/* Verify if client instance is already initialized. */
	if (ClientInstancePtr->IsReady != XASU_CLIENT_READY) {
		/* Assign channel shared memory. */
		ClientInstancePtr->ChannelMemoryPtr = (XAsu_ChannelMemory *)(UINTPTR)(XASU_CHANNEL_MEMORY_BASEADDR +
						(XASU_CHANNEL_MEMORY_OFFSET * ChannelIdx));

		ClientInstancePtr->P0NextFreeIndex = 0U;
		ClientInstancePtr->P1NextFreeIndex = 0U;
		ClientInstancePtr->IsReady = XASU_CLIENT_READY;
	}
	ClientInstancePtr->MailboxPtr = MailboxInstancePtr;
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
 * 	- XST_SUCCESS, if client parameters validation is successful.
 * 	- XASU_INVALID_ARGUMENT, if the input parameters are invalid.
 *	- XASU_INVALID_CLIENT_PARAM, if ClientParamPtr is NULL.
 *	- XASU_INVALID_CALL_BACK_REF, if CallBackFuncPtr is NULL.
 *	- XASU_INVALID_PRIORITY, if invalid priority is selected.
 *
 *************************************************************************************************/
s32 XAsu_ValidateClientParameters(XAsu_ClientParams *ClientParamPtr)
{
    s32 Status = XASU_INVALID_ARGUMENT;

    if (ClientParamPtr == NULL) {
        Status = XASU_INVALID_CLIENT_PARAM;
        goto END;
    }

	/** Validate that the callback function pointer is not NULL. */
    if (ClientParamPtr->CallBackFuncPtr == NULL) {
		Status = XASU_INVALID_CALL_BACK_REF;
		goto END;
	}

	/** Validate the priority. */
    if ((ClientParamPtr->Priority != XASU_PRIORITY_HIGH) &&
	    (ClientParamPtr->Priority != XASU_PRIORITY_LOW)) {
		Status = XASU_INVALID_PRIORITY;
		goto END;
	}

	/** Validate the secure flag. */
    if ((ClientParamPtr->SecureFlag != XASU_CMD_SECURE) &&
	    (ClientParamPtr->SecureFlag != XASU_CMD_NON_SECURE)) {
		Status = XASU_INVALID_SECURE_FLAG;
		goto END;
	}

    Status = XST_SUCCESS;

END:
    return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function updates the queue buffer status to notify the request is present and
 * 		generates a doorbell to ASU.
 *
 * @param	ClientParam	 	Pointer to the XAsu_ClientParams instance.
 * @param	ReqBuffer		Pointer to the XAsu_ChannelQueueBuf's Request buffer data to be filled.
 * @param	Size			Size of the request buffer in bytes.
 * @param	Header			Header of the request buffer to be filled.
 *
 * @return
 * 	- XST_SUCCESS, if IPI is sent successfully.
 * 	- XST_FAILURE, if there is any failure.
 * 	- XASU_INVALID_ARGUMENT, if the input parameters are invalid.
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
		goto END;
	}
	if (ClientInstancePtr->IsReady != XASU_CLIENT_READY) {
		Status = XASU_CLIENT_NOT_INITIALIZED;
		goto END;
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

	/** Copy the request buffer to the free index. */
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

	/** Set IsCmdPresent to XASU_TRUE to indicate the command is present in the queue. */
	ChannelQPtr->IsCmdPresent = XASU_TRUE;
	ChannelQPtr->ReqSent++;

	/** Place an IPI request to ASU. */
	Status = XAsu_SendIpi();

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function registers callback parameters across the generated unique ID per request
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds
 * 								the client input arguments.
 * @param	RespBufferPtr		Buffer to hold the response from response buffer, if no data is
 *					part of response buffer for the request the value shall be NULL.
 * @param	Size			Size of the data to be filled in response buffer.
 * @param	IsFinalCall		Flag indicating whether this is the final callback
 *					(1 if final, 0 otherwise).
 *
 * @return
 * 			- Unique ID used for registration (valid values are 0 to (XASU_UNIQUE_ID_MAX - 1)).
 * 			- XASU_UNIQUE_ID_MAX, if unique ID is unavailable.
 *
 *************************************************************************************************/
u8 XAsu_RegCallBackNGetUniqueId(XAsu_ClientParams *ClientParamPtr, u8 *RespBufferPtr, u32 Size,
	u8 IsFinalCall)
{
	/** Generate unique ID for the request.*/
	u8 UniqueId = XAsu_GenerateUniqueId();

	if (UniqueId == XASU_UNIQUE_ID_MAX) {
		goto END;
	}

	/** Store the callback details. */
	AsuCallBackRef[UniqueId].ClientParams = ClientParamPtr;
	AsuCallBackRef[UniqueId].RespBufferPtr = RespBufferPtr;
	AsuCallBackRef[UniqueId].Size = Size;
	AsuCallBackRef[UniqueId].Clear = IsFinalCall;
END:
	return UniqueId;
}

/*************************************************************************************************/
/**
 * @brief	Updates callback details based on the provided unique ID, response buffer, size, and
 *		final call flag.
 *
 * @param	UniqueId	Unique identifier for the callback update.
 * @param	RespBufferPtr	Pointer to the response buffer containing the data.
 * @param	Size		Size of the response buffer.
 * @param	IsFinalCall	Flag indicating whether this is the final callback (1 if final, 0 otherwise).
 *
 *************************************************************************************************/
void XAsu_UpdateCallBackDetails(u8 UniqueId, u8 *RespBufferPtr, u32 Size, u8 IsFinalCall)
{
	/** Update the callback details. */
	AsuCallBackRef[UniqueId].RespBufferPtr = RespBufferPtr;
	AsuCallBackRef[UniqueId].Size = Size;
	AsuCallBackRef[UniqueId].Clear = IsFinalCall;
}

/*************************************************************************************************/
/**
 * @brief	This function fetches the algorithm information based on Module ID and populates
 * 		the algorithm information structure.
 *
 * @param	AlginfoPtr	Pointer to the structure where the KDF algorithm
 *				information will be stored.
 * @param	ModuleId	Registered ID of the module.
 *
 * @return
 *      	- XST_SUCCESS, if version info is updated successfully.
 *      	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *
 *************************************************************************************************/
s32 XAsu_GetModuleInfo(XAsu_CryptoAlgInfo *AlginfoPtr, u32 ModuleId)
{
	s32 Status = XST_FAILURE;
	const XAsu_CryptoAlgInfo *AlgInfoDataPtr = (const XAsu_CryptoAlgInfo*)XASU_RTCA_MODULE_INFO_BASEADDR;

	/** Validate input parameters. */
	if (ModuleId >= XASU_MAX_MODULES) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	AlginfoPtr->Version = AlgInfoDataPtr[ModuleId].Version;
	AlginfoPtr->NistStatus = AlgInfoDataPtr[ModuleId].NistStatus;

	Status = XST_SUCCESS;
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends an IPI request to ASU.
 *
 * @return
 * 	- XST_SUCCESS, if the IPI is sent successfully.
 * 	- XST_FAILURE, if there is a failure.
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
 * 	- Pointer to the client instance.
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
 * @param	CallBackRef	Callback reference pointer.
 *
 ****************************************************************************/
static void XAsu_DoorBellToClient(void *CallBackRef)
{
	u8 BufferIdx;
	XAsu_Client *ClientInstancePtr = XAsu_GetClientInstance();
	XAsu_ChannelQueue *ChannelQueue = NULL;
	u8 UniqueId;
	u8 Priority = XASU_PRIORITY_HIGH;
	XAsu_ChannelQueueBuf *ChannelQueueBufPtr = NULL;

	(void)CallBackRef;

	ChannelQueue = &ClientInstancePtr->ChannelMemoryPtr->P0ChannelQueue;
	do {
		/** Search for the buffer index whose response is ready. */
		for (BufferIdx = 0U; BufferIdx < XASU_MAX_BUFFERS; ++BufferIdx) {
			ChannelQueueBufPtr = &ChannelQueue->ChannelQueueBufs[BufferIdx];
			if (ChannelQueueBufPtr->RespBufStatus == XASU_RESPONSE_IS_PRESENT) {
				/** Get UniqueID. */
				UniqueId = XAsu_GetUniqueId(ChannelQueueBufPtr->RespBuf.Header);
				/** Copy the response buffer data if any. */
				if (AsuCallBackRef[UniqueId].RespBufferPtr != NULL) {
					if (Xil_SecureMemCpy((void *)AsuCallBackRef[UniqueId].RespBufferPtr,
							AsuCallBackRef[UniqueId].Size,
							(void *)(&(ChannelQueueBufPtr->RespBuf.Arg[XASU_RESPONSE_BUFF_ADDR_INDEX])),
							AsuCallBackRef[UniqueId].Size) != XST_SUCCESS) {
						XilAsu_Printf("Response copy to application failed\r\n");
					}
				}

				/** Perform callback to application to indicate the operation is complete. */
				if (AsuCallBackRef[UniqueId].ClientParams->CallBackFuncPtr != NULL) {
					AsuCallBackRef[UniqueId].ClientParams->CallBackFuncPtr(
						AsuCallBackRef[UniqueId].ClientParams->CallBackRefPtr,
						ChannelQueueBufPtr->RespBuf.Arg[XASU_RESPONSE_STATUS_INDEX]);
					/** Copy additional status from response buffer. */
					AsuCallBackRef[UniqueId].ClientParams->AdditionalStatus =
								ChannelQueueBufPtr->RespBuf.AdditionalStatus;
					/** Clear the callback info upon completion. */
					if (AsuCallBackRef[UniqueId].Clear == XASU_TRUE) {
						AsuCallBackRef[UniqueId].ClientParams = NULL;
					}
					/** Clear request and response status. */
					ChannelQueueBufPtr->ReqBufStatus = 0x0U;
					ChannelQueueBufPtr->RespBufStatus = 0x0U;
				}
			}
		}
		ChannelQueue = &ClientInstancePtr->ChannelMemoryPtr->P1ChannelQueue;
		Priority++;
	} while (Priority <= XASU_PRIORITY_LOW);
}

/*************************************************************************************************/
/**
 * @brief	This function returns the ASUFW application's present status.
 *
 * @return
 * 	- XST_SUCCESS, if ASUFW is present.
 * 	- XST_FAILURE, if ASUFW is not present.
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
 * @brief	This function generates a unique ID.
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
			UniqueId = 1U;
		}
		/** Validate if the assigned unique ID is free. */
		if (AsuCallBackRef[UniqueId].ClientParams == NULL) {
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
 *			- XASU_MAX_BUFFERS, if no index is free
 *
 *************************************************************************************************/
static u8 XAsu_GetFreeIndex(u8 Priority)
{
	XAsu_Client *ClientInstancePtr = XAsu_GetClientInstance();
	XAsu_ChannelQueue *ChannelQPtr = NULL;
	u8 *FreeIndexPtr = NULL;
	u8 TempIndex;

	/** Get the next free index. */
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

/*************************************************************************************************/
/**
 * @brief	Frees the memory associated with the provided context.
 *
 * @param	Context		Pointer to the XAsu_ClientCtx to be freed.
 *
 *************************************************************************************************/
void XAsu_FreeCtx(void *Context)
{
	(void *)memset(Context, 0, sizeof(XAsu_ClientCtx));
}

/*************************************************************************************************/
/**
 * @brief	Saves the provided unique ID and returns the address of the saved context.
 *
 * @param	UniqueId	Unique ID associated with the request
 *
 * @return	Pointer of the XAsu_ClientCtx, which references the saved context.
 *
 *************************************************************************************************/
void *XAsu_UpdateNGetCtx(u8 UniqueId)
{
	u8 Index = 0U;
	XAsu_ClientCtx *Context = NULL;

	/** Find the free index to store the context. */
	do {
		/** Save the context. */
		if (AsuContext[Index].UniqueId == 0U) {
			AsuContext[Index].UniqueId = UniqueId;
			Context = &AsuContext[Index];
			break;
		}
		Index++;
	} while (Index < XASU_NO_OF_CONTEXTS);

	return (void *)Context;
}

/*************************************************************************************************/
/**
 * @brief	Validates the provided context by comparing it with stored contexts to determine
 *          the appropriate multi-update calls.
 *
 * @param	Context		Pointer to the XAsu_ClientCtx reference context.
 * @param	UniqueId	Pointer to the buffer where the corresponding Unique ID will be stored.
 *
 * @return
 * 		- XST_SUCCESS if validation is successful.
 * 		- XASU_INVALID_CLIENT_CTX if the provided context is not found.
 *
 *************************************************************************************************/
s32 XAsu_VerifyNGetUniqueIdCtx(const void *Context, u8 *UniqueId)
{
	volatile s32 Status = XASU_INVALID_CLIENT_CTX;
	u8 Index = 0U;
	const XAsu_ClientCtx *ClientCtx = (const XAsu_ClientCtx *)Context;

	do {
		if (AsuContext[Index].UniqueId == ClientCtx->UniqueId) {
			*UniqueId = AsuContext[Index].UniqueId;
			Status = XST_SUCCESS;
			break;
		}
		Index++;
	} while (Index < XASU_NO_OF_CONTEXTS);

	return Status;
}

/** @} */
