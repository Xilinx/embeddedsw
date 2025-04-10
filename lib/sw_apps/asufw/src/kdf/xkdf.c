/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xkdf.c
*
* This file contains the implementation of the HMAC based Key Derivation Function (HKDF) APIs
* using counter mode as specified in NIST SP 800-108r1.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ma   01/15/25 Initial release
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
 * 	- Errors codes from HMAC, if HMAC operation fails.
 *
 *************************************************************************************************/
s32 XKdf_Generate(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, const XAsu_KdfParams *KdfParams)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XHmac *HmacPtr = XHmac_GetInstance();
	static u32 Iterations = 0U;
	static u32 KdfIndex = 0U;
	u32 KdfValue;
	static u32 HashLen = 0U;
	u8 KOut[XASU_SHA_512_HASH_LEN];
	static u64 KeyOutAddr = 0U;

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
	 * Use HMAC as pseudorandom function, run the below steps in iterations until requested bytes
	 * of key output is generated and form the final output.
	 */
	KeyOutAddr = KdfParams->KeyOutAddr;
	for (KdfIndex = 0U; KdfIndex < Iterations; ++KdfIndex) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		/** - Initialize HMAC with KeyIn provided. */
		Status = XHmac_Init(HmacPtr, DmaPtr, ShaInstancePtr, KdfParams->KeyInAddr,
				KdfParams->KeyInLen, KdfParams->ShaMode, HashLen);
		if (Status != XASUFW_SUCCESS) {
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
			goto END_CLR;
		}

		/** - Update context provided by user to HMAC. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XHmac_Update(HmacPtr, DmaPtr, KdfParams->ContextAddr, KdfParams->ContextLen,
				XASU_TRUE);
		if (Status != XASUFW_SUCCESS) {
			goto END_CLR;
		}

		/** - Get final HMAC for the running iteration. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XHmac_Final(HmacPtr, DmaPtr, (u32 *)KOut);
		if (Status != XASUFW_SUCCESS) {
			goto END_CLR;
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
/** @} */
