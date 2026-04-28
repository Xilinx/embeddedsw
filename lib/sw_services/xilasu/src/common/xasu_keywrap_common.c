/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_keywrap_common.c
 *
 * This file contains the Key wrap unwrap function definitions which are common across
 * client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   02/24/25 Initial release
 *       yog  01/28/26 Added RSA key ID validation.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_keywrap_common_apis KeyWrap Common APIs
 * @{
*/

/***************************** Include Files *****************************************************/
#include "xasu_keywrap_common.h"
#include "xasu_rsainfo.h"
#include "xasu_aesinfo.h"
#include "xasu_aes_common.h"
#include "xasu_keymanager_common.h"

/************************** Constant Definitions *************************************************/

/************************** Macros Definitions ***************************************************/

/**************************** Type Definitions ***************************************************/

/************************** Variable Definitions *************************************************/

/************************** Inline Function Definitions ******************************************/

/************************** Function Prototypes **************************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates input parameters for key wrap and unwrap.
 *
 * @param	KwpunwpParamsPtr	Pointer to XAsu_KeyWrapParams structure that holds the input
 * 								parameters for Key wrap and unwrap
 * @param	OperationType		Indicates whether the operation is AES/RSA key wrap/unwrap or
 * 								AES key wrap/unwrap
 *
 * @return
 *	- XST_SUCCESS, if input validation is successful.
 *	- XST_FAILURE, if input validation fails.
 *
 *************************************************************************************************/
s32 XAsu_KeyWrapUnwrapValidateInputParams(const XAsu_KeyWrapParams *KwpunwpParamsPtr, u8 OperationType)
{
	volatile s32 Status = XST_FAILURE;

	if ((OperationType != XASU_KEYWRAP_AES_RSA_KWPUNWP) && (OperationType != XASU_KEYWRAP_AES_KWPUNWP)) {
		goto END;
	}

	if (KwpunwpParamsPtr == NULL) {
		goto END;
	}

	/** Validate that the addresses of all input and output buffers are non-zero. */
	if ((KwpunwpParamsPtr->InputDataAddr == 0U) || (KwpunwpParamsPtr->OutputDataAddr == 0U)
	    || ((KwpunwpParamsPtr->ActualOutputDataLenAddr == 0U))) {
		goto END;
	}

	/** Validate that the length of all input and output sizes are non-zero. */
	if ((KwpunwpParamsPtr->InputDataLen == 0U) || (KwpunwpParamsPtr->OutputDataLen == 0U)) {
		goto END;
	}

	/** Validate AES key size. */
	if ((KwpunwpParamsPtr->AesKeySize != XASU_AES_KEY_SIZE_128_BITS) &&
		(KwpunwpParamsPtr->AesKeySize != XASU_AES_KEY_SIZE_256_BITS)) {
		goto END;
	}

	if (OperationType == XASU_KEYWRAP_AES_RSA_KWPUNWP) {
		/** Validate that exactly one of KeyCompAddr or RsaKeyId is provided. */
		if (XAsu_KmValidateKeyAddrNdKeyId(KwpunwpParamsPtr->KeyCompAddr,
						  KwpunwpParamsPtr->RsaKeyId) != XST_SUCCESS) {
			goto END;
		}

		if (KwpunwpParamsPtr->OptionalLabelAddr == 0U) {
			goto END;
		}

		/** Validate SHA Mode and SHAType. */
		if (XAsu_ShaValidateModeAndType(KwpunwpParamsPtr->ShaType, KwpunwpParamsPtr->ShaMode) != XST_SUCCESS) {
			goto END;
		}

		/** Validate RSA key size. */
		Status = XAsu_RsaValidateKeySize(KwpunwpParamsPtr->RsaKeySize);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
