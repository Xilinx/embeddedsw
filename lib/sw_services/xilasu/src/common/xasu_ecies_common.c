/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_ecies_common.c
 *
 * This file contains the ECIES function definition which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date     Changes
 * ----- ----  -------- ----------------------------------------------------------------------------
 * 1.0   yog   02/18/25 Initial release
 *       yog   07/11/25 Added support for Edward curves.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_ecies_common_apis ECIES Common APIs
 * @{
*/

/***************************** Include Files *****************************************************/
#include "xasu_ecies_common.h"
#include "xasu_aesinfo.h"
#include "xasu_eccinfo.h"
#include "xasu_shainfo.h"
#include "xasu_kdfinfo.h"
#include "xasu_def.h"

/************************** Constant Definitions *************************************************/

/************************** Macros Definitions ***************************************************/

/**************************** Type Definitions ***************************************************/

/************************** Variable Definitions *************************************************/

/************************** Inline Function Definitions ******************************************/

/************************** Function Prototypes **************************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates the input parameters for ECIES.
 *
 * @param	EciesParams	Pointer to XAsu_EciesParams structure that holds the input parameters
 * 			for ECIES.
 *
 * @return
 * 	- XST_SUCCESS, if ECIES input parameters validation is successful.
 * 	- XST_FAILURE, if ECIES input parameters validation fails.
 *
 *************************************************************************************************/
s32 XAsu_ValidateEciesParameters(const XAsu_EciesParams *EciesParams)
{
	s32 Status = XST_FAILURE;

	if (EciesParams == NULL) {
		goto END;
	}
	/** Validate SHA Type and SHA Mode. */
	if ((EciesParams->ShaType != XASU_SHA2_TYPE) && (EciesParams->ShaType != XASU_SHA3_TYPE)) {
		goto END;
	}

	if ((EciesParams->ShaMode != XASU_SHA_MODE_256) &&
	    (EciesParams->ShaMode != XASU_SHA_MODE_384) &&
	    (EciesParams->ShaMode != XASU_SHA_MODE_512) &&
	    ((EciesParams->ShaType != XASU_SHA3_TYPE) ||
	    (EciesParams->ShaMode != XASU_SHA_MODE_SHAKE256))) {
		goto END;
	}
	/** Validate AES key size. */
	if (((EciesParams->AesKeySize != XASU_AES_KEY_SIZE_128_BITS) &&
	    (EciesParams->AesKeySize != XASU_AES_KEY_SIZE_256_BITS))) {
		goto END;
	}
	/** Validate IV and MAC length parameters. */
	if ((EciesParams->IvLength != XASU_AES_IV_SIZE_96BIT_IN_BYTES) ||
	    (EciesParams->MacLength != XASU_AES_MAX_TAG_LENGTH_IN_BYTES)) {
		goto END;
	}
	/** Validate context and data lengths. */
	if ((EciesParams->ContextLen == 0U) ||
	    (EciesParams->ContextLen > XASU_KDF_MAX_CONTEXT_LEN) ||
	    (EciesParams->DataLength == 0U) ||
	    (EciesParams->DataLength > XASU_ASU_DMA_MAX_TRANSFER_LENGTH)) {
		goto END;
	}
	/** Validate ECC curve type. */
	if (XAsu_EccValidateCurveInfo(EciesParams->EccCurveType, EciesParams->EccKeyLength) !=
			XST_SUCCESS) {
		goto END;
	}
	/** Validate that the addresses of all input and output buffers are non-zero. */
	if ((EciesParams->InDataAddr == 0U) || (EciesParams->IvAddr == 0U) ||
	    (EciesParams->MacAddr == 0U) || (EciesParams->OutDataAddr == 0U) ||
	    (EciesParams->TxKeyAddr == 0U) || (EciesParams->RxKeyAddr == 0U) ||
	    (EciesParams->ContextAddr == 0U)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the given ECC curve type and curve length.
 *
 * @param	CurveType	Curve type provided.
 * @param	CurveLen	Curve length provided.
 *
 * @return
 * 		- XST_SUCCESS, if curve type is valid.
 * 		- XST_FAILURE, if curve length is invalid.
 *
 *************************************************************************************************/
s32 XAsu_EccValidateCurveInfo(u32 CurveType, u32 CurveLen)
{
	s32 Status = XST_FAILURE;
	u32 Len = 0U;

	switch (CurveType) {
		case XASU_ECC_NIST_P192:
			Len = XASU_ECC_P192_SIZE_IN_BYTES;
			break;
		case XASU_ECC_NIST_P224:
			Len = XASU_ECC_P224_SIZE_IN_BYTES;
			break;
		case XASU_ECC_NIST_P256:
		case XASU_ECC_BRAINPOOL_P256:
		case XASU_ECC_NIST_ED25519:
		case XASU_ECC_NIST_ED25519_PH:
			Len = XASU_ECC_P256_SIZE_IN_BYTES;
			break;
		case XASU_ECC_BRAINPOOL_P320:
			Len = XASU_ECC_P320_SIZE_IN_BYTES;
			break;
		case XASU_ECC_NIST_P384:
		case XASU_ECC_BRAINPOOL_P384:
			Len = XASU_ECC_P384_SIZE_IN_BYTES;
			break;
		case XASU_ECC_BRAINPOOL_P512:
			Len = XASU_ECC_P512_SIZE_IN_BYTES;
			break;
		case XASU_ECC_NIST_P521:
			Len = XASU_ECC_P521_SIZE_IN_BYTES;
			break;
		case XASU_ECC_NIST_ED448:
		case XASU_ECC_NIST_ED448_PH:
			Len = XASU_ECC_P448_SIZE_IN_BYTES;
			break;
		default:
			Len = 0U;
			break;
	}

	if ((Len != 0U) && (CurveLen == Len)) {
		Status = XST_SUCCESS;
	}

	return Status;
}
/** @} */
