/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xkdf.c
*
* This file contains the implementation of the HMAC based Key Derivation Function (HKDF) and CMAC
* based Key Derivation Function APIs using counter mode as specified in NIST SP 800-108r1.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ma   01/15/25 Initial release
*       yog  08/13/25 Added support for CMAC based KDF in counter mode.
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xkdf_server_apis KDF Server APIs
* @{
*/

/*************************************** Include Files *******************************************/
#include "xkdf.h"
#include "xhmac.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xfih.h"

#ifdef XASU_KDF_ENABLE
/************************************** Type Definitions *****************************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/

/************************************** Macros Definitions ***************************************/

/************************************** Function Definitions *************************************/

/*************************************************************************************************/
/**
 * @brief	This function performs KDF generate operation using HMAC as pseudorandom function with
 * the user provided inputs and generates the keying material object of specified number of bytes
 * in counter mode as specified in NIST SP 800-108r1.
 *
 * @param	DmaPtr			Pointer to the XAsufw_Dma instance.
 * @param	ShaInstancePtr	Pointer to the XSha instance.
 * @param	KdfParams		Pointer to the KDF structure containing user input parameters.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KDF generate operation is successful.
 * 	- XASUFW_KDF_INVALID_PARAM, if input parameters are invalid.
 * 	- XASUFW_KDF_ERROR, if any operation fails.
 * 	- XASUFW_DMA_COPY_FAIL, if DMA copy fails.
 * 	- Errors codes from HMAC, if HMAC operation fails.
 *
 *************************************************************************************************/
s32 XKdf_Generate(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, const XAsu_KdfParams *KdfParams)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XHmac *HmacPtr = XHmac_GetInstance();
	volatile u32 Iterations = 0U;
	volatile u32 KdfIndex = 0U;
	u32 KdfValue;
	u32 HashLen = 0U;
	u8 KOut[XASU_SHA_512_HASH_LEN];
	u64 KeyOutAddr = 0U;

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (KdfParams == NULL) || (ShaInstancePtr == NULL)) {
		Status = XASUFW_KDF_INVALID_PARAM;
		goto END;
	}

	if ((KdfParams->ContextAddr == 0U) ||
			(KdfParams->ContextLen == 0U) || (KdfParams->ContextLen > XASU_KDF_MAX_CONTEXT_LEN) ||
		(KdfParams->KeyOutAddr == 0U) || (KdfParams->KeyOutLen == 0U)) {
		Status = XASUFW_KDF_INVALID_PARAM;
		goto END;
	}

	/** Get SHA hash length based on SHA Mode. */
	Status = XSha_GetHashLen(KdfParams->ShaMode, &HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SHA_INVALID_SHA_MODE;
		goto END;
	}

	/**
	 * Calculate number of iterations based on KDF output key length and SHA algorithm hash length.
	 * According to algorithm, maximum iterations should not exceed (2^(counter size in bits) - 1).
	 * As in ASU, we are considering the counter size as 32-bits and taking only 32-bit Iterations
	 * variable, checking for maximum iterations is not required as this variable always holds
	 * only in range numbers.
	 */
	Iterations = KdfParams->KeyOutLen/HashLen;
	if ((KdfParams->KeyOutLen % HashLen) != 0x0U) {
		++Iterations;
	}

	/**
	 * Use HMAC as a pseudorandom function, run the below steps in iterations until requested bytes
	 * of key output is generated and form the final output.
	 */
	KeyOutAddr = KdfParams->KeyOutAddr;
	for (KdfIndex = 0U; KdfIndex < Iterations; ++KdfIndex) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		/** - Initialize HMAC with KeyIn provided. */
		Status = XHmac_Init(HmacPtr, DmaPtr, ShaInstancePtr, KdfParams->KeyInAddr,
				KdfParams->KeyInLen, KdfParams->ShaMode, HashLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KDF_ERROR);
			goto END_CLR;
		}

		/**
		 * - As we implement counter mode, give Iteration count as data to HMAC.
		 *   Iteration count should be sent in big endian format.
		 */
		KdfValue = Xil_Htonl(KdfIndex + 1U);
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XHmac_Update(HmacPtr, DmaPtr, (u64)(UINTPTR)&KdfValue, XASUFW_WORD_LEN_IN_BYTES,
				XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KDF_ERROR);
			goto END_CLR;
		}

		/** - Update context provided by the user to HMAC. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XHmac_Update(HmacPtr, DmaPtr, KdfParams->ContextAddr, KdfParams->ContextLen,
				XASU_TRUE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KDF_ERROR);
			goto END_CLR;
		}

		/** - Get final HMAC for the running iteration. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XHmac_Final(HmacPtr, DmaPtr, (u32 *)KOut);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KDF_ERROR);
			XFIH_GOTO(END_CLR);
		}

		/**
		 * - HMAC output size is the SHA algorithm hash length. KDF key output can be any legnth.
		 * Copy the final HMAC of each iteration to the destination key output buffer.
		 * In last iteration, data to be copied to the destination can be less than or equal to
		 * SHA algorithm hash length which varies based on the requested key output length.
		 */
		if (KdfIndex == (Iterations - 1U)) {
			HashLen = KdfParams->KeyOutLen - (KdfIndex * HashLen);
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)KOut, KeyOutAddr, HashLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END_CLR;
		}
		KeyOutAddr = KeyOutAddr + HashLen;
	}

	/** Check if the desired number of iterations are executed. */
	if (KdfIndex != Iterations) {
		Status = XASUFW_KDF_ITERATION_COUNT_MISMATCH;
	}

END_CLR:
	/** Zeroize the intermediate KOut buffer. */
	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, KOut, XASU_SHA_512_HASH_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}
#endif /* XASU_KDF_ENABLE */

/*************************************************************************************************/
/**
 * @brief	This function performs KDF generate operation using CMAC as pseudorandom function with
 * the user provided inputs and generates the keying material object of specified number of bytes
 * in counter mode as specified in NIST SP 800-108r1.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	KdfParams	Pointer to the KDF structure containing user input parameters.
 * @param	AesKeySrc	Aes Key source.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KDF generate operation is successful.
 * 	- XASUFW_KDF_INVALID_PARAM, if input parameters are invalid.
 * 	- XASUFW_KDF_ERROR, if any operation fails.
 * 	- XASUFW_DMA_COPY_FAIL, if DMA copy fails.
 * 	- Errors codes from AES-CMAC, if AES-CMAC operation fails.
 *
 *************************************************************************************************/
s32 XKdf_CmacGenerate(XAsufw_Dma *DmaPtr, const XAsu_KdfParams *KdfParams, u32 AesKeySrc)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XAes *AesInstancePtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
	volatile u32 Iterations = 0U;
	volatile u32 KdfIndex = 0U;
	u32 KdfValue;
	u8 KOut[XASU_AES_MAX_TAG_LENGTH_IN_BYTES] = {0U};
	u64 KeyOutAddr = 0U;
	XAsu_AesKeyObject KeyObject;
	u32 KeyOutLen = 0U;

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (KdfParams == NULL)) {
		Status = XASUFW_KDF_INVALID_PARAM;
		goto END;
	}

	if (AesKeySrc >= XASU_AES_MAX_KEY_SOURCES) {
		Status = XASUFW_KDF_INVALID_PARAM;
		goto END;
	}

	if ((KdfParams->ContextAddr == 0U) || (KdfParams->ContextLen == 0U) ||
	    (KdfParams->ContextLen > XASU_KDF_MAX_CONTEXT_LEN) || (KdfParams->KeyOutAddr == 0U) ||
	    (KdfParams->KeyOutLen == 0U)) {
		Status = XASUFW_KDF_INVALID_PARAM;
		goto END;
	}

	/**
	 * Calculate number of iterations based on KDF output key length and MAC output length.
	 * According to algorithm, maximum iterations should not exceed (2^(counter size in bits) - 1).
	 * As in ASU, we are considering the counter size as 32-bits and taking only 32-bit Iterations
	 * variable, checking for maximum iterations is not required as this variable always holds
	 * only in range numbers.
	 */
	Iterations = KdfParams->KeyOutLen / XASU_AES_MAX_TAG_LENGTH_IN_BYTES;
	if ((KdfParams->KeyOutLen % XASU_AES_MAX_TAG_LENGTH_IN_BYTES) != 0x0U) {
		++Iterations;
	}

	KeyObject.KeySize = KdfParams->KeyInLen;
	KeyObject.KeySrc = AesKeySrc;
	KeyObject.KeyAddress = (u64)(UINTPTR)(KdfParams->KeyInAddr);

	/** Write AES key if the key source is a user key. */
	if (AesKeySrc <= XASU_AES_USER_KEY_7) {
		Status = XAes_WriteKey(AesInstancePtr, DmaPtr, (u64)(UINTPTR)&KeyObject);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KDF_ERROR);
			goto END;
		}
	}
	/**
	 * Use CMAC as a pseudorandom function, run the below steps in iterations until requested bytes
	 * of key output is generated and form the final output.
	 */
	KeyOutAddr = KdfParams->KeyOutAddr;
	KeyOutLen = XASU_AES_MAX_TAG_LENGTH_IN_BYTES;
	for (KdfIndex = 0U; KdfIndex < Iterations; ++KdfIndex) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		/** - Initialize AES with KeyIn provided in CMAC mode. */
		Status = XAes_Init(AesInstancePtr, DmaPtr, (u64)(UINTPTR)&KeyObject, (u64)(UINTPTR)KOut,
				XASU_AES_MAX_TAG_LENGTH_IN_BYTES, XASU_AES_CMAC_MODE,
				XASU_AES_ENCRYPT_OPERATION);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KDF_ERROR);
			goto END_CLR;
		}

		/**
		 * - As we implement counter mode, give Iteration count as data to AES-CMAC.
		 *   Iteration count should be sent in big endian format.
		 */
		KdfValue = Xil_Htonl(KdfIndex + 1U);
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAes_Update(AesInstancePtr, DmaPtr, (u64)(UINTPTR)&KdfValue,
					XAES_AAD_UPDATE_NO_OUTPUT_ADDR, XASUFW_WORD_LEN_IN_BYTES,
					XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KDF_ERROR);
			goto END_CLR;
		}

		/** - Update context provided by the user to AES-CMAC. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAes_Update(AesInstancePtr, DmaPtr, KdfParams->ContextAddr,
					XAES_AAD_UPDATE_NO_OUTPUT_ADDR, KdfParams->ContextLen,
					XASU_TRUE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KDF_ERROR);
			goto END_CLR;
		}

		/** - Get final MAC out for the running iteration. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAes_Final(AesInstancePtr, DmaPtr, (u64)(UINTPTR)KOut,
					XASU_AES_MAX_TAG_LENGTH_IN_BYTES);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KDF_ERROR);
			XFIH_GOTO(END_CLR);
		}

		/**
		 * - CMAC output size is the MAC output length. KDF key output can be any length.
		 * Copy the final MAC of each iteration to the destination key output buffer.
		 * In last iteration, data to be copied to the destination can be less than or equal to
		 * MAC length which varies based on the requested key output length.
		 */
		if (KdfIndex == (Iterations - 1U)) {
			KeyOutLen = KdfParams->KeyOutLen - (KdfIndex * KeyOutLen);
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)KOut, KeyOutAddr, KeyOutLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END_CLR;
		}
		KeyOutAddr = KeyOutAddr + KeyOutLen;
	}

	/** Check if the desired number of iterations are executed. */
	if (KdfIndex != Iterations) {
		Status = XASUFW_KDF_ITERATION_COUNT_MISMATCH;
	}

END_CLR:
	/** Zeroize the intermediate KOut buffer. */
	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, KOut, XASU_AES_MAX_TAG_LENGTH_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;

}
/** @} */
