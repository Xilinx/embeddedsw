/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
*       rmv  09/08/25 Updated Iterations calculation implementation in XKdf_Generate()
*       rmv  09/11/25 Added KDF parameter validation in XAsufw_KdfGenerate()
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
#include "xasu_kdf_common.h"
#include "xasufw_perf.h"
#include "xasufw_memory.h"
#include "xasu_generic.h"
#include "xil_sutil.h"

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
	/**
	 * Capture the start time of the KDF key generation operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_KDF_ID);

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
	if ((DmaPtr == NULL) || (ShaInstancePtr == NULL)) {
		Status = XASUFW_KDF_INVALID_PARAM;
		goto END;
	}

	/** Validate KDF parameters. */
	Status = XAsu_ValidateKdfParameters(KdfParams);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KDF_INVALID_PARAM;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
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
	Iterations = XIL_SCEILDIV(u32, KdfParams->KeyOutLen, HashLen);

	/**
	 * Use HMAC as a pseudorandom function, run the below steps in iterations until requested bytes
	 * of key output is generated and form the final output.
	 */
	KeyOutAddr = KdfParams->KeyOutAddr;
	for (KdfIndex = 0U; KdfIndex < Iterations; ++KdfIndex) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		/** - Initialize HMAC with KeyIn provided. */
		Status = XHmac_Init(HmacPtr, DmaPtr, ShaInstancePtr, KdfParams->KeyObject.KeyInAddr,
				KdfParams->KeyObject.KeyInLen, KdfParams->ShaMode, HashLen);
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
		 * - HMAC output size is the SHA algorithm hash length. KDF key output can be any length.
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

	/**
	 * Measure and print the performance time for the KDF key generation operation, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_KDF_ID);

END_CLR:
	/** Zeroize the intermediate KOut buffer and locally created KeyOutAddr variable. */
	KeyOutAddr = 0U;
	if (KeyOutAddr != 0U) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ZEROIZE_MEMSET_FAIL);
	}

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
 * @param	AesInstancePtr	Pointer to the XAes instance.
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
s32 XKdf_CmacGenerate(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, const XAsu_KdfParams *KdfParams,
		      u32 AesKeySrc)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	volatile u32 Iterations = 0U;
	volatile u32 KdfIndex = 0U;
	u32 KdfValue;
	u8 KOut[XASU_AES_MAX_TAG_LENGTH_IN_BYTES] = {0U};
	u64 KeyOutAddr = 0U;
	XAsu_AesKeyObject KeyObject;
	u32 KeyOutLen = 0U;

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (KdfParams == NULL) || (AesInstancePtr == NULL)) {
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

	KeyObject.KeySize = KdfParams->KeyObject.KeyInLen;
	KeyObject.KeySrc = AesKeySrc;
	KeyObject.KeyAddress = (u64)(UINTPTR)(KdfParams->KeyObject.KeyInAddr);

	/** Write AES key if the key source is a user key. */
	if (AesKeySrc <= XASU_AES_USER_KEY_7) {
		Status = XAes_WriteKey(AesInstancePtr, DmaPtr, &KeyObject);
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
		/* AES-CMAC mode does not require an IV, hence IvAddr and IvLen is set to 0U. */
		Status = XAes_Init(AesInstancePtr, DmaPtr, 0U, 0U, XASU_AES_CMAC_MODE,
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
	/** Zeroize the intermediate KOut buffer and locally created KeyOutAddr variable. */
	KeyOutAddr = 0U;
	if (KeyOutAddr != 0U) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ZEROIZE_MEMSET_FAIL);
	}

	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, KOut, XASU_AES_MAX_TAG_LENGTH_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates a KEK (Key Encryption Key) from efuse black key 0 using
 *		CMAC-KDF.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	Context		Pointer to the context string for KDF.
 * @param	ContextLen	Length of the context string.
 * @param	IvIncVal	IV increment value for black key decryption.
 * @param	KekLen		Length of the KEK to generate in bytes.
 * @param	Kek		Pointer to buffer where the generated KEK will be stored.
 *
 * @return
 *	- XASUFW_SUCCESS, if KEK generation is successful.
 *	- XASUFW_MEM_COPY_FAIL, if memory copy operation fails.
 *	- XASUFW_KDF_DECRYPT_BLACK_KEY_0_FAIL, if black key decryption fails.
 *
 *************************************************************************************************/
s32 XKdf_GenerateKekFromEfuse0(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, const u8 *Context,
			       u32 ContextLen, u8 IvIncVal, u32 KekLen, u8 *Kek)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XAsu_KdfParams KdfParams = {0U};
	u8 KekIv[XASU_AES_IV_SIZE_96BIT_IN_BYTES] = {0U};
	const u8 *IvPtr = (const u8 *)(UINTPTR)XASUFW_PLM_RTCA_EFUSE_0_IV_ADDR;

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (AesInstancePtr == NULL) || (Context == NULL) ||
	    (ContextLen == 0U) || (Kek == NULL) || (KekLen == 0U)) {
		Status = XASUFW_KDF_INVALID_PARAM;
		goto END;
	}

	/** Copy IV from RTCA to local buffer. */
	Status = Xil_SMemCpy(KekIv, XASU_AES_IV_SIZE_96BIT_IN_BYTES, IvPtr,
			     XASU_AES_IV_SIZE_96BIT_IN_BYTES, XASU_AES_IV_SIZE_96BIT_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Check if IV is non-zero. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_IsBufferNonZero(KekIv, XASU_AES_IV_SIZE_96BIT_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KDF_IV_IS_ZERO;
		goto END;
	}

	/** Update IV with the offset for black key decryption. */
	Xil_IncrementBuffer(KekIv, XASU_AES_IV_SIZE_96BIT_IN_BYTES, IvIncVal);

	/** Decrypt efuse black key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAes_DecryptEfuseBlackKey(AesInstancePtr, DmaPtr,
					   XAES_KEY_TO_BE_DEC_SEL_EFUSE_KEY_0_VALUE,
					   XASU_AES_KEY_SIZE_256_BITS, (u64)(UINTPTR)KekIv,
					   XASU_AES_IV_SIZE_96BIT_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KDF_DECRYPT_BLACK_KEY_0_FAIL);
		goto END;
	}

	/** Update KDF parameters. */
	KdfParams.KeyObject.KeyInAddr = 0U;
	KdfParams.ContextAddr = (u64)(UINTPTR)Context;
	KdfParams.KeyOutAddr = (u64)(UINTPTR)Kek;
	KdfParams.KeyObject.KeyInLen = XASU_AES_KEY_SIZE_256_BITS;
	KdfParams.ContextLen = ContextLen;
	KdfParams.KeyOutLen = KekLen;

	/** Generate KEK using CMAC-KDF. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKdf_CmacGenerate(DmaPtr, AesInstancePtr, &KdfParams, XASU_AES_EFUSE_KEY_RED_0);

END:
	return Status;
}
/** @} */
