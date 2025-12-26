/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_keymanager_common.c
 *
 * This file contains the Keymanager function definitions which are common across
 * client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   11/30/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_keymanager_common_apis KeyManager Common APIs
 * @{
*/

/***************************** Include Files *****************************************************/
#include "xasu_keymanager_common.h"
#include "xasu_aesinfo.h"

/************************** Constant Definitions *************************************************/

/************************** Macros Definitions ***************************************************/

/**************************** Type Definitions ***************************************************/

/************************** Variable Definitions *************************************************/

/************************** Inline Function Definitions ******************************************/

/************************** Function Prototypes **************************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates input parameters for keymanager.
 *
 * @param	KmParamsPtr	Pointer to XAsu_KeyManagerParams structure that holds the input
 *				parameters for Key manager
 *
 * @return
 *	- XST_SUCCESS, if input validation is successful.
 *	- XST_FAILURE, if input validation fails.
 *
 *************************************************************************************************/
s32 XAsu_KmValidateVaultParams(const XAsu_KeyManagerParams *KmParamsPtr)
{
	volatile s32 Status = XST_FAILURE;

	if (KmParamsPtr == NULL) {
		goto END;
	}

	/** Validate at least one output destination is provided. */
	if ((KmParamsPtr->KeyObjectAddr == 0U) && (KmParamsPtr->KeyIdAddr == 0U)) {
		goto END;
	}

	/** Validate key metadata. */
	if ((KmParamsPtr->KeyObjectAddr == 0U) && ((KmParamsPtr->KeyUseCase == 0U) ||
		(KmParamsPtr->UsageCount == 0U) || (KmParamsPtr->AccessRights == 0U))) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates key length parameters for a given key.
 *
 * @param	KmSubVaultParamPtr	Pointer to XAsu_KeyManagerParams structure that holds the input
 *					parameters for Key manager
 * @param	KeyType			Type of key to be validated.
 *
 * @return
 *	- XST_SUCCESS, if input validation is successful.
 *	- XST_FAILURE, if input validation fails.
 *
 *************************************************************************************************/
s32 XAsu_KmValidateKeyLength(const XAsu_KeyManagerParams *KmSubVaultParamPtr, u8 KeyType)
{
	volatile s32 Status = XST_FAILURE;

	/** Validate key length is either 128-bit or 256-bit for AES. */
	if (KeyType == XASU_KM_AES_KEYTYPE) {
		if ((KmSubVaultParamPtr->Length != XASU_AES_KEY_SIZE_128BIT_IN_BYTES) &&
		   (KmSubVaultParamPtr->Length != XASU_AES_KEY_SIZE_256BIT_IN_BYTES)) {
			goto END;
		}
	}

	/** Validate IV length is non-zero and does not exceed maximum size. */
	if (KeyType == XASU_KM_IV_KEYTYPE) {
		if ((KmSubVaultParamPtr->Length == 0U) ||
		    (KmSubVaultParamPtr->Length > XASU_AES_IV_SIZE_128BIT_IN_BYTES)) {
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
