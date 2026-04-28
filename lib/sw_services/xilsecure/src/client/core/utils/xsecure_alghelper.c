/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file client/core/utils/xsecure_alghelper.c
*
* This file contains algorithm helper functions for XilSecure library.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------------------------------
* 5.7   tbk     02/05/26 Initial Release
*
* </pre>
*
***************************************************************************************************/

/****************************************** Include Files *****************************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xsecure_alghelper.h"
#include "xsecure_mailbox.h"
#include "xsecure_plat_defs.h"
#include "xilmailbox.h"

/************************************** Constant Definitions **************************************/
#define XSECURE_RESPONSE_STATUS_IDX	0U	/**< Response status index */

/**************************************** Type Definitions ****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/*************************************** Function Prototypes **************************************/

/************************************** Variable Definitions **************************************/

/************************************** Function Definitions **************************************/

/**************************************************************************************************/
/**
*
* @brief	Retrieves cryptographic algorithm information from PLM.
*
* @param	AlgInfo		Pointer to Xil_CryptoAlgInfo structure
* @param	CryptoAlg	Cryptographic algorithm identifier
*
* @return
*		- XST_SUCCESS - On success
*		- XST_INVALID_PARAM - If input parameters are invalid
*		- Error - On failure
*
***************************************************************************************************/
int XSecure_GetAlgInfo(Xil_CryptoAlgInfo *AlgInfo, u32 CryptoAlg)
{
	volatile int Status = XST_FAILURE;
	XMailbox MailboxInst;
	XSecure_ClientInstance SecureInst;
	u32 Payload[PAYLOAD_ARG_CNT];
	u32 Response[RESPONSE_ARG_CNT];

	/* Validate input parameter */
	if (AlgInfo == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Initialize mailbox */
	Status = XMailbox_Initialize(&MailboxInst, 0U);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_ClientInit(&SecureInst, &MailboxInst);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Prepare IPI Payload */
	XSECURE_PACK_PAYLOAD1(Payload, XSECURE_API_FEATURES, CryptoAlg);

	/* Send IPI and wait for response */
	Status = (int)XMailbox_SendData(SecureInst.MailboxPtr, XSECURE_TARGET_IPI_INT_MASK,
				Payload, (u32)(sizeof(Payload) / sizeof(u32)),
				XILMBOX_MSG_TYPE_REQ, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Receive response with algorithm version */
	Status = (int)XMailbox_Recv(SecureInst.MailboxPtr, XSECURE_TARGET_IPI_INT_MASK,
				Response, RESPONSE_ARG_CNT, XILMBOX_MSG_TYPE_RESP);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Extract status and version from response */
	Status = (int)Response[XSECURE_RESPONSE_STATUS_IDX];
	if (Status == XST_SUCCESS) {
		AlgInfo->Version = Response[XSECURE_ALGO_VERSION_RESP_INDEX];
		AlgInfo->NistStatus = Response[XSECURE_NIST_STATUS_RESP_INDEX];
	}

END:
	return Status;
}
