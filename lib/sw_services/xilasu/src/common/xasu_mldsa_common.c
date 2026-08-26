/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_mldsa_common.c
 *
 * This file contains the ML-DSA function definitions which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/21/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_mldsa_common_apis ML-DSA Common APIs
 * @{
*/

/************************************ Include Files **********************************************/
#include "xasu_mldsa_common.h"
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
 * @brief	This function validates the ML-DSA signature generation/verification parameters.
 *
 * @param	MldsaParamsPtr	Pointer to the XAsu_MldsaSignVerifyParams structure which holds
 *				the ML-DSA signature generation/verification parameters.
 *
 * @return
 *		- XST_SUCCESS, if validation of ML-DSA parameters is successful.
 *		- XST_FAILURE, if any argument is invalid.
 *
 *************************************************************************************************/
s32 XAsu_MldsaValidateParams(const XAsu_MldsaSignVerifyParams *MldsaParamsPtr)
{
	s32 Status = XST_FAILURE;

	/** Validate ML-DSA params pointer. */
	if (MldsaParamsPtr == NULL) {
		goto END;
	}

	/** Validate the parameter set against the FIPS 204 approved sets. */
	if (MldsaParamsPtr->ParamSet > (u8)XASU_MLDSA_87) {
		goto END;
	}

	/** Validate signature address, which is the output for sign and the input for verify. */
	if (MldsaParamsPtr->SignatureAddr == 0U) {
		goto END;
	}

	/**
	 * Validate message address and length. MsgAddr must be a valid (non-zero) buffer pointer
	 * even for an empty message (MsgLen == 0), to match the firmware contract that lets the
	 * message stream through the SHA engine: a zero-length update on a valid address is a
	 * no-op, while a zero address is rejected by the SHA driver.
	 */
	if ((MldsaParamsPtr->MsgAddr == 0U) ||
	    (MldsaParamsPtr->MsgLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH)) {
		goto END;
	}

	/** Validate the context string length and its address when a context is supplied. */
	if ((MldsaParamsPtr->ContextLen > XASU_MLDSA_MAX_CTX_LEN) ||
	    ((MldsaParamsPtr->ContextLen != 0U) && (MldsaParamsPtr->ContextAddr == 0U))) {
		goto END;
	}

	/** Validate that exactly one of KeyAddr or KeyId is provided. */
	if (XAsu_KmValidateKeyAddrNdKeyId(MldsaParamsPtr->KeyAddr,
					  MldsaParamsPtr->KeyId) != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Validate the signing mode before the mode dependent checks below, so that an undefined
	 * mode is rejected here rather than travelling to the server to be rejected there.
	 */
	if (MldsaParamsPtr->SignMode > (u8)XASU_MLDSA_SIGN_MODE_PREHASH) {
		goto END;
	}

	/** Validate the pre-hash SHA type and mode, which apply only to HashML-DSA. */
	if (MldsaParamsPtr->SignMode == (u8)XASU_MLDSA_SIGN_MODE_PREHASH) {
		if (XAsu_ShaValidateModeAndType(MldsaParamsPtr->ShaType,
						MldsaParamsPtr->ShaMode) != XST_SUCCESS) {
			goto END;
		}
	}

	/**
	 * Validate the randomness mode, which selects hedged or deterministic signing and is
	 * unused for verification. An out of range value would otherwise be taken as hedged
	 * rather than rejected, silently signing in a mode the request did not ask for.
	 */
	if (MldsaParamsPtr->Deterministic > (u8)XASU_MLDSA_DETERMINISTIC) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the parameters of the ML-DSA signature generation which
 *		takes its rnd from the request, an algorithm validation only service.
 *
 * @param	MldsaParamsPtr	Pointer to the XAsu_MldsaSignDerandParams structure which holds the
 *				ML-DSA signature generation parameters and the rnd to sign with.
 *
 * @return
 *		- XST_SUCCESS, if validation of ML-DSA parameters is successful.
 *		- XST_FAILURE, if any argument is invalid.
 *
 *************************************************************************************************/
s32 XAsu_MldsaValidateSignDerandParams(const XAsu_MldsaSignDerandParams *MldsaParamsPtr)
{
	volatile s32 Status = XST_FAILURE;

	/** Validate ML-DSA params pointer. */
	if (MldsaParamsPtr == NULL) {
		goto END;
	}

	/** Validate the signing parameters this command shares with signature generation. */
	Status = XAsu_MldsaValidateParams(&MldsaParamsPtr->SignParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;

	/**
	 * The secret key comes from KeyAddr only, as the key vault is not reachable from a
	 * validation command.
	 */
	if ((MldsaParamsPtr->SignParams.KeyAddr == 0U) ||
	    (MldsaParamsPtr->SignParams.KeyId != 0U)) {
		goto END;
	}

	/**
	 * rnd must be supplied: a zero address is an error here rather than a request to draw
	 * from the TRNG.
	 */
	if ((MldsaParamsPtr->RndAddr == 0U) ||
	    (MldsaParamsPtr->RndLen != XASU_MLDSA_SIGN_RND_LEN)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
