/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xhkdf.c
*
* This file contains the implementation of RFC based KDF algorithm (HKDF) APIs which uses HMAC as
* the Pseudo Random Function (PRF)
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   LP   04/07/25 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xhkdf_server_apis HKDF Server APIs
* @{
*/

/*************************************** Include Files *******************************************/
#include "xhkdf.h"
#include "xhmac.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xfih.h"

#ifdef XASU_ECIES_ENABLE
/************************************** Type Definitions *****************************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/

/************************************** Macros Definitions ***************************************/
#define XHKDF_MAX_CONTEXT_LEN		(1024U) /**< Maximum context length */
#define XHKDF_MAX_ITERATIONS		(0xFFU) /**< Maximum iterations */
#define XHKDF_BLOCK_INDEX_LENGTH	(0x01U) /**< Block index length */

/************************************** Function Definitions *************************************/
static s32 XHkdf_Extract(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
				const XAsu_HkdfParams *HkdfParams, u8 *Prk);
static s32 XHkdf_Expand(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
				const XAsu_HkdfParams *HkdfParams, u8 *Prk);

/*************************************************************************************************/
/**
 * @brief	This function performs HKDF operation to generate the derived key of specified key
 * length by using the RFC based Key Derivative Function (HKDF), with the user provided inputs in
 * extract and expand steps.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaInstancePtr	Pointer to the XSha instance.
 * @param	HkdfParams	Pointer to the HKDF structure containing user input parameters.
 *
 * @return
 * 	- XASUFW_SUCCESS, if HKDF generate operation is successful.
 * 	- XASUFW_HKDF_INVALID_PARAM, if input parameters are invalid.
 * 	- XASUFW_HKDF_EXTRACT_FAILED, if XHkdf_Extract operation fails.
 *
 *************************************************************************************************/
s32 XHkdf_Generate(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, const XAsu_HkdfParams *HkdfParams)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 Prk[XASU_SHA_512_HASH_LEN];

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (HkdfParams == NULL) || (ShaInstancePtr == NULL)) {
		Status = XASUFW_HKDF_INVALID_PARAM;
		goto END;
	}

	/** Validate keyout address, as it supports only 32-bit address range in server. */
	if (HkdfParams->KdfParams.KeyOutAddr > XASUFW_MAX_32BIT_ADDRESS) {
		Status = XASUFW_HKDF_INVALID_PARAM;
		goto END;
	}

	/**
	 * Extract Pseudo Random Key (PRK) using HMAC with the "salt" as the key and the "IKM" as
	 * the message.
	 */
	Status = XHkdf_Extract(DmaPtr, ShaInstancePtr, HkdfParams, (u8 *)&Prk);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HKDF_EXTRACT_FAILED);
		XFIH_GOTO(END_CLR);
	}

	/**
	 * Generate the keying material object of specified number of bytes by taking "PRK" as key,
	 * prepending the previous hash block to the "info" field and appending with an
	 * incrementing 8-bit counter as message.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XHkdf_Expand(DmaPtr, ShaInstancePtr, HkdfParams, (u8 *)&Prk);

END_CLR:
	/** Zeroize the intermediate Prk buffer. */
	Status = XAsufw_UpdateBufStatus(Status, Xil_SecureZeroize(Prk, XASU_SHA_512_HASH_LEN));

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function extracts pseudorandom key(PRK) using HMAC as pseudorandom function
 * with the input keying material and salt provided by user(salt is optional).
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaInstancePtr	Pointer to the XSha instance.
 * @param	HkdfParams	Pointer to the HKDF structure containing user input parameters.
 * @param	Prk		Pseudorandom key buffer generated from Extract method.
 *
 * @return
 * 	- XASUFW_SUCCESS, if HKDF extract operation was successful.
 * 	- XASUFW_HKDF_INVALID_PARAM, if input parameters are invalid.
 * 	- XASUFW_HKDF_GET_HASHLEN_FAILED, if XSha_GetHashLen operation fails.
 * 	- XASUFW_ZEROIZE_MEMSET_FAIL, if Xil_SMemSet operation fails.
 * 	- XASUFW_MEM_COPY_FAIL, if Xil_SMemCpy operation fails.
 * 	- XASUFW_HKDF_HMAC_INIT_FAILED, if XHmac_Init operation fails.
 * 	- XASUFW_HKDF_HMAC_UPDATE_FAILED, if XHmac_Update operation fails.
 *
**************************************************************************************************/
static s32 XHkdf_Extract(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
				const XAsu_HkdfParams *HkdfParams, u8 *Prk)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XHmac *HmacPtr = XHmac_GetInstance();
	u32 HashLen = 0U;
	u32 Salt = 0x00000000U;
	u32 SaltLen;
	u64 SaltAddr = 0U;

	/** Validate input parameters. */
	if (Prk == NULL) {
		Status = XASUFW_HKDF_INVALID_PARAM;
		goto END;
	}

	/** Get SHA hash length based on SHA Mode. */
	Status = XSha_GetHashLen(HkdfParams->KdfParams.ShaMode, &HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HKDF_GET_HASHLEN_FAILED);
		goto END;
	}

	/**
	 * Salt is optional in HKDF.
	 * If Salt is not provided by the user, zeros are given to HMAC as key.
	 */
	if ((HkdfParams->SaltAddr == 0U) && (HkdfParams->SaltLen == 0U)) {
		SaltAddr = (u64)(UINTPTR)&Salt;
		SaltLen = XASUFW_WORD_LEN_IN_BYTES;
	} else {
		SaltLen = HkdfParams->SaltLen;
		SaltAddr = (u64)(UINTPTR)HkdfParams->SaltAddr;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Initialize HMAC using salt as the input key. */
	Status = XHmac_Init(HmacPtr, DmaPtr, ShaInstancePtr, SaltAddr, SaltLen,
				HkdfParams->KdfParams.ShaMode, HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HKDF_HMAC_INIT_FAILED);
		goto END;
	}

	/**
	 * Update Input Keying Material (IKM) provided by user to the HMAC pseudo random function.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XHmac_Update(HmacPtr, DmaPtr, HkdfParams->KdfParams.KeyInAddr,
			      HkdfParams->KdfParams.KeyInLen, XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HKDF_HMAC_UPDATE_FAILED);
		goto END;
	}

	/** Get the Pseudo Random Key from HMAC. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XHmac_Final(HmacPtr, DmaPtr, (u32 *)Prk);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates the keying material object of specified number of bytes
 * using HMAC as pseudorandom function with the Prk(Pseudo Random Key) got from extract step,
 * previous block hash if present, info and block index.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaInstancePtr	Pointer to the XSha instance.
 * @param	KdfParams	Pointer to the HKDF structure containing user input parameters.
 * @param	Prk		Pointer to the pseudorandom key generated from extract.
 *
 * @return
 * 	- XASUFW_SUCCESS, if HKDF expand operation is successful.
 * 	- XASUFW_HKDF_INVALID_PARAM, if input parameters are invalid.
 * 	- XASUFW_HKDF_GET_HASHLEN_FAILED, if XSha_GetHashLen operation fails.
 * 	- XASUFW_MEM_COPY_FAIL, if Xil_SMemCpy operation fails.
 * 	- XASUFW_HKDF_HMAC_INIT_FAILED, if XHmac_Init operation fails.
 * 	- XASUFW_HKDF_HMAC_UPDATE_FAILED, if XHmac_Update operation fails.
 * 	- XASUFW_HKDF_HMAC_FINAL_FAILED, if XHmac_Final operation fails.
 *
 *************************************************************************************************/
static s32 XHkdf_Expand(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
			const XAsu_HkdfParams *HkdfParams, u8 *Prk)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XHmac *HmacPtr = XHmac_GetInstance();
	volatile u32 Iterations = 0U;
	volatile u8 KdfIndex = 0U;
	u8 KdfValue;
	u32 HashLen = 0U;
	u8 KOut[XASU_SHA_512_HASH_LEN];
	u64 KeyOutAddr = 0U;

	/** Validate input parameters. */
	if (Prk == NULL) {
		Status = XASUFW_HKDF_INVALID_PARAM;
		goto END;
	}

	/** Get SHA hash length based on SHA Mode. */
	Status = XSha_GetHashLen(HkdfParams->KdfParams.ShaMode, &HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HKDF_GET_HASHLEN_FAILED);
		goto END;
	}

	/** Validate Context length and KeyOut length are within range. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((HkdfParams->KdfParams.ContextAddr == 0U) ||
	    (HkdfParams->KdfParams.ContextLen == 0U)  ||
	    (HkdfParams->KdfParams.ContextLen > XHKDF_MAX_CONTEXT_LEN) ||
	    (HkdfParams->KdfParams.KeyOutAddr == 0U)  || (HkdfParams->KdfParams.KeyOutLen == 0U) ||
	    (HkdfParams->KdfParams.KeyOutLen > (HashLen * XHKDF_MAX_ITERATIONS))) {
		Status = XASUFW_HKDF_INVALID_PARAM;
		goto END;
	}

	/**
	 * Calculate number of iterations based on HKDF output key length and SHA algorithm hash
	 * length.
	 * According to the algorithm, maximum iterations should not exceed byte max length 0xFFU.
	 */
	Iterations = (u32)Xil_Ceil((float)HkdfParams->KdfParams.KeyOutLen / HashLen);
	if (Iterations > XHKDF_MAX_ITERATIONS) {
		Status = XASUFW_HKDF_INVALID_PARAM;
		goto END;
	}

	/**
	 * Use HMAC as a pseudorandom function, run the below steps in iterations until requested
	 * bytes of key output is generated and form the final output.
	 * The HMAC inputs are chained by prepending the previous hash block to the "info" field
	 * and appending with an incrementing 8-bit counter.
	 */
	KeyOutAddr = HkdfParams->KdfParams.KeyOutAddr;
	for (KdfIndex = 0U; KdfIndex < Iterations; ++KdfIndex) {
		/** - Initialize HMAC with PRK generated from HKDF extract step. */
		Status = XHmac_Init(HmacPtr, DmaPtr, ShaInstancePtr, (u64)(UINTPTR)Prk, HashLen,
				    HkdfParams->KdfParams.ShaMode, HashLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HKDF_HMAC_INIT_FAILED);
			goto END_CLR;
		}

		/**
		 * - Update Previous Hash block output to HMAC, if HKDF key output length more
		 * than hash length of SHA algorithm.
		 */
		if (KdfIndex > 0U) {
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XHmac_Update(HmacPtr, DmaPtr, (u64)(UINTPTR)KOut, HashLen,
						XASU_FALSE);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
								XASUFW_HKDF_HMAC_UPDATE_FAILED);
				goto END_CLR;
			}
		}

		/** - Update info provided by user to HMAC. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XHmac_Update(HmacPtr, DmaPtr, HkdfParams->KdfParams.ContextAddr,
				      HkdfParams->KdfParams.ContextLen,	XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HKDF_HMAC_UPDATE_FAILED);
			goto END_CLR;
		}

		/** - Update block index of 1 byte to HMAC. */
		KdfValue = KdfIndex + XHKDF_BLOCK_INDEX_LENGTH;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XHmac_Update(HmacPtr, DmaPtr, (u64)(UINTPTR)&KdfValue,
					XHKDF_BLOCK_INDEX_LENGTH, XASU_TRUE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HKDF_HMAC_UPDATE_FAILED);
			goto END_CLR;
		}

		/** - Get final HMAC for the running iteration. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XHmac_Final(HmacPtr, DmaPtr, (u32 *)KOut);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HKDF_HMAC_FINAL_FAILED);
			goto END_CLR;
		}

		/**
		 * - HMAC output size is the SHA algorithm hash length. HKDF key output can be any
		 * length.
		 * - In the last iteration, data to be copied to the destination can be less than or
		 * equal to SHA algorithm hash length which varies based on the requested key
		 * output length.
		 */
		if (KdfIndex == (Iterations - XHKDF_BLOCK_INDEX_LENGTH)) {
			HashLen = HkdfParams->KdfParams.KeyOutLen - (KdfIndex * HashLen);
		}

		/**
		 * - Copy the final HMAC of each iteration to the destination key output buffer.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy((u8 *)(UINTPTR)KeyOutAddr, HashLen, KOut, HashLen, HashLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MEM_COPY_FAIL);
			goto END_CLR;
		}
		KeyOutAddr = KeyOutAddr + HashLen;
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

	Status = XAsufw_UpdateBufStatus(Status, Xil_SecureZeroize(KOut, XASU_SHA_512_HASH_LEN));

	/** In case of failure, zeroize the intermediate KeyOutAddr memory. */
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateBufStatus(Status, Xil_SecureZeroize(
						(u8 *)(UINTPTR)HkdfParams->KdfParams.KeyOutAddr,
						HkdfParams->KdfParams.KeyOutLen));
	}

END:
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HKDF_GENERATE_FAILED);
	}

	return Status;
}
#endif /* XASU_ECIES_ENABLE */
/** @} */
