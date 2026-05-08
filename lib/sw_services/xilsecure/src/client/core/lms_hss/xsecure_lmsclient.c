/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file client/core/lms_hss/xsecure_lmsclient.c
*
* This file contains the implementation of LMS client interface functions.
* It packages an IPI request that asks the server-side LMS handler
* (XSecure_LmsSignVerifyIpi -> XSecure_LmsSignatureVerification) to verify
* an LMS signature against a message and an expected public key.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  05/03/26 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_lms_client_apis XilSecure LMS Client APIs
* @{
*/

/*************************************** Include Files ********************************************/
#include "xsecure_lmsclient.h"
#include "xsecure_generic.h"

/************************************ Constant Definitions ****************************************/

/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Function Prototypes *****************************************/

/************************************ Variable Definitions ****************************************/

/**************************************************************************************************/
/**
 *
 * @brief	This function sends an IPI request to perform an LMS signature
 *		verification operation. The data, signature and public-key
 *		buffers must be reachable by the server (i.e. placed in shared
 *		memory or DDR that the PLM can access) and their addresses /
 *		lengths must already be populated in the params structure.
 *		The caller is responsible for keeping ParamsPtr (and the
 *		buffers it references) alive and coherent until this call
 *		returns.
 *
 * @param	InstancePtr	Pointer to the client instance.
 * @param	ParamsPtr	Pointer to a populated
 *				XSecure_LmsParams structure.
 *
 * @return
 *		 - XST_SUCCESS	On successful signature verification.
 *		 - XST_INVALID_PARAM	If any input pointer is NULL.
 *		 - XST_FAILURE	If the verification or transport fails.
 *
 **************************************************************************************************/
int XSecure_LmsSignVerify(XSecure_ClientInstance *InstancePtr,
			 const XSecure_LmsParams *ParamsPtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];
	u64 Addr = (u64)(UINTPTR)ParamsPtr;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL) || (ParamsPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	XSECURE_PACK_PAYLOAD3(Payload, XSECURE_API_LMS_SIGN_VERIFY, (u32)Addr,
			      (u32)(Addr >> XSECURE_ADDR_HIGH_SHIFT),
			      (u32)sizeof(XSecure_LmsParams));

	Status = XSecure_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

END:
	return Status;
}
/** @} */
