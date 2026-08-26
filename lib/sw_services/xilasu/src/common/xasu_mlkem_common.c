/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_mlkem_common.c
 *
 * This file contains the ML-KEM function definitions which are common across the client and server.
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
 * @addtogroup xasu_mlkem_common_apis ML-KEM Common APIs
 * @{
*/

/************************************ Include Files **********************************************/
#include "xasu_mlkem_common.h"
#include "xasu_keymanager_common.h"

/********************************* Constant Definitions ******************************************/

/*********************************** Macros Definitions ******************************************/

/*********************************** Type Definitions ********************************************/

/********************************* Variable Definitions ******************************************/

/****************************** Inline Function Definitions **************************************/

/********************************** Function Prototypes ******************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates the ML-KEM encapsulation/decapsulation parameters.
 *
 * @param	MlkemParamsPtr	Pointer to the XAsu_MlkemEncapsDecapsParams structure which holds
 *				the ML-KEM encapsulation/decapsulation parameters.
 *
 * @return
 *		- XST_SUCCESS, if validation of ML-KEM parameters is successful.
 *		- XST_FAILURE, if any argument is invalid.
 *
 *************************************************************************************************/
s32 XAsu_MlkemValidateParams(const XAsu_MlkemEncapsDecapsParams *MlkemParamsPtr)
{
	s32 Status = XST_FAILURE;

	/** Validate ML-KEM params pointer. */
	if (MlkemParamsPtr == NULL) {
		goto END;
	}

	/** Validate the parameter set against the FIPS 203 approved sets. */
	if (MlkemParamsPtr->ParamSet > (u8)XASU_MLKEM_1024) {
		goto END;
	}

	/** Validate ciphertext address, which is the output for encaps and the input for decaps. */
	if (MlkemParamsPtr->CtAddr == 0U) {
		goto END;
	}

	/**
	 * Validate the shared secret address and buffer length, which is an output of both
	 * operations and is 32 bytes for every parameter set.
	 */
	if ((MlkemParamsPtr->SharedSecretAddr == 0U) ||
	    (MlkemParamsPtr->SharedSecretLen < XASU_MLKEM_SHARED_SECRET_LEN)) {
		goto END;
	}

	/** Validate that exactly one of KeyAddr or KeyId is provided. */
	if (XAsu_KmValidateKeyAddrNdKeyId(MlkemParamsPtr->KeyAddr,
					  MlkemParamsPtr->KeyId) != XST_SUCCESS) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the parameters of the ML-KEM encapsulation which takes the
 *		message m from the request, an algorithm validation only service.
 *
 * @param	MlkemParamsPtr	Pointer to the XAsu_MlkemEncapsDerandParams structure which holds
 *				the ML-KEM encapsulation parameters and the message m.
 *
 * @return
 *		- XST_SUCCESS, if validation of ML-KEM parameters is successful.
 *		- XST_FAILURE, if any argument is invalid.
 *
 *************************************************************************************************/
s32 XAsu_MlkemValidateEncapsDerandParams(const XAsu_MlkemEncapsDerandParams *MlkemParamsPtr)
{
	volatile s32 Status = XST_FAILURE;

	/** Validate ML-KEM params pointer. */
	if (MlkemParamsPtr == NULL) {
		goto END;
	}

	/** Validate the parameters this command shares with encapsulation. */
	Status = XAsu_MlkemValidateParams(&MlkemParamsPtr->EncapsParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;

	/**
	 * The encapsulation key comes from KeyAddr only, as the key vault is not reachable from a
	 * validation command.
	 */
	if ((MlkemParamsPtr->EncapsParams.KeyAddr == 0U) ||
	    (MlkemParamsPtr->EncapsParams.KeyId != 0U)) {
		goto END;
	}

	/**
	 * m must be supplied: a zero address is an error here rather than a request to draw from
	 * the TRNG.
	 */
	if ((MlkemParamsPtr->SeedAddr == 0U) ||
	    (MlkemParamsPtr->SeedLen != XASU_MLKEM_ENCAPS_SEED_LEN)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
