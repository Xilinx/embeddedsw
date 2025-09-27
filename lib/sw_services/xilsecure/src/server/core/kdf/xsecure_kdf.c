/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_kdf.c
* This file contains the implementation of the HMAC based Key Derivation Function (HKDF).
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.5   tvp  05/13/25 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_kdf_server_apis Xilsecure KDF Server APIs
* @{
*/

/*************************************** Include Files ********************************************/
#include "xsecure_kdf.h"
#include "xsecure_sha_common.h"
#include "xsecure_hmac.h"
#include "xsecure_init.h"

/**************************************************************************************************/
/**
 *
 * @brief
 * This function performs the HMAC based KDF.
 *
 * @param	InDataPtr is pointer to the XSecure_KdfParams instance.
 *		one should initialize the structure with following parameters
 *		before calling this API.
 *		- Key - Is a pointer to the key
 * 		- KeyLen - length of key pointer
 * 		- Context - Is a pointer to the context for KDF
 * 		- ContextLen - Specifies the length of the context pointer.
 * @param	KdfOut is the pointer to output storage.
 * @param	KdfOutLen is the size of output.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error Code on failure.
 *
 **************************************************************************************************/
int XSecure_Hkdf(XSecure_KdfParams *InDataPtr, u8 *KdfOut, u32 KdfOutLen)
{
	int Status = XST_FAILURE;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	XSecure_Hmac HmacInstance;
	u32 KdfValue = 0U;
	u32 Iterations = 0U;
	u32 KdfIndex = 0U;
	XSecure_HmacRes Hmac;

	/** Validate input parameters. */
	if (InDataPtr == NULL) {
		Status = XSECURE_ERR_HKDF_INVALID_PARAM;
		goto END;
	}

	/**
	 * Calculate number of iterations based on HKDF output key length and SHA hash length.
	 */
	Iterations = (u32)Xil_Ceil((float)KdfOutLen / XSECURE_HASH_SIZE_IN_BYTES);
	if (Iterations == 0U) {
		Status = XSECURE_ERR_HKDF_INVALID_PARAM;
		goto END;
	}

	if (Iterations > XSECURE_HKDF_MAX_ITERATIONS) {
		Status = XSECURE_ERR_HKDF_INVALID_PARAM;
		goto END;
	}

	for(KdfIndex = 0U; KdfIndex < Iterations; KdfIndex++)
	{
		/* Calculate HMAC and form KdfInput */
		Status = XSecure_HmacInit(&HmacInstance, Sha3InstPtr, (u64)(UINTPTR)InDataPtr->Key,
				InDataPtr->KeyLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/** - Update block index of 1 byte to HMAC. */
		KdfValue = KdfIndex + XSECURE_HKDF_BLOCK_INDEX_LENGTH;

		Status = XSecure_HmacUpdate(&HmacInstance, (u64)(UINTPTR)&KdfValue,
				XSECURE_WORD_SIZE);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/* Update Context */
		Status = XSecure_HmacUpdate(&HmacInstance, (u64)(UINTPTR)InDataPtr->Context,
				InDataPtr->ContextLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/* Get final HMAC */
		Status = XSecure_HmacFinal(&HmacInstance, &Hmac);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/**
		 * - HMAC output size is the SHA algorithm hash length. HKDF key output can be any
		 * length.
		 * - In the last iteration, data to be copied to the destination can be less than or
		 * equal to SHA algorithm hash length which varies based on the requested key
		 * output length.
		 */
		if (KdfIndex == (Iterations - XSECURE_HKDF_BLOCK_INDEX_LENGTH)) {
			Status = Xil_SMemCpy((u8 *)&KdfOut[XSECURE_HASH_SIZE_IN_BYTES * KdfIndex],
					(KdfOutLen - (KdfIndex * XSECURE_HASH_SIZE_IN_BYTES)),
					Hmac.Hash, XSECURE_HASH_SIZE_IN_BYTES,
					KdfOutLen - (KdfIndex * XSECURE_HASH_SIZE_IN_BYTES));
		} else {

			/**
			 * - Copy the final HMAC of each iteration to the destination key output
			 *   buffer.
			 */
			Status = Xil_SMemCpy((u8 *)&KdfOut[XSECURE_HASH_SIZE_IN_BYTES * KdfIndex],
					XSECURE_HASH_SIZE_IN_BYTES, Hmac.Hash,
					XSECURE_HASH_SIZE_IN_BYTES,
					XSECURE_HASH_SIZE_IN_BYTES);
		}

	}
END:
	return Status;
}
