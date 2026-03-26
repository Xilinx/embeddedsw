/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_generic.c
 *
 * This file contains the implementation of the generic request API that handles
 * both SMC and IPI mailbox communication for xilplmi client library
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 2.4   tbk  03/24/26 Initial release for unified SMC/Mailbox handling
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xilplmi_client_apis XilPlmi Client APIs
 * @{
 */

/*************************************** Include Files *******************************************/
#include "xplmi_generic.h"
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
#include "xplmi_smc.h"
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
 * @param	InstancePtr	Pointer to the client instance (used for mailbox, can be NULL for SMC)
 * @param	PayloadBuf	Pointer to payload buffer containing command and arguments
 * @param	PayloadLen	Length of payload buffer (number of u32 elements)
 * @param	ResponseBuf	Pointer to store response values (can be NULL if response not needed)
 * @param	ResponseLen	Length of response buffer (number of u32 elements)
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XST_FAILURE on failure.
 *		- XST_INVALID_PARAM If input parameters are invalid
 *
 * @note	This is a generic API that internally handles SMC or Mailbox communication based on
 *		the build configuration.
 *
 *************************************************************************************************/
int XPlmi_SendRequest(XPlmi_ClientInstance *InstancePtr, u32 *PayloadBuf, u32 PayloadLen,
		u32 *ResponseBuf, u32 ResponseLen)
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
	Status = XPlmi_SmcCall(PayloadBuf, PayloadLen, ResponseBuf, ResponseLen);
#else
	/** Validate mailbox pointer for IPI communication */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** Send IPI request to PLM and wait for response */
	Status = XPlmi_ProcessMailbox(InstancePtr, PayloadBuf, PayloadLen);

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

/** @} End of xilplmi_client_apis group */
