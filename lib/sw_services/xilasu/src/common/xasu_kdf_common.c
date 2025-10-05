/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_kdf_common.c
 *
 * This file contains the KDF function definitions which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  09/11/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_kdf_common_apis KDF Common APIs
 * @{
*/

/************************************ Include Files **********************************************/
#include "xasu_kdf_common.h"
#include "xasu_kdfinfo.h"
#include "xasu_shainfo.h"
#include "xasu_hmacinfo.h"
#include "xstatus.h"

/********************************* Constant Definitions ******************************************/

/*********************************** Macros Definitions ******************************************/

/*********************************** Type Definitions ********************************************/

/********************************* Variable Definitions ******************************************/

/****************************** Inline Function Definitions **************************************/

/********************************** Function Prototypes ******************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates the KDF input parameters.
 *
 * @param	KdfParamsPtr	Pointer to XAsu_KdfParams structure which holds the parameters of
 * 				KDF input arguments.
 *
 * @return
 * 	- XST_SUCCESS, if KDF input parameters validation is successful.
 * 	- XST_FAILURE, if KDF input parameters validation fails.
 *
 *************************************************************************************************/
s32 XAsu_ValidateKdfParameters(const XAsu_KdfParams *KdfParamsPtr)
{
	s32 Status = XST_FAILURE;

	if (KdfParamsPtr == NULL) {
		goto END;
	}

	/** Validate SHA Mode and SHA Type. */
	if (XAsu_ShaValidateModeAndType(KdfParamsPtr->ShaType, KdfParamsPtr->ShaMode) !=
	    XST_SUCCESS) {
		goto END;
	}

	if ((KdfParamsPtr->KeyInAddr == 0U) || (KdfParamsPtr->KeyInLen == 0U) ||
		(KdfParamsPtr->KeyInLen > XASU_HMAC_MAX_KEY_LENGTH) ||
		(KdfParamsPtr->ContextAddr == 0U) || (KdfParamsPtr->ContextLen == 0U) ||
		(KdfParamsPtr->ContextLen > XASU_KDF_MAX_CONTEXT_LEN) ||
		(KdfParamsPtr->KeyOutAddr == 0U) || (KdfParamsPtr->KeyOutLen == 0U)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
