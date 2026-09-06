/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file client/core/generic/xsecure_generic.c
*
* This file contains the implementation of the generic request API that handles both SMC and IPI
* mailbox communication for xilsecure client library.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.7   tbk  03/24/26 Initial release for unified SMC/Mailbox handling
* 5.8   tus  08/21/26 Collected multi-word IPI response, shifted out the status word to
*                     match the SMC response layout, and added response buffer and
*                     length parameter validation
*
* </pre>
*
**************************************************************************************************/

/**
 * @addtogroup xsecure_client_apis XilSecure Client APIs
 * @{
 */

/*************************************** Include Files *******************************************/
#include "xsecure_generic.h"
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
#include "xsecure_smc.h"
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
 *		- XST_SUCCESS  If the request is successful
 *		- XST_FAILURE  If there is a failure
 *		- XST_INVALID_PARAM  If input parameters are invalid
 *
 * @note	This is a generic API that internally handles SMC or Mailbox communication based on
 *			the build configuration.
 *
 **************************************************************************************************/
int XSecure_SendRequest(const XSecure_ClientInstance *InstancePtr, u32 *PayloadBuf,
			u32 PayloadLen, u32 *ResponseBuf, u32 ResponseLen)
{
	volatile int Status = XST_FAILURE;
#if !(defined (__aarch64__) && (EL1_NONSECURE == 1))
	u32 Idx;
	u32 LocalLen = ResponseLen;
#endif

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
	/** - For AArch64 EL1 non-secure, use SMC call */
	(void)InstancePtr; /**< Mark InstancePtr as unused to avoid compiler warning */

	/** - Perform SMC call for EL1 non-secure AArch64 applications */
	Status = XSecure_SmcCall(PayloadBuf, PayloadLen, ResponseBuf, ResponseLen);
#else
	/** - Validate mailbox pointer for IPI communication */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * - Send request through IPI mailbox. The full IPI response payload is
	 *   collected into ResponseBuf (which must hold RESPONSE_ARG_CNT words when
	 *   non-NULL). ResponseBuf may be NULL when only the status word is needed.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, PayloadBuf, PayloadLen,
				ResponseBuf);
	/** - On failure, return without shifting/mutating ResponseBuf. */
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - The IPI response places the status word at index 0 and the command
	 *   payload from index 1 onwards. Shift the payload down by one word so
	 *   ResponseBuf[0] holds the first data word, matching the SMC response
	 *   layout and presenting an identical index format to callers.
	 */
	if (ResponseBuf != NULL) {
		/** - Clamp to the reachable data words (Response[0] is the status word). */
		if (LocalLen > ((u32)RESPONSE_ARG_CNT - 1U)) {
			LocalLen = (u32)RESPONSE_ARG_CNT - 1U;
		}
		for (Idx = 0U; Idx < LocalLen; Idx++) {
			ResponseBuf[Idx] = ResponseBuf[Idx + 1U];
		}
	}
#endif

END:
	return Status;
}

/** @} */
