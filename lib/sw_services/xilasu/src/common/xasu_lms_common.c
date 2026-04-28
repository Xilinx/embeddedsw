/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_lms_common.c
 *
 * This file contains the LMS function definitions which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   03/24/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_lms_common_apis LMS Common APIs
 * @{
*/

/************************************ Include Files **********************************************/
#include "xasu_lms_common.h"
#include "xasu_shainfo.h"
#include "xasu_def.h"
#include "xasu_keymanager_common.h"

/********************************* Constant Definitions ******************************************/

/*********************************** Macros Definitions ******************************************/

/*********************************** Type Definitions ********************************************/

/********************************* Variable Definitions ******************************************/

/****************************** Inline Function Definitions **************************************/

/********************************** Function Prototypes ******************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates the LMS signature verification parameters.
 *
 * @param	LmsParamsPtr	Pointer to the XAsu_LmsHssSignVerifyParams structure which holds
 *				the LMS signature verification parameters.
 *
 * @return
 *		- XST_SUCCESS, if validation of LMS parameters is successful.
 *		- XST_FAILURE, if any argument is invalid.
 *
 *************************************************************************************************/
s32 XAsu_LmsValidateParams(const XAsu_LmsHssSignVerifyParams *LmsParamsPtr)
{
	s32 Status = XST_FAILURE;

	/** Validate LMS params pointer. */
	if (LmsParamsPtr == NULL) {
		goto END;
	}

	/** Validate message address. */
	if (LmsParamsPtr->MsgAddr == 0U) {
		goto END;
	}

	/** Validate signature address and length. */
	if ((LmsParamsPtr->SignatureAddr == 0U) || (LmsParamsPtr->SignatureLen == 0U)) {
		goto END;
	}

	/**
	 * Validate signature length against NIST SP 800-208 bounds.
	 * Minimum: 780 bytes (smallest LMS param set: N=24, W=8, H=5).
	 * Maximum: 18708 bytes (largest 2-level HSS: N=32, W=1, H=25).
	 */
	if ((LmsParamsPtr->SignatureLen < XASU_LMS_MIN_SIGNATURE_SIZE) ||
		(LmsParamsPtr->SignatureLen > XASU_LMS_MAX_SIGNATURE_SIZE)) {
		goto END;
	}

	/** Validate that exactly one of PubKeyAddr or PubKeyId is provided. */
	if (XAsu_KmValidateKeyAddrNdKeyId(LmsParamsPtr->LmsHssKeyObj.PubKeyAddr,
					  LmsParamsPtr->LmsHssKeyObj.PubKeyId) != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Validate public key length is within NIST SP 800-208 valid range.
	 * Min: 48 bytes (LMS N=24), Max: 60 bytes (HSS N=32) if PublicKeyAddr is provided.
	 */
	if ((LmsParamsPtr->LmsHssKeyObj.PubKeyAddr != 0U) &&
	    ((LmsParamsPtr->LmsHssKeyObj.PubKeyLen < XASU_LMS_PUB_KEY_SIZE) ||
	    (LmsParamsPtr->LmsHssKeyObj.PubKeyLen > XASU_LMS_MAX_PUB_KEY_SIZE))) {
		goto END;
	}

	/** Validate PreHashedMsg flag value. */
	if ((LmsParamsPtr->PreHashedMsg != XASU_LMS_MSG_NOT_PREHASHED) &&
		(LmsParamsPtr->PreHashedMsg != XASU_LMS_MSG_PREHASHED)) {
		goto END;
	}

	/**
	 * Validate message length for pre-hashed case per NIST SP 800-208.
	 * When message is NOT pre-hashed: any length is valid per RFC 8554.
	 * When message IS pre-hashed: length must equal a NIST-valid hash output size
	 * (24 bytes for N=24 parameter sets, 32 bytes for N=32 parameter sets).
	 */
	if ((LmsParamsPtr->PreHashedMsg == XASU_LMS_MSG_PREHASHED) &&
		(LmsParamsPtr->MsgLen != XASU_LMS_HASH_LEN_N32) &&
		(LmsParamsPtr->MsgLen != XASU_LMS_HASH_LEN_N24)) {
		goto END;
	}

	/** Validate message length doesn't exceed DMA max transfer length. */
	if (LmsParamsPtr->MsgLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH) {
		goto END;
	}

	/**
	 * Validate SHA type and mode per NIST SP 800-208 / RFC 8554.
	 * LMS only permits two hash functions:
	 *  - SHA-256: ShaType = XASU_SHA2_TYPE, ShaMode = XASU_SHA_MODE_256
	 *  - SHAKE256: ShaType = XASU_SHA3_TYPE, ShaMode = XASU_SHA_MODE_SHAKE256
	 * No other combinations (SHA-384, SHA-512, SHA3-256, etc.) are defined
	 * in any LMS/HSS parameter set.
	 */
	if (((LmsParamsPtr->ShaType == XASU_SHA2_TYPE) &&
		(LmsParamsPtr->ShaMode != XASU_SHA_MODE_256)) ||
		((LmsParamsPtr->ShaType == XASU_SHA3_TYPE) &&
		(LmsParamsPtr->ShaMode != XASU_SHA_MODE_SHAKE256)) ||
		((LmsParamsPtr->ShaType != XASU_SHA2_TYPE) &&
		(LmsParamsPtr->ShaType != XASU_SHA3_TYPE))) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
