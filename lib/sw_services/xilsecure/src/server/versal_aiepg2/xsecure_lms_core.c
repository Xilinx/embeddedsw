/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/******************************************************************************/
/**
*
* @file xsecure_lms_core.c
*
* This file consists definitions of LMS authentication routines
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.4   kal  07/24/24  Initial release
*       kal  01/30/25  Update LMS/HSS APIs to accept the original data instead
*                      of pre calculated hash.
*
* </pre>
* @note
*
*******************************************************************************/

/***************************** Include Files **********************************/
#include "xsecure_sha.h"
#include "xsecure_lms_core.h"
#include "xsecure_defs.h"

/************************** Constant Definitions *****************************/
static volatile XSecure_LmsPublicKey AuthenticatedKey;
static XSecure_LmsDataDigestFixedFields DigestPrefixFields;
static volatile u32 SignatureLengthConsumed = 0U;
static XSecure_LmsDataDigest DigestChecksum;
static u32 CurrentLmsQ = 0U;

/***************** Macros (Inline Functions) Definitions *********************/
static int XSecure_LmsOtsSignatureCompute(XSecure_Sha *ShaInstPtr, XPmcDma *DmaPtr,
	u8* Data, u32 DataLen,
	u32 PreHashedMsg, u8* LmsOtsSignatureBuff,
	u32 LmsOtsSignatureLen, XSecure_LmsOtsType LmsOtsExpectedType,
	u8* Kc);
static u32 XSecure_SwapBytes(const u8 *const source, size_t bytes);

/**************************** Type Definitions *******************************/

#define XSECURE_PUB_ALGO_LMS_HSS	(4U)
#define XSECURE_PUB_ALGO_LMS		(5U)
#define XSECURE_LMS_PUB_KEY_TMP_BUF_ADJ_NODE_VAL_INDEX		(32U)

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/******************************************************************************/
/**
* @brief	This function takes in LMS OTS Signature and produces LMS OTS
* 		Public key, which in turn is used to calculate the root public
* 		key of Merkle tree
*
* @param	Data			- Data to be authenticated
* @param	DataLen			- Length of data to be authenticated
* @param	PreHashedMsg		-  If TRUE, then Data contains digest,
* 					if FALSE contains raw msg to be authenticated
* @param	LmsOtsSignatureBuff	- Pointer to OTS signature buffer
* @param	LmsOtsSignatureLen 	- Length of OTS signature buffer
* @param	LmsOtsExpectedType 	- Expected OTS signature type, value read
* 					from public key of this level XSecure_LmsOtsType
* @param	Kc			- Buffer where the computed value of root
* 					needs to be copied, assumed that has enough place
*
* @return
*	-	@ref XST_SUCCESS - If operation success, otherwise following errors
*******************************************************************************/
static int XSecure_LmsOtsSignatureCompute(XSecure_Sha *ShaInstPtr,
        XPmcDma *DmaPtr,
	u8* Data, u32 DataLen,
	u32 PreHashedMsg, u8* LmsOtsSignatureBuff,
	u32 LmsOtsSignatureLen, XSecure_LmsOtsType LmsOtsExpectedType,
	u8* Kc)
{
	int Status = XST_FAILURE;
	volatile XSecure_LmsOtsType LmsOtsType = XSECURE_LMS_OTS_NOT_SUPPORTED;
	const XSecure_LmsOtsParam* LmsOtsSignParam = NULL;
	u32 Checksum = 0U;
	u32 DigitVal = 0U;
	u32 DigitIndex = 0U;
	u32 ChainIter = 0U;
	u32 SignToOtsBuffIndex = 0U;
	u32 IntToOutLoopBuffIndex = 0U;
	static XSecure_LmsOtsSignToPubKeyHash LmsOtsSignVerifBuff __attribute__((aligned(16)));
	static XSecure_LmsOtsHashPerDigit TmpHashPerDigitBuff;
	static const u8 XSECURE_D_PBLC[2U] = { 0x80U, 0x80U };

	/* Computed root to be copied here,
	 * assumed to have enough room, created and checked by caller,
	 * also should be cleared by caller
	 */
	if (NULL == Kc) {
		Status = XSECURE_LMS_INVALID_PARAM;
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS OTS - Output buffer at invalid address\n\r");
		goto END;
	}

	if (XSECURE_LMS_OTS_TYPE_SIZE > LmsOtsSignatureLen) {
		Status = XSECURE_LMS_INVALID_PARAM;
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS OTS - Invalid signature length 0x%x\n\r", LmsOtsSignatureLen);
		goto END;
	}

	if (NULL == LmsOtsSignatureBuff) {
		Status = XSECURE_LMS_INVALID_PARAM;
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS OTS - Invalid signature addr\n\r");
		goto END;
	}

	LmsOtsType = (XSecure_LmsOtsType)XSecure_SwapBytes(
		(const void*)&LmsOtsSignatureBuff[XSECURE_LMS_OTS_SIGN_TYPE_FIELD_OFFSET],
		XSECURE_LMS_OTS_TYPE_SIZE);

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS OTS - Signature Type 0x%x\n\r", LmsOtsType);

	if (LmsOtsType != LmsOtsExpectedType) {
		Status = XSECURE_LMS_OTS_PUB_KEY_SIGN_TYPE_MISMATCH_ERROR;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS OTS - Signature Type mismatch 0x%x, expected 0x%x\n\r",
			LmsOtsType, LmsOtsExpectedType);
		goto END;
	}

	Status = XSecure_LmsOtsLookupParamSet(LmsOtsType, (XSecure_LmsOtsParam**)&LmsOtsSignParam);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_LMS_OTS_SIGN_UNSUPPORTED_TYPE_ERROR;
		goto END;
	}

	if (LmsOtsSignParam->SignLen != LmsOtsSignatureLen) {
		Status = XSECURE_LMS_INVALID_PARAM;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS OTS - Signature Total length error 0x%x, expected 0x%x\n\r",
			LmsOtsSignatureLen, LmsOtsSignParam->SignLen);
		goto END;
	}

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS OTS - Signature length 0x%x\n\r", LmsOtsSignParam->SignLen);

	/* ****************************************************************************************** */
	/* Message processing - sequence-2 */
	/* ****************************************************************************************** */
	/**
	 * Now the message's digest needs to be calculated and checksum needs to be appended
	 * Q = H(I || u32str(q) || u16str(D_MESG) || C || message)
	 * a = (Q || checksum(Q))
	 */

	 /* If message is pre hashed skip digest calculation, the same buffer is used to pass digest,
		else calculate digest */
	if ((u32)FALSE == PreHashedMsg) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS OTS - Data to be hashed\n\r");

		/** Extracting C, Big Endian to Big Endian */
		Status = Xil_SMemCpy((void*)DigestPrefixFields.Fields.C, XSECURE_LMS_C_FIELD_SIZE,
			(void*)&LmsOtsSignatureBuff[XSECURE_LMS_OTS_SIGN_C_FIELD_OFFSET],
			XSECURE_LMS_C_FIELD_SIZE, XSECURE_LMS_C_FIELD_SIZE);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/* initialize HASH to calculate digest */
		Status = XSecure_ShaInitialize(ShaInstPtr, DmaPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = XSecure_LmsHashMessage(ShaInstPtr, Data, DataLen, LmsOtsSignParam->H);
		if (Status != XST_SUCCESS) {
			Status = XSECURE_LMS_OTS_DIGEST_CHECKSUM_OP_FAILED_ERROR;
			goto END;
		}
	}

	/* Append checksum at end of buffer */
	Status = XSecure_LmsOtsComputeChecksum(DigestChecksum.Fields.Digest, XSECURE_LMS_DIGEST_SIZE,
			(u32)LmsOtsSignParam->w, (u32)LmsOtsSignParam->ls, (u32* const)&Checksum);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_AUTH_LMS_DIGEST_CHECKSUM_FAILED_ERROR;
		goto END;
	}

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS OTS - Checksum computed 0x%x\n\r", Checksum);

	/* Little Endian to Big Endian */
	DigestChecksum.Fields.Checksum[1u] = (u8)(Checksum & 0x00FF);
	DigestChecksum.Fields.Checksum[0U] = (u8)((Checksum >> 8U) & 0x00FF);

	/* ****************************************************************************************** */
	/* Signature processing - sequence-2 */
	/* ****************************************************************************************** */

	/**
	 * All params are extracted from public key, now proceed to parse signature
	 * 4b.3
	 * Q = H(I || u32str(q) || u16str(D_MESG) || C || message)
	 * a = (Q || checksum(Q))
	 *  for ( i = 0; i < p; i = i + 1 ) {
	 *    a = coef(Q || Cksm(Q), i, w)
	 *    tmp = y[i]
	 *    for ( j = a; j < 2^w - 1; j = j + 1 ) {
	 *      tmp = H(I || u32str(q) || u16str(i) || u8str(j) || tmp)
	 *    }
	 *    z[i] = tmp
	 *  }
	 *  Kc = H(I || u32str(q) || u16str(D_PBLC) ||
	 *                                z[0] || z[1] || ... || z[p-1])
	 */

	 /** Fill @ref XSecure_LmsOtsHashPerDigit fields, we copy early so that during internal loop we
		 reduce operations during loop */

	/* Copy 'I' from public key, Big Endian to Big Endian copy */
	Status = Xil_SMemCpy((void*)&TmpHashPerDigitBuff.Fields.I[0U], XSECURE_LMS_I_FIELD_SIZE,
		(void*)DigestPrefixFields.Fields.I, XSECURE_LMS_I_FIELD_SIZE,
		XSECURE_LMS_I_FIELD_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Copy 'q' from public key, Little Endian to Big Endian copy */
	TmpHashPerDigitBuff.Fields.q = Xil_Htonl(CurrentLmsQ);

	/** Fill @ref XSecure_LmsOtsSignToPubKeyHash fields, we copy early so that during internal loop
		we reduce operations during loop
		as PMCRAM is cleared, we do clear first OTS op is complete, again used for lower level tree
		OTS op, then cleared again. */

	/* Copy 'I' from public key, Big Endian to Big Endian copy */
	Status = Xil_SMemCpy((void*)&LmsOtsSignVerifBuff.Buff[XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_I_OFFSET],
		XSECURE_LMS_I_FIELD_SIZE,
		(void*)DigestPrefixFields.Fields.I,
		XSECURE_LMS_I_FIELD_SIZE, XSECURE_LMS_I_FIELD_SIZE);
	if (Status != XST_SUCCESS) {
                goto END;
        }


	/* Copy 'q' from public key, Little Endian to Big Endian copy */
	LmsOtsSignVerifBuff.Fields.q = Xil_Htonl(CurrentLmsQ);

	/* Big Endian to Big Endian */
	LmsOtsSignVerifBuff.Fields.D_PBLC[0U] = XSECURE_D_PBLC[0U];
	LmsOtsSignVerifBuff.Fields.D_PBLC[1U] = XSECURE_D_PBLC[1U];

	SignToOtsBuffIndex = XSECURE_LMS_OTS_SIGN_Y_FIELD_OFFSET;

	/*
	 * Loop through each digit in (Digest || checksum)
	 * Here 'p' can be from signature or public key
	 * covers 4b.2e & 4b.3 step
	 */
	for (DigitIndex = 0u, IntToOutLoopBuffIndex = 0U;
		 DigitIndex < (u16)LmsOtsSignParam->p;
		 DigitIndex++, IntToOutLoopBuffIndex += XSECURE_LMS_N_FIELD_SIZE,
		SignToOtsBuffIndex += XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS OTS - outer loop 0x%x\n\r", DigitIndex);

		/* Extract digit value from (Digest || checksum) */
		DigitVal = XSecure_LmsOtsCoeff(DigestChecksum.Buff, DigitIndex, LmsOtsSignParam->w);

		/* Copy i, Little Endian to Big Endian */
		TmpHashPerDigitBuff.Fields.i = (u16)(DigitIndex);
		TmpHashPerDigitBuff.Fields.i = Xil_Htons(TmpHashPerDigitBuff.Fields.i);

		/* 'j' will be copied in inner loop, copy y[i], Big Endian to Big Endian */
		Status = Xil_SMemCpy((void*)&TmpHashPerDigitBuff.Fields.y[0U],
			XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE,
			(void*)&LmsOtsSignatureBuff[SignToOtsBuffIndex],
			XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE,
			XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/* Forward the hash chain to get to public key */
		for (ChainIter = DigitVal; ChainIter < (LmsOtsSignParam->NoOfInvSign); ChainIter++) {
			XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS OTS - Inner loop 0x%x\n\r", ChainIter);

			/* Copy 'j', just a byte, no endianness */
			TmpHashPerDigitBuff.Fields.j = (u8)(ChainIter);

			/* Compute digest for this iteration */
			Status = XSecure_ShaDigest(ShaInstPtr, LmsOtsSignParam->H,
					(u64)(UINTPTR)&TmpHashPerDigitBuff.Buff[XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_I_OFFSET],
					XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_TOTAL_SIZE,
					(u64)(UINTPTR)&TmpHashPerDigitBuff.Fields.y[0U],
					XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE);
			if (Status != XST_SUCCESS) {
				Status = XSECURE_LMS_OTS_SIGN_SHA_DIGEST_FAILED_ERROR;
				goto END;
			}
		}

		/* Copy to overall buffer, later all values concatenated used to calculate LMS OTS public key */
		Status = Xil_SMemCpy((void*)&LmsOtsSignVerifBuff.Fields.z[IntToOutLoopBuffIndex],
			XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE,
			(void*)&TmpHashPerDigitBuff.Buff[XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_Y_OFFSET],
			XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE,
			XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS OTS - IntToOutLoopBuffIndex 0x%x\n\r", IntToOutLoopBuffIndex);
	}

	/* At this point all the hash chains for n byte arrays in signature are completed, we have to
	   complete the final step of signature verification
	   Kc = H(I || u32str(q) || u16str(D_PBLC) || z[0] || z[1] || ... || z[p-1]) */

	/* concatenated buffer's hash */
	Status = XSecure_ShaDigest(ShaInstPtr, LmsOtsSignParam->H,
			(u64)(UINTPTR)LmsOtsSignVerifBuff.Buff,
			XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_CURRENT_SIZE(LmsOtsSignParam->p),
			(u64)(UINTPTR)Kc,
			XSECURE_LMS_N_FIELD_SIZE);

END:
	return Status;
}

/******************************************************************************/
/**
* @brief	This function performs LMS level signature verification
*
* @param	Data - Data to be authenticated, will be passed to
* 				LMS OTS signature verification
* @param	DataLen	- Length of data to be authenticated, will be
* 				passed to LMS OTS signature verification
* @param	PreHashedMsg	- If TRUE, then Data contains digest, if FALSE
*				contains raw msg to be authenticated
* @param	LmsSign		- Pointer to LMS signature buffer for a tree
* @param	LmsSignLen	- Length of LMS signature
* @param	ExpectedPubKey	- Pointer to expected  LMS public key
* @param	PubKeyLen	- Length of LMS public key
*
* @return
*		XST_SUCCESS - if operation success, otherwise following errors
*
*******************************************************************************/
int XSecure_LmsSignatureVerification(XSecure_Sha *ShaInstPtr, XPmcDma *DmaPtr,
	XSecure_LmsSignVerifyParams *LmsSignVerifyParams)
{
	volatile int Status = XST_FAILURE;
	u32 LoopError = XST_FAILURE;
	volatile XSecure_LmsType PubKeyLmsType = XSECURE_LMS_NOT_SUPPORTED;
	volatile XSecure_LmsOtsType PubKeyLmsOtsType = XSECURE_LMS_OTS_NOT_SUPPORTED;
	const XSecure_LmsParam* LmsPubKeyParam = NULL;
	const XSecure_LmsOtsParam* LmsOtsSignParam = NULL;
	volatile XSecure_LmsType SignLmsType = XSECURE_LMS_NOT_SUPPORTED;
	volatile XSecure_LmsOtsType SignLmsOtsType = XSECURE_LMS_OTS_NOT_SUPPORTED;
	const u8* Path = NULL;
	const XSecure_LmsParam* LmsSignParam = NULL;
	u32 CurrentNodeNum = 0U;
	/** Used to find 32 byte value from LMS signature array */
	volatile u32 PathIndex = 0U;
	volatile u32 Index = 0U;
	static u8 TmpBuff[XSECURE_LMS_M_BYTE_FIELD_SIZE];
	static XSecure_LmsPubKeyTmp LmsPubKeyTmpBuff;
	static u8 ExpectedPublicKey[XSECURE_LMS_OTS_PUB_KEY_K_FIELD_SIZE];
	static const u8 XSECURE_D_LEAF[2U] = {0x82U, 0x82U};
	static const u8 XSECURE_D_INTR[2U] = {0x83U, 0x83U};

	/* ****************************************************************************************** */
	/* Expected public key processing - sequence - 1*/
	/* ****************************************************************************************** */

	/**
	 *  6. Public key checks
	 *  1, 2a, 2b - Length should be atleast 8 bytes to be able to fetch Type of algo used for
	 * 		LMS & LMS OTS
	 *  2c  - Once Type is extracted, set m
	 *  2d - Public key should be == 24 + m otherwise stop process and raise error
	 */
	if (NULL == LmsSignVerifyParams->ExpectedPubKey) {
		Status = XSECURE_LMS_INVALID_PARAM;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS signature verification - expected pub key at invalid addr\n\r");
		goto END;
	}

	/** 6.1: Standard recommends less than 8 is not valid, this check covers 4a.1 */
	if ((XSECURE_LMS_TYPE_SIZE + XSECURE_LMS_OTS_TYPE_SIZE) > LmsSignVerifyParams->PubKeyLen) {
		Status = XSECURE_LMS_INVALID_PARAM;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS signature verification - invalid pub key Length 0x%x\n\r",
			LmsSignVerifyParams->PubKeyLen);
		goto END;
	}

	/** 6.2a Extracting Public key LMS Type, Big Endian to Little Endian */
	PubKeyLmsType = (XSecure_LmsType)XSecure_SwapBytes((const void*)&LmsSignVerifyParams->ExpectedPubKey[XSECURE_LMS_PUB_KEY_TYPE_OFFSET],
		XSECURE_LMS_TYPE_SIZE);

	/** 6.2b Extracting Public key LMS OTS Type, Big Endian to Little Endian */
	PubKeyLmsOtsType = (XSecure_LmsOtsType)XSecure_SwapBytes(
			(const void*)&LmsSignVerifyParams->ExpectedPubKey[XSECURE_LMS_PUB_KEY_OTS_TYPE_OFFSET],
			XSECURE_LMS_OTS_TYPE_SIZE);

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS signature verification - expected public key Type 0x%x\n\r", PubKeyLmsType);

	/* 6.2c, Fetch the Type of public key, if not a valid/supported Type return error, also covers 4a.2 */
	Status = XSecure_LmsLookupParamSet(PubKeyLmsType,
			(XSecure_LmsParam**)&LmsPubKeyParam);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_LMS_PUB_KEY_UNSUPPORTED_TYPE_1_ERROR;
		goto END;
	}

	/** 6.2d: Length of key check, should be equal to 24 + m, covers 4a.2b */
	if ((XSECURE_LMS_PUB_KEY_FIXED_FIELD_SIZE + LmsPubKeyParam->m) != LmsSignVerifyParams->PubKeyLen) {
		Status = XSECURE_LMS_SIGN_EXPECTED_PUB_KEY_LEN_2_ERROR;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS signature verification - invalid pub key Length 0x%x, expected 0x%x\n\r",
			LmsSignVerifyParams->PubKeyLen, (XSECURE_LMS_PUB_KEY_FIXED_FIELD_SIZE + LmsPubKeyParam->m));
		goto END;
	}

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS signature verification - public key length 0x%x\n\r", LmsSignVerifyParams->PubKeyLen);

	/* ****************************************************************************************** */
	/* Signature processing - sequence - 1*/
	/* ****************************************************************************************** */

	/**
	 * 6a. Signature checks
	 * 1 	- Length should be atleast 8 bytes long
	 * 2a 	- Parse 'q', OTS 'Type'
	 * 2b,2c - OTS 'Type' should match with pub key OTS 'Type'
	 * 2d 	- Set 'n', 'p' from signature OTS 'Type',
	 * 2d 	- Length should be AT LEAST (12 + n * (p + 1)), this is to check if signature has enough
	 * 			  for OTS signature component
	 * 2e,2f - OTS signature [4 to (7 + n (p + 1))]
	 * 2g 	- Set LMS signature Type [(8 + n (p + 1)) to (11 + n (p + 1)) bytes], LMS signature
	 * 			  Type should match with LMS public 'Type'
	 * 2h 	- Set 'm', 'h' according to LMS signature Type
	 * 2i 	- 'q' >= 2^h or LMS signature Length != 12 + n * (p + 1) + m * h) then error
	 */

	if (NULL == LmsSignVerifyParams->LmsSign) {
		Status = XSECURE_LMS_INVALID_PARAM;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS signature verification - invalid addr for signature buff\n\r");
		goto END;
	}

	/** 6a.1 Length should be at least 8 bytes */
	if ((XSECURE_LMS_Q_FIELD_SIZE + XSECURE_LMS_OTS_TYPE_SIZE) > LmsSignVerifyParams->LmsSignLen) {
		Status = XSECURE_LMS_SIGN_LEN_1_ERROR;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS signature verification - invalid signature Length 0x%x, expected 0x%x\n\r",
			LmsSignVerifyParams->LmsSignLen, (XSECURE_LMS_Q_FIELD_SIZE + XSECURE_LMS_OTS_TYPE_SIZE));
		goto END;
	}

	/* If a message is not pre-hashed, we need to compute the digest for authentication, prefetch q,
	   I before*/
	if ((u32)FALSE == LmsSignVerifyParams->PreHashedMsg) {
		/** 6a.2a parse q from LMS signature, Big Endian to Little Endian */
		CurrentLmsQ = XSecure_SwapBytes(&LmsSignVerifyParams->LmsSign[XSECURE_LMS_SIGNATURE_Q_FIELD_OFFSET],
			XSECURE_LMS_Q_FIELD_SIZE);
		DigestPrefixFields.Fields.q = Xil_Htonl(CurrentLmsQ);
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS signature verification - 'q' extracted !!!!! 0x%x\n\r",
			CurrentLmsQ);

		/** 6.2e parse I from LMS Public key */
		Status = Xil_SMemCpy((void*)DigestPrefixFields.Fields.I,
				XSECURE_LMS_I_FIELD_SIZE,
				(void*)&LmsSignVerifyParams->ExpectedPubKey[XSECURE_LMS_PUB_KEY_I_OFFSET],
				XSECURE_LMS_I_FIELD_SIZE,
				XSECURE_LMS_I_FIELD_SIZE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/* 6a.2b, Read LMS OTS type, Big Endian to Little Endian */
	SignLmsOtsType = (XSecure_LmsOtsType)XSecure_SwapBytes(
			&LmsSignVerifyParams->LmsSign[XSECURE_LMS_SIGNATURE_OTS_FIELD_OFFSET + XSECURE_LMS_OTS_SIGN_TYPE_FIELD_OFFSET],
			XSECURE_LMS_OTS_TYPE_SIZE);

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS signature verification - signature addr 0x%x\n\r", LmsSignVerifyParams->LmsSign);

	/* Extract LMS OTS signature from buffer */
	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS signature verification - OTS Signature Type 0x%x\n\r", SignLmsOtsType);

	/* Covers 6a.2c in standard, If LMS OTS signature Type doesn't match with LMS pub key Type,
	   both are in Little Endian */
	if (PubKeyLmsOtsType != SignLmsOtsType) {
		Status = XSECURE_LMS_OTS_PUB_KEY_LMS_OTS_SIGN_TYPE_MISMATCH_ERROR;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS signature verification - OTS Type mismatch 0x%x, expected 0x%x\n\r",
			SignLmsOtsType, PubKeyLmsOtsType);
		goto END;
	}

	Status = XSecure_LmsOtsLookupParamSet(SignLmsOtsType,
			(XSecure_LmsOtsParam**)&LmsOtsSignParam);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_LMS_SIGN_UNSUPPORTED_OTS_TYPE_1_ERROR;
		goto END;
	}

	/**
	 * 6a.2d: Total signature length check, should have at least LMS OTS signature required Length
	 * to proceed
	 */
	if ((LmsOtsSignParam->SignLen + XSECURE_LMS_Q_FIELD_SIZE) > LmsSignVerifyParams->LmsSignLen) {
		Status = XSECURE_LMS_SIGN_LEN_2_ERROR;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS signature verification - signature Length error 0x%x, expected 0x%x\n\r",
			LmsSignVerifyParams->LmsSignLen, (LmsOtsSignParam->SignLen + XSECURE_LMS_Q_FIELD_SIZE));
		goto END;
	}
	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS signature verification - signature Length 0x%x\n\r", LmsSignVerifyParams->LmsSignLen);

	/* 6a.2f Extract LMS signature Type from buffer, Big Endian to Little Endian */
	SignLmsType = (XSecure_LmsType)XSecure_SwapBytes(
			(const void*)&LmsSignVerifyParams->LmsSign[(LmsOtsSignParam->SignLen + XSECURE_LMS_Q_FIELD_SIZE)],
			XSECURE_LMS_TYPE_SIZE);

	/* 6a.2g, Comparing Public key's LMS type & Signature's LMS type, both are in Little Endian */
	if (PubKeyLmsType != SignLmsType) {
		Status = XSECURE_LMS_PUB_KEY_LMS_SIGN_TYPE_MISMATCH_ERROR;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS signature verification - LMS Type mismatch 0x%x, expected 0x%x\n\r",
			SignLmsType, PubKeyLmsType);
		goto END;
	}
	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS signature verification - LMS Type matched 0x%x\n\r", SignLmsType);

	/**
	 * 6a.2h: Fetch the params for LMS signature from Type of signature, if not a valid/supported
	 * Type return error
	 */
	Status = XSecure_LmsLookupParamSet(SignLmsType,
			(XSecure_LmsParam**)&LmsSignParam);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_LMS_SIGN_UNSUPPORTED_TYPE_1_ERROR;
		goto END;
	}

	/**
	 * 6a.2i: Leaf nodes for a tree range from [0 to (2^h - 1)], to be a valid node number
	 * LMS signature length should be 12 + (n * (p + 1)) + (m * h) bytes
	 */
	if ((u32)(1U << LmsSignParam->h) <= CurrentLmsQ) {
		Status = XSECURE_LMS_SIGN_INVALID_NODE_NUMBER_ERROR;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS signature verification - node out of range 0x%x, allowed 0x%x\n\r",
			CurrentLmsQ, (1U << LmsSignParam->h));
		goto END;
	}

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS signature verification - node used 0x%x\n\r", CurrentLmsQ);

	/* 6a.2i, Size of 'q' + size of LMS OTS signature + LMS signature Type size + (m * h) */
	if (LmsSignVerifyParams->LmsSignLen != (LmsOtsSignParam->SignLen +
		XSECURE_LMS_Q_FIELD_SIZE +
		XSECURE_LMS_TYPE_SIZE +
		LmsSignParam->mh)) {
		Status = XSECURE_LMS_SIGN_LEN_3_ERROR;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS signature verification - signature Length error 0x%x, expected 0x%x\n\r",
			LmsSignVerifyParams->LmsSignLen, (LmsOtsSignParam->SignLen + XSECURE_LMS_Q_FIELD_SIZE + XSECURE_LMS_TYPE_SIZE + LmsSignParam->mh));
		goto END;
	}

	XSecure_Printf(XSECURE_DEBUG_GENERAL,
		"LMS signature verification - length of total siglist[0] matched 0x%x\n\r",
		LmsSignVerifyParams->LmsSignLen);

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS signature verification - Initiating OTS verification\n\r");

	/** 6a.3: LMS OTS candidate public key 'Kc' */
	Status = XSecure_LmsOtsSignatureCompute(ShaInstPtr, DmaPtr, LmsSignVerifyParams->Data,
			LmsSignVerifyParams->DataLen, LmsSignVerifyParams->PreHashedMsg,
			&LmsSignVerifyParams->LmsSign[XSECURE_LMS_Q_FIELD_SIZE], LmsOtsSignParam->SignLen,
			PubKeyLmsOtsType, (u8* const)ExpectedPublicKey);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_LMS_SIGN_OTS_OP_ERROR;
		goto END;
	}

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS signature verification - OTS operation Complete\n\r");

	/* 6a.2j, Extract LMS signature from buffer */
	Path = (const u8*)& LmsSignVerifyParams->LmsSign[(LmsOtsSignParam->SignLen + XSECURE_LMS_Q_FIELD_SIZE + XSECURE_LMS_TYPE_SIZE)];
	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS signature verification - path addr 0x%x\n\r", Path);

	/* copy I, Big Endian to Big Endian */
	Status = Xil_SMemCpy((void*)LmsPubKeyTmpBuff.Fields.I,
		XSECURE_LMS_I_FIELD_SIZE,
		(void*)&LmsSignVerifyParams->ExpectedPubKey[XSECURE_LMS_PUB_KEY_I_OFFSET],
		XSECURE_LMS_I_FIELD_SIZE,
		XSECURE_LMS_I_FIELD_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Node number = (2 ^ h) + q */
	CurrentNodeNum = (1U << LmsSignParam->h) + CurrentLmsQ;

	/* Copy node number - Little Endian to Big Endian */
	LmsPubKeyTmpBuff.Fields.half_node_number = Xil_Htonl(CurrentNodeNum);

	/* Copy constant - Big Endian to Big Endian */
	LmsPubKeyTmpBuff.Fields.D[0U] = XSECURE_D_LEAF[0U];
	LmsPubKeyTmpBuff.Fields.D[1U] = XSECURE_D_LEAF[1U];

	/* Copy LMS OTS output to buffer - Big Endian to Big Endian */
	Status = Xil_SMemCpy((void*)&LmsPubKeyTmpBuff.Fields.Tmp[0U],
		XSECURE_LMS_OTS_PUB_KEY_K_FIELD_SIZE,
		(void*)&ExpectedPublicKey[0U],
		XSECURE_LMS_OTS_PUB_KEY_K_FIELD_SIZE,
		XSECURE_LMS_OTS_PUB_KEY_K_FIELD_SIZE);	/* 32 bytes */
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Now a total of 16 + 4 + 2 + 56 bytes are in buffer, which needs to be processed,
	   During LMS OTS HASH hw has been used, need to start again for public key calculation */

	/* Start till finish HASH to calculate digest */
	Status = XSecure_ShaDigest(ShaInstPtr, LmsOtsSignParam->H,
			(u64)(UINTPTR)&LmsPubKeyTmpBuff.Buff[0U],
			XSECURE_LMS_PUB_KEY_TMP_BUFFER_LEAF_TOTAL_SIZE,
			(u64)(UINTPTR)TmpBuff,
			XSECURE_LMS_M_BYTE_FIELD_SIZE);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_LMS_SIGN_VERIF_SHA_DIGEST_LEAF_FAILED_ERROR;
		goto END;
	}

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS signature verification - completed node's operations\n\r");

	/* For preparing for loop, we need I (which is already copied), node numbers needs to be copied
	   inside loop, D_INTR (to be copied before loop), then hashes according to odd or even nodes */

	/* Copy constant - Big Endian to Big Endian */
	LmsPubKeyTmpBuff.Fields.D[0U] = XSECURE_D_INTR[0U];
	LmsPubKeyTmpBuff.Fields.D[1U] = XSECURE_D_INTR[1U];


	while (CurrentNodeNum > 1U) {
		/* Copy node number - Little Endian to Big Endian */
		LmsPubKeyTmpBuff.Fields.half_node_number = Xil_Htonl((CurrentNodeNum) / 2U);

		if (XSECURE_IS_ODD(CurrentNodeNum)) {
			/* Odd: 	H(I||u32str(node_num/2)||u16str(D_INTR)||path[i]||tmp) */
			XSecure_Printf(XSECURE_DEBUG_GENERAL,
				"LMS signature verification - odd 0x%x\n\r",
				CurrentNodeNum);

			/* Copy path[i], then output of previous HASH - Big Endian to Big Endian */
			Status = Xil_SMemCpy((void*)LmsPubKeyTmpBuff.Fields.Tmp,
				XSECURE_LMS_M_BYTE_FIELD_SIZE,
				(void*)&Path[PathIndex],
				XSECURE_LMS_M_BYTE_FIELD_SIZE,
				XSECURE_LMS_M_BYTE_FIELD_SIZE);
			if (Status != XST_SUCCESS) {
				goto END;
			}

			Status = Xil_SMemCpy(
				(void*)&LmsPubKeyTmpBuff.Fields.Tmp[XSECURE_LMS_PUB_KEY_TMP_BUF_ADJ_NODE_VAL_INDEX],
				XSECURE_LMS_M_BYTE_FIELD_SIZE,
				(void*)TmpBuff,
				XSECURE_LMS_M_BYTE_FIELD_SIZE,
				XSECURE_LMS_M_BYTE_FIELD_SIZE);
			if (Status != XST_SUCCESS) {
				goto END;
			}

			LoopError = (u32)XSECURE_LMS_SIGN_VERIF_SHA_DIGEST_INTR_ODD_FAILED_ERROR;

		}
		else {
			/* Even: 	H(I||u32str(node_num/2)||u16str(D_INTR)||tmp||path[i]) */
			XSecure_Printf(XSECURE_DEBUG_GENERAL,
				"LMS signature verification - even 0x%x\n\r",
				CurrentNodeNum);

			/* Copy output of previous HASH then path[i] - Big Endian to Big Endian */
			Status = Xil_SMemCpy((void*)LmsPubKeyTmpBuff.Fields.Tmp,
				XSECURE_LMS_M_BYTE_FIELD_SIZE,
				(void*)TmpBuff,
				XSECURE_LMS_M_BYTE_FIELD_SIZE,
				XSECURE_LMS_M_BYTE_FIELD_SIZE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Status = Xil_SMemCpy(
				(void*)&LmsPubKeyTmpBuff.Fields.Tmp[XSECURE_LMS_PUB_KEY_TMP_BUF_ADJ_NODE_VAL_INDEX],
				XSECURE_LMS_M_BYTE_FIELD_SIZE,
				(void*)&Path[PathIndex],
				XSECURE_LMS_M_BYTE_FIELD_SIZE,
				XSECURE_LMS_M_BYTE_FIELD_SIZE);
			if (Status != XST_SUCCESS) {
				goto END;
			}

			LoopError = (u32)XSECURE_LMS_SIGN_VERIF_SHA_DIGEST_INTR_EVEN_FAILED_ERROR;
		}

		/* Start till finish HASH to calculate digest */
		Status = XSecure_ShaDigest(ShaInstPtr, LmsOtsSignParam->H,
				(u64)(UINTPTR)LmsPubKeyTmpBuff.Buff,
				XSECURE_LMS_PUB_KEY_TMP_BUFFER_INTR_TOTAL_SIZE,
				(u64)(UINTPTR)TmpBuff,
				XSECURE_LMS_M_BYTE_FIELD_SIZE);
		if (Status != XST_SUCCESS) {
			Status = (int)LoopError;
			goto END;
		}

		CurrentNodeNum = CurrentNodeNum / 2U;

		/* If following operation is glitched, we will end up with wrong root value which can be detected */
		PathIndex += XSECURE_LMS_M_BYTE_FIELD_SIZE;
	}

	/**
	 * 6.4, Now that we have arrived at root value, compare with expected to see if it matches,
	 * comparision should be single glitch resistant
	 */
	for (Index = 0U; Index < XSECURE_LMS_PUB_KEY_T_FIELD_SIZE; Index++) {
		if (LmsSignVerifyParams->ExpectedPubKey[XSECURE_LMS_PUB_KEY_T_OFFSET + Index] != TmpBuff[Index]) {
			Status = XSECURE_LMS_PUB_KEY_AUTHENTICATION_FAILED_ERROR;
			XSecure_Printf(XSECURE_DEBUG_GENERAL,
				"LMS signature verification - public key 0x%x, expected 0x%x\n\r",
				TmpBuff[Index], LmsSignVerifyParams->ExpectedPubKey[XSECURE_LMS_PUB_KEY_T_OFFSET + Index]);
			goto END;
		}
	}

	if (XSECURE_LMS_PUB_KEY_T_FIELD_SIZE != Index) {
		Status = XSECURE_LMS_PUB_KEY_AUTHENTICATION_GLITCH_ERROR;
		goto END;
	}

END:
	return Status;
}

/******************************************************************************/
/**
* @brief	This function Initiates LMS Signature verification process.
* 		Completes all upper level Merkle trees verification and prepares
* 		for data's Digest calculation
*
* @param[in]	Signature	- Pointer to Signature buffer
* @param[in]	SignatureLen	- Length of signature buffer
* @param[in]	PublicKey	- Pointer to public key buffer
* @param[in]	PublicKeyLen	- Length of public key buffer
*
* @return
*	-	@ref XST_SUCCESS - If operation success, otherwise following errors
*******************************************************************************/
int XSecure_HssInit(XSecure_Sha *ShaInstPtr, XPmcDma *DmaPtr, XSecure_HssInitParams *HssInitParams)
{
	volatile int Status = XST_FAILURE;
	XSecure_LmsType PublicKeyLmsType = XSECURE_LMS_NOT_SUPPORTED;
	XSecure_LmsOtsType PublicKeyLmsOtsType = XSECURE_LMS_OTS_NOT_SUPPORTED;
	const u8* TmpPublicKeyPtr = NULL;
	const XSecure_LmsParam* PubKeyLmsParam = NULL;
	const XSecure_LmsOtsParam* PubKeyLmsOtsParam = NULL;
	u32 TotalLevelsInHssPublicKey = XSECURE_ALLFS;
	const u8* TmpSignaturePtr = NULL;
	u32 CurrentLevelSignLen = 0U;
	u32 Index = 0U;
	u32 TotalLevelsInSignature = 0U;
	CurrentLmsQ = XSECURE_ALLFS;
	SignatureLengthConsumed = 0U;
	XSecure_LmsSignVerifyParams LmsSignVerifyParams;


	/* ****************************************************************************************** */
	/* Public key processing - sequence */
	/* ****************************************************************************************** */

	/* Length should have at least levels information */
	if (XSECURE_LMS_HSS_LEVELS_FIELD_SIZE > HssInitParams->PublicKeyLen) {
		Status = XSECURE_LMS_INVALID_PARAM;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS HSS Init - public key Length error 1 0x%x\n\r",
			HssInitParams->PublicKeyLen);
		goto END;
	}

	/* Extracting Levels information from HSS public key, Big Endian to Little Endian */
	TotalLevelsInHssPublicKey = XSecure_SwapBytes(
				&HssInitParams->PublicKey[XSECURE_HSS_PUBLIC_KEY_LEVEL_FIELD_OFFSET],
				XSECURE_LMS_HSS_LEVELS_FIELD_SIZE);

	TmpPublicKeyPtr = (const u8*)&HssInitParams->PublicKey[XSECURE_HSS_PUBLIC_KEY_LMS_FIELD_OFFSET];

	/* Length check - excluding levels field, it should match the Length of LMS public key of that size */
	if (XSECURE_HSS_PUBLIC_KEY_TOTAL_SIZE != HssInitParams->PublicKeyLen) {
		Status = XSECURE_LMS_HSS_PUB_KEY_INVALID_LEN_2_ERROR;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS HSS Init - public key total Length error 2 0x%x\n\r",
			HssInitParams->PublicKeyLen);
		goto END;
	}

	Status = Xil_SecureZeroize((u8 *)(UINTPTR)&AuthenticatedKey.Buff[0U], sizeof(XSecure_LmsPublicKey));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* ****************************************************************************************** */
	/* Signature processing - sequence */
	/* ****************************************************************************************** */

	/* Length should have atleast levels mentioned */
	if (XSECURE_LMS_HSS_LEVELS_FIELD_SIZE > HssInitParams->SignatureLen) {
		Status = XSECURE_LMS_INVALID_PARAM;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS HSS Init - signature Length error 1 0x%x\n\r",
			HssInitParams->SignatureLen);
		goto END;
	}

	/* tracking signature usage location in pointer */
	TmpSignaturePtr = (const u8*)HssInitParams->SignBuff;

	/* HSS levels mentioned in signature list, Big Endian to Little Endian */
	TotalLevelsInSignature = (XSecure_SwapBytes(
			&HssInitParams->SignBuff[XSECURE_HSS_SIGN_LIST_LEVEL_FIELD_OFFSET],
			XSECURE_LMS_HSS_LEVELS_FIELD_SIZE) + 1U);

	/* Track usage */
	TmpSignaturePtr += XSECURE_LMS_HSS_LEVELS_FIELD_SIZE;
	SignatureLengthConsumed += XSECURE_LMS_HSS_LEVELS_FIELD_SIZE;

	if ((TotalLevelsInSignature < XSECURE_LMS_HSS_MIN_LEVELS_SUPPORTED)
		|| (TotalLevelsInSignature > XSECURE_LMS_HSS_MAX_LEVELS_SUPPORTED)) {
		Status = XSECURE_LMS_HSS_SIGN_LEVEL_UNSUPPORTED_ERROR;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS HSS Init - signature levels un-supported 0x%x\n\r",
			TotalLevelsInSignature);
		goto END;
	}

	if (TotalLevelsInSignature != TotalLevelsInHssPublicKey) {
		Status = XSECURE_LMS_HSS_SIGN_PUB_KEY_LEVEL_MISMATCH_ERROR;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS HSS Init - levels mismatch signature - 0x%x, pub key - 0x%x\n\r",
			TotalLevelsInSignature, TotalLevelsInHssPublicKey);
		goto END;
	}

	/* Now we have to parse through 0 to (Levels - 1) and verify the public keys
	   using signatures at each level, but PLM only supports 2 levels, so need to run the verification
	   only once */
	/**
	 * Signature structure
	 * u32str(Nspk) || signed_pub_key[0] || ... || signed_pub_key[Nspk-1] || sig[Nspk]
	 * u32str(Nspk) || sig[0] || pub[1] || ... || sig[Nspk-1] || pub[Nspk] || sig[Nspk]
	 *
	 * Public key of lower level is used as a message to be authenticated, and signature of that
	 * level to back calculate the public key at that level. if this is happening for top most
	 * tree then value will be HSS public key saved as PPK or SPK, in turn all the levels above
	 * [levels - 1] are verified, so when the actual message is verified using lowest tree's
	 * leaf node, as the public key for that tree (lowest) is already verified, message is
	 * authenticated.
	 *
	 * PLM Supports two levels trees, level-0 & 1 namely.
	 * For KAT, only 1 levels is required, which
	 * is level-0. for KAT the following condition is skipped
	 */

	for (Index = 0U; Index < (TotalLevelsInSignature - 1U); Index++) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS HSS Init - Loop iteration - 0x%x\n\r", Index);

		/* Extracting Current level Public key's LMS Type & LMS OTS Type,
		 * Big Endian to Little Endian
		 */
		PublicKeyLmsType = (XSecure_LmsType)XSecure_SwapBytes(
				&TmpPublicKeyPtr[XSECURE_LMS_PUB_KEY_TYPE_OFFSET],
				XSECURE_LMS_TYPE_SIZE);
		PublicKeyLmsOtsType = (XSecure_LmsOtsType)XSecure_SwapBytes(
				&TmpPublicKeyPtr[XSECURE_LMS_PUB_KEY_OTS_TYPE_OFFSET],
				XSECURE_LMS_OTS_TYPE_SIZE);

		/**
		 * Parse HSS/Current level's public key provided,
		 * determine the expected signature length of sig[0]
		 */

		/* Fetch the type of public key,
		 * if not a valid/supported type return error */
		Status = XSecure_LmsLookupParamSet(PublicKeyLmsType,
			(XSecure_LmsParam**)&PubKeyLmsParam);
		if (Status != XST_SUCCESS) {
			Status = XSECURE_LMS_HSS_L0_PUB_KEY_LMS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}

		/* Fetch the Type of public key, if not a valid/supported Type return error */
		Status = XSecure_LmsOtsLookupParamSet(PublicKeyLmsOtsType,
			(XSecure_LmsOtsParam**)&PubKeyLmsOtsParam);
		if (Status != XST_SUCCESS) {
			Status = XSECURE_LMS_HSS_L0_PUB_KEY_LMS_OTS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}

		/* Before we proceed to LMS OTS, we also need to check if SHA algo selected in BH matches the
		   type selected in signature,
		   currently we do not support combinations of HASH algorithm */
		if ((PubKeyLmsParam->H != PubKeyLmsOtsParam->H) ||
			(ShaInstPtr->HashAlgo != PubKeyLmsOtsParam->H)) {
			Status = XSECURE_LMS_SIGN_VERIFY_BH_AND_TYPE_SHA_ALGO_MISMATCH_L0_ERROR;
			goto END;
		}

		CurrentLevelSignLen = (PubKeyLmsOtsParam->SignLen +
					XSECURE_LMS_Q_FIELD_SIZE +
					XSECURE_LMS_TYPE_SIZE +
					PubKeyLmsParam->mh);

		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS HSS Init - Current Level %u signature length - 0x%x\n\r",
			Index,
			CurrentLevelSignLen);
		/*
		 * Signature Length should be equal or more than required,
		 * should have atleast sig[0] + pub[1] Length
		 */
		if (HssInitParams->SignatureLen < (XSECURE_LMS_HSS_LEVELS_FIELD_SIZE +
				CurrentLevelSignLen +
				XSECURE_LMS_PUB_KEY_TOTAL_SIZE +
				SignatureLengthConsumed)) {
			Status = XSECURE_LMS_HSS_L0_SIGN_INVALID_LEN_2_ERROR;
			XSecure_Printf(XSECURE_DEBUG_GENERAL,
				"LMS HSS Init - signature Length error level %u - 0x%x\n\r",
				Index,
				HssInitParams->SignatureLen);
			goto END;
		}

		LmsSignVerifyParams.Data = (u8 *)&TmpSignaturePtr[CurrentLevelSignLen];
		LmsSignVerifyParams.DataLen = XSECURE_LMS_PUB_KEY_TOTAL_SIZE;
		LmsSignVerifyParams.PreHashedMsg = FALSE;
		LmsSignVerifyParams.LmsSign = (u8 *)TmpSignaturePtr;
		LmsSignVerifyParams.LmsSignLen = CurrentLevelSignLen;
		LmsSignVerifyParams.ExpectedPubKey = (u8 *)TmpPublicKeyPtr;
		LmsSignVerifyParams.PubKeyLen = XSECURE_LMS_PUB_KEY_TOTAL_SIZE;

		/* Now that we have checked if signature has sig[0] & pub[1] contents verify pub[1] as data to
		   be authenticated, with sig[0], output of operation should match with HSS public key (either spk
		   or ppk in case of ROM) */
		Status = XSecure_LmsSignatureVerification(ShaInstPtr, DmaPtr, &LmsSignVerifyParams);
		if (Status != XST_SUCCESS) {
			Status = XSECURE_LMS_HSS_L0_PUB_KEY_AUTH_FAILED_ERROR;
			goto END;
		}

		/* At this point the signature has a valid Length signature
		 * for a valid Length public key for a lower level tree,
		 * which is all verified
		 */

		XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS HSS Init - LMS verification complete for tree at level 0x%x\n\r", Index);

		/* Updating signature pointer for next loop, current signature length + public key length */
		TmpPublicKeyPtr = (TmpSignaturePtr + CurrentLevelSignLen);
		TmpSignaturePtr += (CurrentLevelSignLen + XSECURE_LMS_PUB_KEY_TOTAL_SIZE);
		SignatureLengthConsumed += (CurrentLevelSignLen + XSECURE_LMS_PUB_KEY_TOTAL_SIZE);
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS HSS Init - pointers update at level %u, public key 0x%x, signature 0x%x, len consumed 0x%x\n\r",
			Index,
			TmpPublicKeyPtr,
			TmpSignaturePtr,
			SignatureLengthConsumed);
	}

	/*
	* Copy authenticated key
	* 	- If Incrementing Index is glitched, we re-copy same location
	* 	- If copy fails, read back and check will catch it
	*/

	/* As T field of pub[1]/pub[Npsk] is authenticated, now let's copy rest of pub[1] contents for further use */
	for (Index = 0U; Index < XSECURE_LMS_PUB_KEY_TOTAL_SIZE; Index++) {
		AuthenticatedKey.Buff[Index] = TmpPublicKeyPtr[Index];
		if (TmpPublicKeyPtr[Index] != AuthenticatedKey.Buff[Index]) {
			Status = XSECURE_LMS_PUB_OP_FAILED_ERROR;
			XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS HSS Init - public key copy failed location 0x%x\n\r", Index);
			goto END;
		}
	}

	if (XSECURE_LMS_PUB_KEY_TOTAL_SIZE != Index) {
		Status = XSECURE_LMS_PUB_OP_FAILED_1_ERROR;
		goto END;
	}

	/* Extract 'q' from lowest level Signature Npsk, for use in Data's pre-processing before authentication,
	   Big Endian to Little Endian */
	CurrentLmsQ = XSecure_SwapBytes(&TmpSignaturePtr[XSECURE_LMS_SIGNATURE_Q_FIELD_OFFSET],
		XSECURE_LMS_Q_FIELD_SIZE);
	DigestPrefixFields.Fields.q = Xil_Htonl(CurrentLmsQ);
	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS HSS Init - 'q' extracted from Signature Npsk 0x%x\n\r", CurrentLmsQ);

	/* Extract 'I' from latest authenticated public key, for use in Data's pre-processing before authentication */
	Status = Xil_SMemCpy((void*)DigestPrefixFields.Fields.I,
		XSECURE_LMS_I_FIELD_SIZE,
		(void*)AuthenticatedKey.Fields.I,
		XSECURE_LMS_I_FIELD_SIZE,
		XSECURE_LMS_I_FIELD_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Extract 'C' from lowest level Signature Npsk, for use in Data's pre-processing before authentication */
	Status = Xil_SMemCpy((void*)DigestPrefixFields.Fields.C,
		XSECURE_LMS_C_FIELD_SIZE,
		(void*)&TmpSignaturePtr[XSECURE_LMS_SIGNATURE_OTS_FIELD_OFFSET + XSECURE_LMS_OTS_SIGN_C_FIELD_OFFSET],
		XSECURE_LMS_C_FIELD_SIZE,
		XSECURE_LMS_C_FIELD_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Extracting LMS & LMS OTS Type from Public key, Big Endian to Little Endian */
	PublicKeyLmsType = (XSecure_LmsType)XSecure_SwapBytes((const void*)&AuthenticatedKey.Buff[XSECURE_LMS_PUB_KEY_TYPE_OFFSET],
		XSECURE_LMS_TYPE_SIZE);
	PublicKeyLmsOtsType = (XSecure_LmsOtsType)XSecure_SwapBytes(&TmpPublicKeyPtr[XSECURE_LMS_PUB_KEY_OTS_TYPE_OFFSET],
		XSECURE_LMS_OTS_TYPE_SIZE);

	/* Fetch the Type of public key, if not a valid/supported Type return error */
	Status = XSecure_LmsLookupParamSet(PublicKeyLmsType,
			(XSecure_LmsParam**)&PubKeyLmsParam);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_LMS_HSS_L1_PUB_KEY_LMS_TYPE_1_UNSUPPORTED_ERROR;
		goto END;
	}

	/* Fetch the Type of public key, if not a valid/supported Type return error */
	Status = XSecure_LmsOtsLookupParamSet(PublicKeyLmsOtsType,
			(XSecure_LmsOtsParam**)&PubKeyLmsOtsParam);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_LMS_HSS_L0_PUB_KEY_LMS_OTS_TYPE_UNSUPPORTED_ERROR;
		goto END;
	}

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS HSS Init - SHA mode init for data authentication 0x%x\n\r", PubKeyLmsParam->H);

	/* Initialize HASH to calculate digest */
	Status = XSecure_ShaInitialize(ShaInstPtr, DmaPtr);

END:
	return Status;
}

/******************************************************************************/
/**
* @brief	This function calculates the Digest of data to authenticate
* 		to initiate the process of LMS verification
*
* @param	Data	- Data to be authenticated
* @param	DataLen	- Length of data to be authenticated
*
* @return
*	-	@ref XST_SUCCESS - If operation success, otherwise following errors
*******************************************************************************/
int XSecure_LmsHashMessage(XSecure_Sha *ShaInstPtr,
	u8* Data, u32 DataLen, XSecure_ShaMode Mode)
{
	int Status = XST_FAILURE;
	static const u8 XSECURE_D_MESG[2U] = { 0x81U, 0x81U };

	/* Process @ref XSECURE_D_MESG, to be sent in Big Endian - XSECURE_D_MESG value is same*/
	DigestPrefixFields.Fields.D_MESG[0U] = XSECURE_D_MESG[0U];
	DigestPrefixFields.Fields.D_MESG[1U] = XSECURE_D_MESG[1U];

	/* Start SHA to calculate digest */
	Status = XSecure_ShaStart(ShaInstPtr, Mode);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Process prefix */
	if (DataLen == 0U) {
		Status = XSecure_ShaLastUpdate(ShaInstPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XSecure_ShaUpdate(ShaInstPtr, (u64)(UINTPTR)DigestPrefixFields.Buff,
		XSECURE_LMS_MESSAGE_TO_DIGEST_PREFIX_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (0U != DataLen) {
		/* Process data now */
		Status = XSecure_ShaLastUpdate(ShaInstPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_ShaUpdate(ShaInstPtr, (u64)(UINTPTR)Data, DataLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XSecure_ShaFinish(ShaInstPtr,
		(u64)(UINTPTR)&DigestChecksum.Buff,
		XSECURE_LMS_DIGEST_SIZE);

END:
	return Status;
}

/******************************************************************************/
/**
* @brief	This function Completes LMS Signature verification process.
*		Data should have been pre-processed before calling this function, by
*		calling @ref XSecure_HssInit & @ref XSecure_LmsHashMessage in sequence
*
* @param	SignBuff	- Pointer to Signature buffer
* @param	SignatureLen	- Length of signature buffer
*
* @return
*		XST_SUCCESS - If operation success, otherwise following errors
*******************************************************************************/
int XSecure_HssFinish(XSecure_Sha *ShaInstPtr,
	XPmcDma *DmaPtr, u8* SignBuff,
	u32 SignatureLen)
{
	int Status = XST_FAILURE;
	const XSecure_LmsParam* PskPubKeyLmsParam = NULL;
	const XSecure_LmsOtsParam* PskPubKeyLmsOtsParam = NULL;
	XSecure_LmsType PublicKeyLmsType = XSECURE_LMS_NOT_SUPPORTED;
	XSecure_LmsOtsType PublicKeyLmsOtsType = XSECURE_LMS_OTS_NOT_SUPPORTED;
	u32 SignNpskLen = 0U;
	volatile u32 Sign1Len = (SignatureLen - SignatureLengthConsumed);
	XSecure_LmsSignVerifyParams LmsSignVerifyParams;

	PublicKeyLmsType = (XSecure_LmsType)XSecure_SwapBytes(
		(const void*)&AuthenticatedKey.Buff[XSECURE_LMS_PUB_KEY_TYPE_OFFSET],
		XSECURE_LMS_TYPE_SIZE);
	PublicKeyLmsOtsType = (XSecure_LmsOtsType)XSecure_SwapBytes(
		(const void*)&AuthenticatedKey.Buff[XSECURE_LMS_PUB_KEY_OTS_TYPE_OFFSET],
		XSECURE_LMS_OTS_TYPE_SIZE);

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS HSS Finish - LMS Type from public key 0x%x\n\r", PublicKeyLmsType);
	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS HSS Finish - LMS OTS Type from public key 0x%x\n\r", PublicKeyLmsOtsType);

	/* Fetch the Type of public key, if not a valid/supported Type return error */
	Status = XSecure_LmsLookupParamSet(PublicKeyLmsType,
			(XSecure_LmsParam**)&PskPubKeyLmsParam);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_LMS_HSS_L1_PUB_KEY_LMS_TYPE_2_UNSUPPORTED_ERROR;
		goto END;
	}

	/* Fetch the Type of public key, if not a valid/supported Type return error */
	Status = XSecure_LmsOtsLookupParamSet(PublicKeyLmsOtsType,
			(XSecure_LmsOtsParam**)&PskPubKeyLmsOtsParam);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_LMS_HSS_L1_PUB_KEY_LMS_OTS_TYPE_UNSUPPORTED_ERROR;
		goto END;
	}

	/** Signature Length should be at least 4 bytes */
	if (XSECURE_LMS_OTS_TYPE_SIZE > Sign1Len) {
		Status = XSECURE_LMS_HSS_OTS_SIGN_INVALID_LEN_1_ERROR;
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS HSS Finish - Level 1 signature error 1 0x%x\n\r", Sign1Len);
		goto END;
	}

	/* ****************************************************************************************** */
	/* Signature checks */
	/* ****************************************************************************************** */

	/* After pub[Npsk], sig[Npsk] fields start, from there the following is check for length required */
	SignNpskLen = (PskPubKeyLmsOtsParam->SignLen +
		XSECURE_LMS_Q_FIELD_SIZE +
		XSECURE_LMS_TYPE_SIZE +
		PskPubKeyLmsParam->mh);

	/* Signature Length should be equal to sig[0] + pub[Npsk] Length + sig[Npsk] */
	if (Sign1Len != SignNpskLen) {
		Status = XSECURE_LMS_HSS_L1_SIGN_INVALID_LEN_2_ERROR;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"LMS HSS Finish - Level 1 signature error 2 actual 0x%x, expected 0x%x\n\r",
			Sign1Len,
			SignNpskLen);
		goto END;
	}

	LmsSignVerifyParams.Data = NULL;
	LmsSignVerifyParams.DataLen = 0U;
	LmsSignVerifyParams.PreHashedMsg = TRUE;
	LmsSignVerifyParams.LmsSign = (u8 *)&SignBuff[SignatureLengthConsumed];
	LmsSignVerifyParams.LmsSignLen = SignNpskLen;
	LmsSignVerifyParams.ExpectedPubKey = (u8 *)AuthenticatedKey.Buff;
	LmsSignVerifyParams.PubKeyLen = XSECURE_LMS_PUB_KEY_TOTAL_SIZE;


	/* Now that we have checked if signature has sig[Npsk],
	 * verify the already authenticated and stored public key and data
	 */
	Status = XSecure_LmsSignatureVerification(ShaInstPtr,
			DmaPtr, &LmsSignVerifyParams);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_LMS_HSS_L0_PUB_KEY_AUTH_FAILED_ERROR;
		goto END;
	}

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "LMS HSS Finish - Data Authenticated using LMS\n\r");

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function get bytes from buffer,
 * 		to a unsigned int value, Changes endianness
 *
 * @param	source - pointer to source buffer
 * @param	bytes -  number of bytes to be extracted
 *
 * @return
 *		u32 - resultant value
 *
 ******************************************************************************/
static u32 XSecure_SwapBytes(const u8 *const source, size_t bytes)
{
    const u8* const b = source;
    u32 result = 0U;
    u32 i;

    for (i = 0; i < bytes; i++) {
        result = ((256U * result) + (b[i] & 0xFFU));
    }

    return result;
}

/******************************************************************************/
/**
* @brief	This function returns public key LMS type.
*
* @param	PubAlgo	- Algorithm selected by current PDI
* @param	PubKey	- pointer to PPK location, PPK is used to detect the variant of LMS selected
*
* @return
*	-	@ref XST_SUCCESS - If operation success, otherwise following errors
*	-	@ref XSECURE_LMS_KAT_PUB_KEY_UNSUPPORTED_LMS_TYPE_ERROR
*******************************************************************************/
int XSecure_GetLmsHashAlgo(u32 PubAlgo, const u8* const PubKey, XSecure_ShaMode *SignAlgo)
{
	int Status = XST_FAILURE;

	/** Public key's LMS type */
	XSecure_LmsType PublicKeyLmsType = XSECURE_LMS_NOT_SUPPORTED;
	/** LMS Parameters extracted from Public Key */
	const XSecure_LmsParam* PubKeyLmsParam = NULL;

	if(XSECURE_PUB_ALGO_LMS_HSS == PubAlgo) {
		PublicKeyLmsType =  (XSecure_LmsType)XSecure_SwapBytes(&PubKey[XSECURE_HSS_PUBLIC_KEY_LMS_FIELD_OFFSET],
			XSECURE_LMS_TYPE_SIZE);
	}
	else if(XSECURE_PUB_ALGO_LMS == PubAlgo) {
		PublicKeyLmsType =  (XSecure_LmsType)XSecure_SwapBytes(&PubKey[XSECURE_LMS_PUB_KEY_TYPE_OFFSET],
		XSECURE_LMS_TYPE_SIZE);
	}

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "Pub Key LMS Type 0x%x\n\r", PublicKeyLmsType);
	/* Fetch the Type of public key, if not a valid/supported Type return error */
	Status = XSecure_LmsLookupParamSet(PublicKeyLmsType,
			(XSecure_LmsParam**)&PubKeyLmsParam);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_LMS_HSS_L0_PUB_KEY_LMS_TYPE_UNSUPPORTED_ERROR;
		goto END;
	}

	*SignAlgo = PubKeyLmsParam->H;

END:
	return Status;

}

/** @} */
