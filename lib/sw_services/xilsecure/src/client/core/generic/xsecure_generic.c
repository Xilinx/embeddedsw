/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xsecure_generic.c
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
 * @param	ResponseBuf	Pointer to store response values (can be NULL)
 * @param	ResponseLen	Length of response buffer (number of u32 elements)
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

	/** Validate input parameters */
	if ((PayloadBuf == NULL) || (PayloadLen == 0U)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

#if defined (__aarch64__) && (EL1_NONSECURE == 1)
	/** For AArch64 EL1 non-secure, use SMC call */
	(void)InstancePtr; /**< Mark InstancePtr as unused to avoid compiler warning */

	/** Perform SMC call for EL1 non-secure AArch64 applications */
	Status = XSecure_SmcCall(PayloadBuf, PayloadLen, ResponseBuf, ResponseLen);
#else
	/** Validate mailbox pointer for IPI communication */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** Send request through IPI mailbox */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, PayloadBuf, PayloadLen);

	/**
	 * Note: For mailbox path, response is not directly returned.
	 * The caller should handle response retrieval if needed.
	 */
	(void)ResponseBuf; /* Unused in mailbox path */
	(void)ResponseLen; /* Unused in mailbox path */
#endif

END:
	return Status;
}

/** @} */
