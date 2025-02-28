/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_keywrap_common.c
 * @addtogroup Overview
 * @{
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
 *
 * </pre>
 *
 *************************************************************************************************/

/***************************** Include Files *****************************************************/
#include "xasu_keywrap_common.h"
#include "xasu_rsainfo.h"
#include "xasu_aesinfo.h"
#include "xasu_aes_common.h"

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
 * @param	KwpunwpParamsPtr	Pointer which holds the parameters of Key wrap unwrap
 * 					input arguments.
 *
 * @return
 *		- Upon successful validation of input parameters, it returns XST_SUCCESS.
 *		- XST_FAILURE on failure.
 *
 *************************************************************************************************/
s32 XAsu_KeyWrapUnwrapValidateInputParams(const XAsu_KeyWrapParams *KwpunwpParamsPtr)
{
	s32 Status = XST_FAILURE;

	if (KwpunwpParamsPtr == NULL) {
		goto END;
	}

	if ((KwpunwpParamsPtr->InputDataAddr == 0U) || (KwpunwpParamsPtr->OutputDataAddr == 0U)
	    || (KwpunwpParamsPtr->KeyCompAddr == 0U)
	    || (KwpunwpParamsPtr->OptionalLabelAddr == 0U)) {
		goto END;
	}

	if ((KwpunwpParamsPtr->InputDataLen == 0U) || (KwpunwpParamsPtr->OutuputDataLen == 0U)
	    || (KwpunwpParamsPtr->OptionalLabelSize == 0U)) {
		goto END;
	}

	if ((KwpunwpParamsPtr->AesKeySize != XASU_AES_KEY_SIZE_128_BITS) &&
		(KwpunwpParamsPtr->AesKeySize != XASU_AES_KEY_SIZE_256_BITS)) {
		goto END;
	}

	if ((KwpunwpParamsPtr->ShaMode != XASU_SHA_MODE_SHA256) &&
	    (KwpunwpParamsPtr->ShaMode != XASU_SHA_MODE_SHA384) &&
	    (KwpunwpParamsPtr->ShaMode != XASU_SHA_MODE_SHA512) &&
	    ((KwpunwpParamsPtr->ShaType != XASU_SHA3_TYPE) ||
	    (KwpunwpParamsPtr->ShaMode != XASU_SHA_MODE_SHAKE256))) {
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(KwpunwpParamsPtr->RsaKeySize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
