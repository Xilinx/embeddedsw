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

	/** Validate public key address and length. */
	if ((LmsParamsPtr->PublicKeyAddr == 0U) || (LmsParamsPtr->PublicKeyLen == 0U)) {
		goto END;
	}

	/** Validate PreHashedMsg flag value. */
	if ((LmsParamsPtr->PreHashedMsg != XASU_LMS_MSG_NOT_PREHASHED) &&
		(LmsParamsPtr->PreHashedMsg != XASU_LMS_MSG_PREHASHED)) {
		goto END;
	}

	/**
	 * Validate message length for pre-hashed case.
	 * When message is NOT pre-hashed: zero-length message is valid per RFC 8554.
	 * When message IS pre-hashed: length must equal the hash output size (32 bytes).
	 */
	if ((LmsParamsPtr->PreHashedMsg == XASU_LMS_MSG_PREHASHED) &&
		(LmsParamsPtr->MsgLen != XASU_SHA_SHAKE_256_HASH_LEN)) {
		goto END;
	}

	/** Validate message length doesn't exceed DMA max transfer length. */
	if (LmsParamsPtr->MsgLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH) {
		goto END;
	}

	/** Validate public key length is at least the minimum LMS public key size. */
	if (LmsParamsPtr->PublicKeyLen < XASU_LMS_PUB_KEY_SIZE) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
