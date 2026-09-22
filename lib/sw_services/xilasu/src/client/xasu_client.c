/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       rmv  09/15/25 Updated XAsu_UpdateCallBackDetails() prototype to return status value
 *       rmv  09/20/25 Updated memory copy logic to store Xil_SecureMemCpy return value in local
 *                     variable to improve readability.
 *       rmv  09/21/25 Used unique id as an index for AsuContext
 *       ks   09/13/26 Validated client context ownership before accepting or releasing it
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_client_info ASU Client Functionality
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

#define XASU_ASUFW_BIT_CHECK_TIMEOUT_VALUE	0xFFFFFU	/**< ASUFW check timeout value */
#define XASU_KAT_EXEC_STATUS_CHECK_TIMEOUT_VALUE	(0x4C4B40U)	/**< KAT execution status
								check timeout value in microseconds
								(5sec) */

#define XASU_NO_OF_CONTEXTS		XASU_UNIQUE_ID_MAX	/**< No of contexts can be saved by
								client */

#define XASU_INITIAL_CALL_COUNT			(1U)	/**< Initial count for multi-update requests */
#define XASU_INVALID_UNIQUE_ID_VAL		(0xFFU) /**< Invalid unique id value */

/** @cond xasu_client_internal */
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
	u32 PendingMultiUpdates; /**< Number of pending multi-update requests */
	u8 Clear;				/**< Clear the contents after the callback if no pending updates */
} XAsu_RefToCallBack;

/** @endcond */
/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static XAsu_Client *XAsu_GetClientInstance(void);
static s32 XAsu_SendIpi(void);
static void XAsu_DoorBellToClient(void *CallBackRef);
static s32 XAsu_CheckAsufwPrsntBit(void);
static u8 XAsu_GenerateUniqueId(void);
static u8 XAsu_GetFreeIndex(u8 Priority);
static u8 XAsu_IsCtxOfRequest(const void *Context, u8 UniqueId);
static u8 XAsu_GetUniqueIdOfCtx(const void *Context, u8 *UniqueId);
static s32 XAsu_VerifyNGetUniqueIdCtx(const XAsu_ClientParams *ClientParamPtr, u8 *UniqueId);

/************************************ Variable Definitions ***************************************/
static XAsu_RefToCallBack AsuCallBackRef[XASU_UNIQUE_ID_MAX]; /**< Entry of callback info */

static XAsu_ClientCtx AsuContext[XASU_NO_OF_CONTEXTS] = {[0 ... (XASU_NO_OF_CONTEXTS - 1U)] =
							 {.UniqueId = XASU_INVALID_UNIQUE_ID_VAL}};
	/**< ASU Context with initial UniqueId value as XASU_INVALID_UNIQUE_ID_VAL
	for all contexts */

/*
 * A context is addressed by the unique ID of the request that owns it, so a unique ID validated
 * against one array is used to index the other. Both arrays must therefore hold one entry per
 * unique ID.
 */
_Static_assert(XASU_NO_OF_CONTEXTS == XASU_UNIQUE_ID_MAX,
	       "Context array must have one entry per unique ID");

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

	/** Check ASUFW version compatibility with client library. */
	Status = XAsu_CheckFwCompatibility();
	if (Status != XST_SUCCESS) {
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

	if (ChannelIdx >= CommChannelInfo->NumOfIpiChannels) {
		Status = XASU_IPI_CONFIG_NOT_FOUND;
		goto END;
	}

	/* Verify if client instance is already initialized. */
	if (ClientInstancePtr->IsReady != XASU_CLIENT_READY) {
		/* Assign channel shared memory. */
		ClientInstancePtr->ChannelMemoryPtr = (XAsu_ChannelMemory *)(UINTPTR)(XASU_CHANNEL_MEMORY_BASEADDR +
						(XASU_CHANNEL_MEMORY_OFFSET * (UINTPTR)ChannelIdx));

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
s32 XAsu_SendCmdToAsu(XAsu_ClientParams *ClientParam, void *ReqBuffer, u32 Size,
				   u32 Header)
{
	s32 Status = XST_FAILURE;
	XAsu_Client *ClientInstancePtr = XAsu_GetClientInstance();
	u8 FreeIndex = 0U;
	XAsu_ChannelQueueBuf *QueueBufPtr = NULL;
	XAsu_ChannelQueue *ChannelQPtr = NULL;

	/** Validate input parameters. */
	if (ClientParam == NULL) {
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
	if (Status != XST_SUCCESS) {
		/** If sending IPI fails, clean up the queue buffer. */
		ChannelQPtr->ReqSent--;
		QueueBufPtr->ReqBufStatus = 0U;
		ChannelQPtr->IsCmdPresent = XASU_FALSE;
	}

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
	AsuCallBackRef[UniqueId].PendingMultiUpdates = XASU_INITIAL_CALL_COUNT;
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
 * @return
 *	- XST_SUCCESS, if callback details are updated successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsu_UpdateCallBackDetails(u8 UniqueId, u8 *RespBufferPtr, u32 Size, u8 IsFinalCall)
{
	s32 Status = XST_FAILURE;

	/** Validate unique ID. */
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		goto END;
	}

	/** Update the callback details. */
	AsuCallBackRef[UniqueId].RespBufferPtr = RespBufferPtr;
	AsuCallBackRef[UniqueId].Size = Size;
	AsuCallBackRef[UniqueId].Clear = IsFinalCall;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function fetches the algorithm information based on Module ID and populates
 * 		the algorithm information structure.
 *
 * @param	AlginfoPtr	Pointer to the structure where the algorithm
 *				information of module will be stored.
 * @param	ModuleId	Registered ID of the module.
 *
 * @return
 *      	- XST_SUCCESS, if version info is updated successfully.
 * 		- XST_FAILURE, if version info update fails.
 * 		- XASU_KAT_EXEC_NOT_COMPLETED, if KAT execution is not completed.
 *      	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *
 *************************************************************************************************/
s32 XAsu_GetModuleInfo(XAsu_CryptoAlgInfo *AlginfoPtr, u32 ModuleId)
{
	s32 Status = XST_FAILURE;
	u32 Timeout = 0U;
	const XAsu_CryptoAlgInfo *AlgInfoDataPtr = NULL;

	/** Validate input parameters. */
	if ((AlginfoPtr == NULL) || (ModuleId >= XASU_MAX_MODULES)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Wait until KAT execution is completed. */
	while(((Xil_In32(XASU_RTCA_EXEC_STATUS_ADDR)) & XASU_RTCA_KAT_EXEC_STATUS_MASK)
		!= XASU_RTCA_KAT_EXEC_STATUS_VALUE) {
		if (Timeout == XASU_KAT_EXEC_STATUS_CHECK_TIMEOUT_VALUE) {
			Status = XASU_KAT_EXEC_NOT_COMPLETED;
			goto END;
		}
		usleep(1U);
		Timeout++;
	}

	AlgInfoDataPtr = (const XAsu_CryptoAlgInfo *)XAsu_GetModuleInfoSlot(ModuleId);

	Status = Xil_SecureMemCpy((void *)AlginfoPtr, (u32)sizeof(XAsu_CryptoAlgInfo),
				   (const void *)AlgInfoDataPtr, (u32)sizeof(XAsu_CryptoAlgInfo));
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks the backward compatibility between the ASUFW version
 *		running on the server and the XilAsu client library version. It reads the
 *		ASUFW version from the RTCA shared memory and compares it against
 *		the client library version.
 *
 * @return
 *	- XST_SUCCESS, if ASUFW version is compatible with the client.
 *	- XASU_FW_VERSION_MISMATCH, if ASUFW version is not compatible.
 *
 *************************************************************************************************/
s32 XAsu_CheckFwCompatibility(void)
{
	s32 Status = XASU_FW_VERSION_MISMATCH;
	u32 FwVersionReg;
	u32 FwMajor;
#if (XASU_FW_VERSION_MINOR > 0U)
	u32 FwMinor;
#endif

	/** Read ASUFW firmware version from RTCA shared memory. */
	FwVersionReg = Xil_In32(XASU_RTCA_FW_VERSION_ADDR);

	/** Extract major and minor version fields. */
	FwMajor = (FwVersionReg & XASU_FW_VERSION_MAJOR_MASK) >> XASU_FW_VERSION_MAJOR_SHIFT;
#if (XASU_FW_VERSION_MINOR > 0U)
	FwMinor = (FwVersionReg & XASU_FW_VERSION_MINOR_MASK);
#endif

	/**
	 * Backward compatibility check:
	 * - Major version must match the client major version (different major = breaking change).
	 * - Minor version of ASUFW must be >= client minor version (higher minor = new features
	 *   added but backward compatible, lower minor = server missing features client expects).
	 */
	if (FwMajor != (u32)XASU_FW_VERSION_MAJOR) {
		goto END;
	}

#if (XASU_FW_VERSION_MINOR > 0U)
	if (FwMinor < (u32)XASU_FW_VERSION_MINOR) {
		goto END;
	}
#endif

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
	s32 Status = XST_FAILURE;
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
				if (UniqueId >= XASU_UNIQUE_ID_MAX) {
					continue;
				}
				/** Copy the response buffer data if any. */
				if (AsuCallBackRef[UniqueId].RespBufferPtr != NULL) {
					Status = Xil_SecureMemCpy((void *)AsuCallBackRef[UniqueId].RespBufferPtr,
							AsuCallBackRef[UniqueId].Size,
							(void *)(&(ChannelQueueBufPtr->RespBuf.Arg[XASU_RESPONSE_BUFF_ADDR_INDEX])),
							AsuCallBackRef[UniqueId].Size);
					if (Status != XST_SUCCESS) {
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
					AsuCallBackRef[UniqueId].PendingMultiUpdates--;
					/** Clear the callback info upon completion. */
					if ((AsuCallBackRef[UniqueId].PendingMultiUpdates == 0U) &&
						(AsuCallBackRef[UniqueId].Clear == XASU_TRUE)) {
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
		if ((Xil_In32(XASU_RTCA_EXEC_STATUS_ADDR) & XASU_RTCA_FW_IS_PRESENT_STATUS_MASK)
			== XASU_RTCA_FW_IS_PRESENT_STATUS_VALUE) {
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
 * @brief	This function generates a unique ID for client requests. The client library assigns
 * 		a unique ID to each request, ensuring distinct identification.
 * 		This ID is used to map the application's response handlers, which are invoked when the
 * 		corresponding response IPI interrupt is received from ASUFW.
 *
 * @return	Unique ID
 *			- 0 to (XASU_UNIQUE_ID_MAX-1) if AsuCallBackRef array has any empty index.
 *			- XASU_UNIQUE_ID_MAX upon no free space is left
 *
 *************************************************************************************************/
static u8 XAsu_GenerateUniqueId(void)
{
	u8 UniqueId;

	for (UniqueId = 0U; UniqueId < XASU_UNIQUE_ID_MAX; UniqueId++) {
		if (AsuCallBackRef[UniqueId].ClientParams == NULL) {
			goto END;
		}
	}

	UniqueId = XASU_UNIQUE_ID_MAX;

END:
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
 * @return
 * 		- XST_SUCCESS, if the context is freed successfully.
 * 		- XASU_INVALID_ARGUMENT, if the context pointer is NULL.
 * 		- XASU_FREE_CTX_FAIL, if freeing the context fails.
 *
 *************************************************************************************************/
s32 XAsu_FreeCtx(void *Context)
{
	s32 Status = XST_FAILURE;

	/** Validate the context pointer. */
	if (Context == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/* Clear context buffer */
	Status = Xil_SMemSet(Context, sizeof(XAsu_ClientCtx), 0U, sizeof(XAsu_ClientCtx));
	if (Status != XST_SUCCESS) {
		Status = XASU_FREE_CTX_FAIL;
		goto END;
	}

	/* Mark context as free by resetting UniqueId to XASU_INVALID_UNIQUE_ID_VAL. */
	((XAsu_ClientCtx *)Context)->UniqueId = XASU_INVALID_UNIQUE_ID_VAL;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Checks whether the given context is the context allocated for the given request.
 *
 * @param	Context		Pointer to the XAsu_ClientCtx supplied by the client.
 * @param	UniqueId	Unique ID associated with the request.
 *
 * @return
 * 		- XASU_TRUE, if the context is the one allocated for the request.
 * 		- XASU_FALSE, otherwise.
 *
 *************************************************************************************************/
static u8 XAsu_IsCtxOfRequest(const void *Context, u8 UniqueId)
{
	u8 IsCtxOfRequest = XASU_FALSE;

	/**
	 * A free slot carries XASU_INVALID_UNIQUE_ID_VAL, so requiring the slot to record its own
	 * index keeps a pointer to a slot that was never handed out, or that has since been
	 * released, from passing as a live context.
	 */
	if ((Context != NULL) && (UniqueId < XASU_NO_OF_CONTEXTS) &&
	    (Context == (const void *)&AsuContext[UniqueId]) &&
	    (AsuContext[UniqueId].UniqueId == UniqueId)) {
		IsCtxOfRequest = XASU_TRUE;
	}

	return IsCtxOfRequest;
}

/*************************************************************************************************/
/**
 * @brief	Looks up the unique ID of the allocated context the given pointer refers to.
 *
 * @param	Context		Pointer to the XAsu_ClientCtx supplied by the client.
 * @param	UniqueId	Pointer to the buffer where the unique ID owning the context will be
 *				stored.
 *
 * @return
 * 		- XASU_TRUE, if the pointer refers to an allocated context.
 * 		- XASU_FALSE, otherwise.
 *
 *************************************************************************************************/
static u8 XAsu_GetUniqueIdOfCtx(const void *Context, u8 *UniqueId)
{
	u8 IsCtxAllocated = XASU_FALSE;
	u8 Index;

	/** Validate the output parameter. */
	if (UniqueId == NULL) {
		goto END;
	}

	/**
	 * The position of the context in AsuContext is the unique ID that owns it, so matching the
	 * pointer against every slot yields the unique ID without reading client supplied memory.
	 */
	for (Index = 0U; Index < XASU_NO_OF_CONTEXTS; Index++) {
		if (XAsu_IsCtxOfRequest(Context, Index) == XASU_TRUE) {
			*UniqueId = Index;
			IsCtxAllocated = XASU_TRUE;
			break;
		}
	}

END:
	return IsCtxAllocated;
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
	XAsu_ClientCtx *Context = NULL;

	/** Validate unique id. */
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		goto END;
	}

	/** Store the context based on unique id. */
	AsuContext[UniqueId].UniqueId = UniqueId;
	Context = &AsuContext[UniqueId];

END:
	return (void *)Context;
}

/*************************************************************************************************/
/**
 * @brief	Validates the context carried by the client parameters and returns the unique ID
 *		of the request that owns it.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams holding the reference context.
 * @param	UniqueId	Pointer to the buffer where the corresponding Unique ID will be stored.
 *
 * @return
 * 		- XST_SUCCESS if validation is successful.
 * 		- XASU_INVALID_ARGUMENT if ClientParamPtr, the context it carries or UniqueId is NULL.
 * 		- XASU_INVALID_CLIENT_CTX if the context is not the one allocated to this request.
 *
 *************************************************************************************************/
static s32 XAsu_VerifyNGetUniqueIdCtx(const XAsu_ClientParams *ClientParamPtr, u8 *UniqueId)
{
	volatile s32 Status = XASU_INVALID_CLIENT_CTX;
	u8 CtxUniqueId = 0U;

	/** Validate input parameters. */
	if ((ClientParamPtr == NULL) || (ClientParamPtr->ClientCtx == NULL) || (UniqueId == NULL)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Accept the context only when the client pointer refers to an allocated context and that
	 * allocation still belongs to this request. The unique ID is taken from the position of
	 * the context in AsuContext rather than from the client supplied memory it points at, so
	 * a foreign pointer can neither fault this read nor charge a multi-update to the slot of
	 * another request and leave that slot reserved for good.
	 */
	if ((XAsu_GetUniqueIdOfCtx(ClientParamPtr->ClientCtx, &CtxUniqueId) == XASU_TRUE) &&
	    (AsuCallBackRef[CtxUniqueId].ClientParams == ClientParamPtr)) {
		*UniqueId = CtxUniqueId;
		AsuCallBackRef[CtxUniqueId].PendingMultiUpdates++;
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Handles INIT or UPDATE/FINAL operations for multi-part cryptographic
 *		operations with priority-based context management.
 *		The context created on INIT is tied to the XAsu_ClientParams instance it was
 *		created with, so every UPDATE and FINAL call of one multi-part operation must be
 *		issued with that same instance. A different instance is rejected with
 *		XASU_INVALID_CLIENT_CTX even when it carries a copy of the context pointer.
 *		An instance holds a single context, so it can track only one multi-part
 *		operation at a time. Starting a second one replaces the context of the first,
 *		which can then be neither continued nor finished and holds its priority for
 *		the lifetime of the program.
 *
 * @param	ClientParamPtr		Pointer to XAsu_ClientParams structure with priority information.
 * @param	P0CtxPtr		Pointer to HIGH priority (P0) context.
 * @param	P1CtxPtr		Pointer to LOW priority (P1) context.
 * @param	OperationFlags		Current operation flags (combination of XASU_INIT, XASU_UPDATE,
 *					XASU_FINISH) from the client request. XASU_INIT creates a
 *					context, and XASU_FINISH alongside it marks a request that
 *					completes within this call and therefore needs none.
 *					XASU_FINISH on its own is not examined: every request
 *					without XASU_INIT continues the operation that owns the
 *					context.
 * @param	UniqueIdPtr		Pointer to store the generated or retrieved Unique ID.
 *
 * @return
 *		- XST_SUCCESS, if operation is successful.
 *		- XASU_INVALID_ARGUMENT, if ClientParamPtr, P0CtxPtr, P1CtxPtr or UniqueIdPtr is
 *		  NULL.
 *		- XASU_INVALID_PRIORITY, if the priority in ClientParamPtr is invalid.
 *		- XASU_INVALID_UNIQUE_ID, if generated Unique ID is invalid.
 *		- XASU_REQUEST_INPROGRESS, if a multi-part request is already in progress.
 *		- XASU_FAIL_SAVE_CTX, if context save fails.
 *		- XASU_CLIENT_CTX_NOT_CREATED, if client context does not exist.
 *		- XASU_INVALID_CLIENT_CTX, if the context does not belong to this request.
 *
 *************************************************************************************************/
s32 XAsu_HandleContextOperation(XAsu_ClientParams *ClientParamPtr, void **P0CtxPtr,
				void **P1CtxPtr, u32 OperationFlags, u8 *UniqueIdPtr)
{
	s32 Status = XST_FAILURE;
	void **CurrentCtxPtr;

	/** Validate input parameters. */
	if ((ClientParamPtr == NULL) || (P0CtxPtr == NULL) || (P1CtxPtr == NULL) ||
	    (UniqueIdPtr == NULL)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Determine context pointer based on priority. */
	if (ClientParamPtr->Priority == XASU_PRIORITY_HIGH) {
		CurrentCtxPtr = P0CtxPtr;
	} else if (ClientParamPtr->Priority == XASU_PRIORITY_LOW) {
		CurrentCtxPtr = P1CtxPtr;
	} else {
		Status = XASU_INVALID_PRIORITY;
		goto END;
	}

	/** Check if this is an INIT operation. */
	if ((OperationFlags & XASU_INIT) == XASU_INIT) {
		/** Check if context already exists for this priority level. */
		if (*CurrentCtxPtr != NULL) {
			/** A request that finishes here needs no context of its own. */
			if ((OperationFlags & XASU_FINISH) == XASU_FINISH) {
				*UniqueIdPtr = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U,
								XASU_TRUE);
				if (*UniqueIdPtr >= XASU_UNIQUE_ID_MAX) {
					Status = XASU_INVALID_UNIQUE_ID;
					goto END;
				}
			} else {
				Status = XASU_REQUEST_INPROGRESS;
				goto END;
			}
		} else {
			/** Create new context for new multi-part or single-shot operation. */
			*UniqueIdPtr = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U,
							XASU_FALSE);
			if (*UniqueIdPtr >= XASU_UNIQUE_ID_MAX) {
				Status = XASU_INVALID_UNIQUE_ID;
				goto END;
			}

			/** Save context and update client context pointer. */
			*CurrentCtxPtr = XAsu_UpdateNGetCtx(*UniqueIdPtr);
			if (*CurrentCtxPtr == NULL) {
				Status = XASU_FAIL_SAVE_CTX;
				goto END;
			}

			ClientParamPtr->ClientCtx = *CurrentCtxPtr;
		}
	} else {
		/** Handle UPDATE/FINAL operation - verify that context exists. */
		if (ClientParamPtr->ClientCtx == NULL) {
			Status = XASU_CLIENT_CTX_NOT_CREATED;
			goto END;
		}

		/**
		 * The context must be the one this priority handed out. Starting a second
		 * operation through the same client parameters instance replaces the context of
		 * the first, and without this check the first one would carry on under the
		 * unique ID of the second.
		 */
		if (ClientParamPtr->ClientCtx != *CurrentCtxPtr) {
			Status = XASU_INVALID_CLIENT_CTX;
			goto END;
		}

		Status = XAsu_VerifyNGetUniqueIdCtx(ClientParamPtr, UniqueIdPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Cleans up context and updates callback details when a FINISH/FINAL operation
 *		is executed.
 *
 * @param	ClientParamPtr	Pointer to XAsu_ClientParams structure.
 * @param	P0CtxPtr	HIGH priority (P0) context.
 * @param	P1CtxPtr	LOW priority (P1) context.
 * @param	UniqueId	Unique ID associated with this operation.
 * @param	ResponseBuffer	Pointer to response buffer address (can be NULL).
 * @param	ResponseSize	Size of the response buffer.
 *
 * @return
 *		- XST_SUCCESS, if cleanup is successful.
 *		- XASU_INVALID_ARGUMENT, if ClientParamPtr, P0CtxPtr or P1CtxPtr is NULL.
 *		- XASU_INVALID_CLIENT_CTX, if the context belongs to neither priority of this
 *		  operation.
 *		- Error code from callback update or context free.
 *
 *************************************************************************************************/
s32 XAsu_CleanupFinishOperation(XAsu_ClientParams *ClientParamPtr, void **P0CtxPtr,
				void **P1CtxPtr, u8 UniqueId, u8 *ResponseBuffer,
				u32 ResponseSize)
{
	s32 Status = XST_FAILURE;

	/** Validate input parameters. */
	if ((ClientParamPtr == NULL) || (P0CtxPtr == NULL) || (P1CtxPtr == NULL)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	Status = XAsu_UpdateCallBackDetails(UniqueId, ResponseBuffer, ResponseSize, XASU_TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * A request that owns no context has nothing to release. That is the single-shot request
	 * served while a multi-part operation holds the context of its priority. The unique ID is
	 * the one the library issued for this request rather than a value taken from the client,
	 * so matching the address of its context slot is ownership enough here.
	 */
	if (XAsu_IsCtxOfRequest(ClientParamPtr->ClientCtx, UniqueId) == XASU_FALSE) {
		goto END;
	}

	/**
	 * Release only the context this operation was given for its priority. A continuation is
	 * bound to that slot by XAsu_HandleContextOperation() before it gets here, so this guards
	 * against a caller pairing a context with the priority slots of a different operation.
	 */
	if (ClientParamPtr->ClientCtx == *P0CtxPtr) {
		*P0CtxPtr = NULL;
	} else if (ClientParamPtr->ClientCtx == *P1CtxPtr) {
		*P1CtxPtr = NULL;
	} else {
		Status = XASU_INVALID_CLIENT_CTX;
		goto END;
	}

	Status = XAsu_FreeCtx(ClientParamPtr->ClientCtx);
	ClientParamPtr->ClientCtx = NULL;

END:
	return Status;
}

/** @} */
