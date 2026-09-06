/***************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xpuf_generic.c
 *
 * This file contains the implementation of the generic request API that handles
 * both SMC and IPI mailbox communication for xilpuf client library.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------------------------
 * 2.7   tbk  03/24/26 Initial release for unified SMC/Mailbox handling
 * 2.8   tbk  06/22/26 Added response buffer parameter validation
 *       tus  08/07/26 Returned the response payload on the IPI mailbox path
 *
 * </pre>
 *
*************************************************************************************************/

/**
 * @addtogroup xpuf_client_api XilPuf Client APIs
 * @{
 */

/*************************************** Include Files *******************************************/
#include "xpuf_generic.h"
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
#include "xpuf_smc.h"
#endif

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/

/*************************************************************************************************/
/**
 * @brief  This function sends request to PLM through either SMC or IPI mailbox based on the
 *         execution environment. For EL1 non-secure AArch64 applications, SMC is used. For other
 *         environments, IPI mailbox is used.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	PayloadBuf	Pointer to the payload message
 * @param	PayloadLen	Length of the payload message
 * @param	ResponseBuf	Pointer to the caller buffer that receives the response
 *				data words. When non-NULL, it must hold RESPONSE_ARG_CNT
 *				words: the IPI receive path fills the whole response frame
 *				before the status word is shifted out. May be NULL when no
 *				response data is needed.
 * @param	ResponseLen	Number of response data words the caller consumes
 *				(0..RESPONSE_ARG_CNT - 1). This is the count of returned
 *				words, not the required buffer size (see ResponseBuf).
 *
 * @return
 *		- XST_SUCCESS  if the request is successful
 *		- XST_FAILURE  if there is a failure
 *		- XST_INVALID_PARAM  if input parameters are invalid
 *
 * @note	This is a generic API that internally handles SMC or Mailbox communication based on
 *		the build configuration.
 *
*************************************************************************************************/
int XPuf_SendRequest(const XPuf_ClientInstance *InstancePtr, u32 *PayloadBuf, u32 PayloadLen,
			u32 *ResponseBuf, u32 ResponseLen)
{
	volatile int Status = XST_FAILURE;
	u32 Idx = 0U;

	/**
	 * - A non-zero ResponseLen requires a non-NULL ResponseBuf. ResponseLen is
	 *   clamped below to the reachable data words before the mailbox shift.
	 */
	if ((PayloadBuf == NULL) || (PayloadLen == 0U) ||
	    (ResponseLen > (u32)RESPONSE_ARG_CNT) ||
	    ((ResponseLen > 0U) && (ResponseBuf == NULL))) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

#if defined (__aarch64__) && (EL1_NONSECURE == 1)
	/** - For AArch64 EL1 non-secure, use SMC call. */
	(void)InstancePtr; /**< Mark InstancePtr as unused to avoid compiler warning */
	(void)Idx; /**< Mark Idx as unused to avoid compiler warning */

	/** - Perform SMC call for EL1 non-secure AArch64 applications */
	Status = XPuf_SmcCall(PayloadBuf, PayloadLen, ResponseBuf, ResponseLen);
#else
	/** - For other environments, use IPI mailbox communication. */
	/** - Validate mailbox pointer for IPI communication */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Send request through IPI mailbox */
	Status = XPuf_ProcessMailbox(InstancePtr->MailboxPtr, PayloadBuf, PayloadLen,
				ResponseBuf);
	/** - On failure, return without shifting/mutating ResponseBuf. */
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Cap ResponseLen: the mailbox shift below consumes Response[0], leaving
	 *   RESPONSE_ARG_CNT - 1 data slots reachable via ResponseBuf[Idx + 1U].
	 */
	if (ResponseLen > ((u32)RESPONSE_ARG_CNT - 1U)) {
		ResponseLen = (u32)RESPONSE_ARG_CNT - 1U;
	}

	/**
	 * - Shift IPI response data in ResponseBuf to skip Response[0] (PLM exec status,
	 * returned via Status) so ResponseBuf[0] holds the first data word, matching
	 * the SMC path layout. XPuf_ProcessMailbox() writes RESPONSE_ARG_CNT words when
	 * ResponseBuf is non-NULL.
	 */
	for (Idx = 0U; Idx < ResponseLen; Idx++) {
		ResponseBuf[Idx] = ResponseBuf[Idx + 1U];
	}
#endif

END:
	return Status;
}

/** @} */
