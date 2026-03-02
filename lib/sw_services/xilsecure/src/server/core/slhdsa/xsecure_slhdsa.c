/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_slhdsa.c
*
* This file contains the software implementation of the signature verification functionality for
* SLHDSA driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  02/19/26 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_slhdsa_server_apis XilSecure SLHDSA Server APIs
* @{
*/
/*************************************** Include Files ********************************************/
#include "xsecure_slhdsa_fors.h"
#include "xsecure_slhdsa_ht.h"
#include "xsecure_slhdsa_hash.h"
#include "xsecure_slhdsa.h"
#include "xsecure_sha.h"
#include "xsecure_utils.h"
#include "xil_io.h"

/************************************ Constant Definitions ****************************************/
/* Macros for offsets in Data1 buffer for SLH-DSA verify context/message prefix */
#define XSECURE_SLH_DSA_VERIFY_PREFIX_OFFSET_DS		0U	/**< Domain separation offset */
#define XSECURE_SLH_DSA_VERIFY_PREFIX_OFFSET_CTXLEN	1U	/**< Context length offset */
#define XSECURE_SLH_DSA_VERIFY_PREFIX_OFFSET_CTX	2U	/**< Context string offset */

/**************************************** Function Prototypes *************************************/
static int XSecure_SlhdsaVerifyInternal(void);

/************************************* Variable Definitions ***************************************/

/************************************* Function Definitions ***************************************/

/**************************************************************************************************/
/**
 * @brief	Internal SLH-DSA signature verification (FIPS 205 Algorithm 20).
 *		Gets signature and public key addresses from the instance structure.
 *
 * @return
 *		- XST_SUCCESS On successful signature verification.
 *		- ErrorCode On failure.
 *
 **************************************************************************************************/
static int XSecure_SlhdsaVerifyInternal(void)
{
	volatile int Status = XST_FAILURE;
	volatile int ClrStatus = XST_FAILURE;

	XSecure_SlhdsaInstance *InstancePtr = XSecure_SlhdsaGetInstance();
	XSecure_SlhdsaHtIndices HtIndices;

	/*
	 * Step 6: SIG_FORS starts at n, SIG[n:(1+k(1+a))*n]; Position in signature buffer where
	 * FORS relevant portion starts.
	 */
	const u32 ForsOffset = (u32)InstancePtr->Param->n;

	/*
	 * Step 7: SIG_HT offset, (1+k(1+a)) * n, position in signature where HyperTree's relevant
	 * portion starts. For SHAKE-256s FIPS 205 Table 2: k=22, a=14, n=32, so
	 * (1 + 22*(1+14))*32 = 10592 bytes
	 */
	const u32 HtOffset = (XSECURE_VALUE_ONE + ((u32)InstancePtr->Param->k * (XSECURE_VALUE_ONE
				+ (u32)InstancePtr->Param->a))) * (u32)InstancePtr->Param->n;

	/*
	 * Step 9: md <- digest[0 : ceil(k * a/8)]
	 * For SHAKE-256s FIPS 205 Table 2: k=22, a=14, so ceil(22*14/8) = 39 bytes
	 */
	const u32 mdSize = XSecure_SlhCeilDivU32(((u32)InstancePtr->Param->k *
				(u32)InstancePtr->Param->a), XSECURE_BYTE_IN_BITS);

	/*
	 * Used to store size, number of bytes to represent TmpIdxTree
	 * For SHAKE-256s FIPS 205 Table 2: h=64, d=8, so
	 * ceil((h-h/d)/8) = ceil((64-64/8)/8) = 7 bytes
	 */
	const u32 idTreeSize = XSecure_SlhCeilDivU32(((u32)InstancePtr->Param->h -
				(u32)InstancePtr->Param->hprime), XSECURE_BYTE_IN_BITS);
	/* Pointer to digest buffer location where TmpIdxTree value starts */
	const u8* TmpIdxTree;
	/* Resultant IdxTree Value */
	u64 IdxTree;

	/*
	 * Used to store size, number of bytes to represent TmpIdxLeaf
	 * For SHAKE-256s FIPS 205 Table 2: h=64, d=8, so ceil(h/(8d)) = ceil(64/(8*8)) = 1 byte
	 */
	const u32 idLeafSize = XSecure_SlhCeilDivU32((u32)InstancePtr->Param->h,
				(XSECURE_BYTE_IN_BITS * (u32)InstancePtr->Param->d));
	/* Pointer to digest buffer location where TmpIdxLeaf value starts */
	const u8* TmpIdxLeaf;
	/* Resultant IdxLeaf Value */
	u64 IdxLeaf;

	/*
	 * Step 1: if |SIG| != (1+k(1+a)+h+d * len) * n then
	 * For SHAKE-256s FIPS 205 Table 2: k=22, a=14, h=64, d=8, len=67, n=32
	 * Signature length = (1 + k*(1+a) + h + d*len)*n
	 * = (1 + 22*(1+ 14) + 64 + 8*67)*32 = 29792 bytes
	 */
	if (InstancePtr->Param->SignLen != InstancePtr->SignatureLen) {
		/* Step 2: return false */
		Status = XSECURE_SLHDSA_SIGN_LEN_ERROR;
		goto END;
		/* Step 3: end if */
	}

	/* Step 4: ADRS <- toByte(0, 32) */
	XSecure_SlhdsaClearAddress(InstancePtr->Addr);

	/*
	 * Step 8: digest <- H_msg(R, PK.seed, PK.root, M)
	 * R from signature, Prefix from Data1, and actual data from instance addresses
	 * Output in Data2
	 */
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_SlhdsaShake256sHashMsg);

	/* Step 10: tmp_idxtree <- digest[ceil(k*a/8) : ceil(k*a/8) + ceil((h-h/d)/8)] */
	TmpIdxTree = &InstancePtr->Data2[mdSize];

	/*
	 * Step 11: tmp_idxleaf <- digest[ceil(k*a/8) + ceil((h-h/d)/8) :
	 *				 ceil(k*a/8) + ceil((h-h/d)/8) + ceil(h/(8d))]
	 */
	TmpIdxLeaf = &InstancePtr->Data2[mdSize + idTreeSize];

	/*
	 * Step 12: idxtree <- toInt(tmp_idxtree, ceil((h-h/d)/8)) mod 2^(h-h/d)
	 * For SHAKE-256s FIPS 205 Table 2: idTreeSize = 7 bytes, h=64, d=8, so h-h/d=56
	 */
	IdxTree = XSecure_ModPow2U64(XSecure_BytesToU64(TmpIdxTree, idTreeSize),
			((u32)InstancePtr->Param->h - (u32)InstancePtr->Param->hprime));

	/*
	 * Step 13: idxleaf <- toInt(tmp_idxleaf, ceil(h/(8d))) mod 2^(h/d)
	 * For SHAKE-256s FIPS 205 Table 2: idLeafSize = 1 byte, h=64, d=8, so h/d=8
	 */
	IdxLeaf = XSecure_ModPow2U64(XSecure_BytesToU64(TmpIdxLeaf, idLeafSize),
			((u32)InstancePtr->Param->h / (u32)InstancePtr->Param->d));

	/* Step 14: ADRS.setTreeAddress(idxtree) */
	XSecure_SlhdsaSetTreeAddress(InstancePtr->Addr, IdxTree);

	/* Step 15: ADRS.setTypeAndClear(FORS_TREE) */
	XSecure_SlhdsaSetTypeAndClear(InstancePtr->Addr, XSECURE_SLH_DSA_ADRS_TYPE_FORS_TREE);

	/* Step 16: ADRS.setKeyPairAddress(idxleaf) */
	XSecure_SlhdsaSetKeyPairAddress(InstancePtr->Addr, (u32)IdxLeaf);

	/* Step 17: PK_FORS <- fors_pkFromSig(SIG_FORS, md, PK.seed, ADRS) */
	/* Pass u64 addresses for IPC compatibility */
	XSECURE_TEMPORAL_CHECK(END,
			Status,
			XSecure_SlhdsaForsPkFromSig,
			InstancePtr->SignatureAddr + ForsOffset,	/* [in] SIG_FORS address */
			InstancePtr->Data2,				/* [in] md */
			InstancePtr->PublicKeyAddr,			/* [in] PK.seed address */
			InstancePtr->Data3);				/* [out] PK_FORS */

	/* Step 18: return ht_verify(PK_FORS, SIG_HT, PK.seed, idxtree, idxleaf, PK.root) */
	HtIndices.IdxTree = IdxTree;
	HtIndices.IdxLeaf = IdxLeaf;

	XSECURE_TEMPORAL_CHECK(END,
			Status,
			XSecure_SlhdsaHtVerify,
			InstancePtr->Data3,			/* [in] PK_FORS */
			InstancePtr->SignatureAddr + HtOffset,	/* [in] SIG_HT address */
			InstancePtr->PublicKeyAddr,		/* [in] PK.seed address */
			&HtIndices);				/* [in] Hypertree indices */

END:
	/* Clear Data1 */
	ClrStatus = Xil_SecureZeroize((u8 *)(UINTPTR)InstancePtr->Data1,
			XSECURE_SLHDSA_MAX_DATA1_LEN_IN_BYTES);
	if ((Status == XST_SUCCESS) && (ClrStatus != XST_SUCCESS)) {
		Status = ClrStatus;
	}
	InstancePtr->Data1Len = XSECURE_ZERO;

	/* Clear Data2 */
	ClrStatus = Xil_SecureZeroize((u8 *)(UINTPTR)InstancePtr->Data2,
			XSECURE_SLHDSA_MAX_DATA2_LEN_IN_BYTES);
	if ((Status == XST_SUCCESS) && (ClrStatus != XST_SUCCESS)) {
		Status = ClrStatus;
	}

	/* Clear Data3 */
	ClrStatus = Xil_SecureZeroize((u8 *)(UINTPTR)InstancePtr->Data3,
			XSECURE_SLHDSA_MAX_DATA3_LEN_IN_BYTES);
	if ((Status == XST_SUCCESS) && (ClrStatus != XST_SUCCESS)) {
		Status = ClrStatus;
	}

	/* Clear Data4 */
	ClrStatus = Xil_SecureZeroize((u8 *)(UINTPTR)InstancePtr->Data4,
			XSECURE_SLHDSA_MAX_DATA4_LEN_IN_WORDS * XSECURE_WORD_LEN);
	if ((Status == XST_SUCCESS) && (ClrStatus != XST_SUCCESS)) {
		Status = ClrStatus;
	}

	/* Clear address structure for security */
	ClrStatus = Xil_SecureZeroize((u8 *)(UINTPTR)InstancePtr->Addr, sizeof(ADRS));
	if ((Status == XST_SUCCESS) && (ClrStatus != XST_SUCCESS)) {
		Status = ClrStatus;
	}

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function verifies an SLH-DSA signature for a given message using the provided
 *		public key. It implements the pure SLH-DSA verification algorithm with optional
 *		context string support.
 *
 * @param	ShaInstPtr	Pointer to initialized SHA instance.
 * @param	SlhdsaParams	Pointer to structure containing all SLH-DSA verification parameters:
 *			- DataAddr: 64-bit address of message data buffer to be verified
 *			- DataLen: Length of message data in bytes
 *			- SignatureAddr: 64-bit address of signature buffer (R || SIG_FORS || SIG_HT)
 *			- SignatureLen: Length of signature in bytes
 *			- ContextAddr: 64-bit address of optional context string buffer (can be 0)
 *			- ContextLen: Length of context string in bytes (must be <= 255)
 *			- PublicKeyAddr: 64-bit address of public key buffer (PK.seed || PK.root)
 *			- PublicKeyLen: Length of public key in bytes (must be 2n)
 *
 * @return
 *		- XST_SUCCESS On successful signature verification.
 *		- ErrorCode On failure.
 *
 **************************************************************************************************/
int XSecure_SlhdsaVerify(XSecure_Sha *ShaInstPtr,
			 const XSecure_SlhdsaInputParams * const SlhdsaParams)
{
	volatile int Status = XST_FAILURE;
	u32 Idx;
	XSecure_SlhdsaInstance *InstancePtr = XSecure_SlhdsaGetInstance();

	if (InstancePtr == NULL) {
		Status = XSECURE_SLHDSA_INVALID_PARAM;
		goto END;
	}

	if (ShaInstPtr == NULL) {
		Status = XSECURE_SLHDSA_INVALID_PARAM;
		goto END;
	}

	InstancePtr->ShaInstance = ShaInstPtr;

	if (InstancePtr->Param == NULL) {
		Status = XSECURE_SLHDSA_INVALID_PARAM;
		goto END;
	}

	if (SlhdsaParams == NULL) {
		Status = XSECURE_SLHDSA_INVALID_PARAM;
		goto END;
	}

	if (SlhdsaParams->PublicKeyLen != (XSECURE_VALUE_TWO * (u32)InstancePtr->Param->n)) {
		Status = XSECURE_SLHDSA_PK_LEN_ERROR;
		goto END;
	}

	/* Step 1: if |ctx| > 255 then */
	if (SlhdsaParams->ContextLen > XSECURE_SLH_DSA_CTX_MAX_LEN_BYTES) {
		/* Step 2: return false */
		Status = XSECURE_SLHDSA_CTX_LEN_ERROR;
		goto END;
	}
	/* Step 3: end if */

	/*
	 * Step 4: M' <- toByte(0,1) || toByte(|ctx|, 1) || ctx || M
	 * Set domain separation byte for SLH-DSA verification (Algorithm 24, step 4)
	 */
	InstancePtr->Data1[XSECURE_SLH_DSA_VERIFY_PREFIX_OFFSET_DS] = XSECURE_SLH_DSA_PURE_SLH_VERIFY_DS;
	InstancePtr->Data1Len = XSECURE_SLH_DSA_DS_LEN_BYTES;

	/* Store context length as a single byte (Algorithm 24, step 4) */
	InstancePtr->Data1[XSECURE_SLH_DSA_VERIFY_PREFIX_OFFSET_CTXLEN] = (u8)SlhdsaParams->ContextLen;
	InstancePtr->Data1Len += XSECURE_SLH_DSA_CTX_LEN_FIELD_LEN_BYTES;

	/* Copy context string to buffer using XSecure_InByte64() for IPC compatibility */
	for (Idx = 0U; Idx < SlhdsaParams->ContextLen; Idx++) {
		InstancePtr->Data1[XSECURE_SLH_DSA_VERIFY_PREFIX_OFFSET_CTX + Idx] =
			XSecure_InByte64(SlhdsaParams->ContextAddr + Idx);
	}
	InstancePtr->Data1Len += Idx;

	/* Store addresses in instance for IPC compatibility */
	InstancePtr->DataAddr = SlhdsaParams->DataAddr;
	InstancePtr->DataLen = SlhdsaParams->DataLen;
	InstancePtr->SignatureAddr = SlhdsaParams->SignatureAddr;
	InstancePtr->PublicKeyAddr = SlhdsaParams->PublicKeyAddr;
	InstancePtr->SignatureLen = SlhdsaParams->SignatureLen;

	/* Step 5: return slh_verify_internal(M', SIG, PK) */
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_SlhdsaVerifyInternal);

END:
	return Status;
}
/** @} */
