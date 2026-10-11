/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xslhdsa_core.c
*
* This file contains the SLH-DSA FORS, WOTS+, XMSS and hypertree implementation for
* ASUFW firmware.
*
* These are the tree primitives the top-level operations are built from:
* - FORS (Algorithms 14 to 16): the bottom-layer few-time signature.
* - WOTS+ (Algorithms 4 to 7): the one-time signature used at every XMSS leaf.
* - XMSS (Algorithms 8 to 10): the Merkle tree over WOTS+ public keys.
* - Hypertree (Algorithms 11 to 13): the d-layer XMSS chain ending at PK.root.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   kk   07/10/26 Initial release
*       kk   09/18/26 Pass the yield unit to the slice checkpoints
*       kk   10/08/26 Addressed FMEA findings
*
* </pre>
*
*************************************************************************************************/
/*************************************** Include Files *******************************************/
/**
* @addtogroup xslhdsa_server_apis SLH-DSA Server APIs
* @{
*/
#include "xil_sutil.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xfih.h"
#include "xslhdsa.h"
#include "xslhdsa_core.h"
#include "xslhdsa_hash.h"

#ifdef XASU_SLHDSA_ENABLE
/************************************ Constant Definitions ***************************************/
/** Largest DigitWidth XSlhDsa_Base2b can extract; its accumulator holds DigitWidth + 7 bits. */
#define XSLHDSA_MAX_BASE2B_DIGIT_WIDTH	(25U)

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XSlhDsa_Base2b(const u8 *Input, u32 DigitWidth, u32 OutLen, u32 *DigitsOut);
static s32 XSlhDsa_CheckIndexBufferFits(u32 Count);
static s32 XSlhDsa_CheckScratchFits(u32 Count, u32 HashWidth);
static s32 XSlhDsa_ForsCheckDigestFits(const XSlhDsa_Param *ParamPtr);

static s32 XSlhDsa_ForsLeafHash(XSlhDsa_Instance *InstPtr, u32 ApplyF, u8 *Out);
static s32 XSlhDsa_ForsSubtreeRoot(XSlhDsa_Instance *InstPtr, u32 BaseLeaf,
					u32 SubHeight, u8 *Root);

static void XSlhDsa_WotsDecodeWin4Digits(const u8 *Src, u32 Count, u32 *DigitsOut);
static s32 XSlhDsa_WotsDecodeMsgDigits(const u8 *Msg,
					     const XSlhDsa_Param *ParamPtr,
					     u32 *DigitsOut);
static u32 XSlhDsa_WotsChecksumValue(const u32 *MsgDigits, u32 MsgChains,
					  u32 WinMax, u32 PadBits);
static s32 XSlhDsa_WotsExpandChecksumDigits(u32 CsVal,
						 const XSlhDsa_Param *ParamPtr,
						 u32 *CsDigitsOut);
static s32 XSlhDsa_WotsDeriveChainIndices(const u8 *Msg,
					       const XSlhDsa_Param *ParamPtr,
					       u32 *ChainIndicesOut);
static s32 XSlhDsa_WotsWriteSignChains(XSlhDsa_Instance *InstPtr,
					    const u32 *ChainIndices,
					    const XSlhDsa_Param *ParamPtr, u8 *SigOut);
static s32 XSlhDsa_WotsRebuildChainTops(XSlhDsa_Instance *InstPtr,
					     const u8 *WotsSign, const u32 *ChainIndices,
					     const XSlhDsa_Param *ParamPtr,
					     u8 *ScratchOut);

static s32 XSlhDsa_XmssBuildLeafPk(XSlhDsa_Instance *InstPtr, u32 LeafIdx,
					const XSlhDsa_Param *ParamPtr, u8 *ChainScratch,
					u8 *LeafPkOut);
static s32 XSlhDsa_XmssCollapseCarryLevels(XSlhDsa_Instance *InstPtr, u32 LeafIdx,
						u32 LeafSeq, u8 NodeStack[][XSLHDSA_MAX_N],
						s32 *StackDepthPtr, u8 *Root);
static s32 XSlhDsa_XmssIntegrateLeaf(XSlhDsa_Instance *InstPtr, u32 LeafIdx,
					  u32 LeafSeq, const XSlhDsa_Param *ParamPtr,
					  u8 NodeStack[][XSLHDSA_MAX_N],
					  s32 *StackDepthPtr, u8 *Root, u8 *ChainScratch);
static s32 XSlhDsa_XmssWriteWotsSignature(XSlhDsa_Instance *InstPtr,
					       const u8 *Msg, u32 IdxLeaf, u8 *WotsSigOut);
static s32 XSlhDsa_XmssWriteAuthPathNodes(XSlhDsa_Instance *InstPtr, u32 IdxLeaf,
					       const XSlhDsa_Param *ParamPtr,
					       u8 *AuthPathOut);
static s32 XSlhDsa_XmssRebuildLeafPk(XSlhDsa_Instance *InstPtr,
					  const u8 *SignXmss, const u8 *Msg, u32 IdxLeaf,
					  u8 *LeafPkOut);
static s32 XSlhDsa_XmssAscendAuthPath(XSlhDsa_Instance *InstPtr, u32 IdxLeaf,
					   const u8 *AuthPathPtr,
					   const XSlhDsa_Param *ParamPtr, u8 *Node);
static s32 XSlhDsa_XmssCheckLeaf(const XSlhDsa_Param *ParamPtr, u32 IdxLeaf);

static void XSlhDsa_HtMeasureDigestLayout(const XSlhDsa_Param *ParamPtr,
					       u32 *MdByteLen, u32 *TreeFieldLen,
					       u32 *LeafFieldLen);
static u64 XSlhDsa_HtExtractTreeIndex(const u8 *Digest, u32 MdByteLen, u32 TreeFieldLen,
					   const XSlhDsa_Param *ParamPtr);
static u64 XSlhDsa_HtExtractLeafIndex(const u8 *Digest, u32 MdByteLen, u32 TreeFieldLen,
					   u32 LeafFieldLen, const XSlhDsa_Param *ParamPtr);
static u32 XSlhDsa_HtLayerSigBytes(const XSlhDsa_Param *ParamPtr);
static void XSlhDsa_HtPeelLayerIndices(const XSlhDsa_Param *ParamPtr,
					    u64 *IdxTree, u32 *IdxLeafOut);
static s32 XSlhDsa_HtEmitBottomLayer(XSlhDsa_Instance *InstPtr, const u8 *LayerMsg,
					  u64 IdxTree, u32 IdxLeaf, u8 *LayerSigOut);
static s32 XSlhDsa_HtEmitUpperLayers(XSlhDsa_Instance *InstPtr,
					  const XSlhDsa_Param *ParamPtr, u8 *LayerMsg,
					  u64 *IdxTree, u32 *IdxLeaf, u8 *LayerSigOut);
static s32 XSlhDsa_HtRecoverBottomNode(XSlhDsa_Instance *InstPtr, const u8 *Msg,
					    const u8 *SignHt, u64 IdxTree, u64 IdxLeaf,
					    u8 *NodeOut);
static s32 XSlhDsa_HtClimbUpperLayers(XSlhDsa_Instance *InstPtr,
					   const XSlhDsa_Param *ParamPtr, const u8 **SignHt,
					   u64 *IdxTree, u8 *NodeInOut);
static s32 XSlhDsa_HtCheckIndices(const XSlhDsa_Instance *InstPtr,
			const XSlhDsa_Param *ParamPtr, u64 IdxTree, u64 IdxLeaf,
			const u8 *SignHt);
static s32 XSlhDsa_HtRootMatchesPk(const u8 *RecoveredRoot, const u8 *PkRoot, u32 HashWidth);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
* @brief	Expand a byte string into an array of b-bit unsigned integers (FIPS base_2b).
*
* @param	Input		Input byte string to convert.
* @param	DigitWidth	Width in bits of each output element (must be 1..25).
* @param	OutLen		Number of b-bit output elements to produce.
* @param	DigitsOut	Output array of b-bit values; must hold OutLen elements.
*
* @return
*		- XASUFW_SUCCESS, if the expansion succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if Input or DigitsOut is NULL, DigitWidth is zero
*		  or above 25, or OutLen is zero or does not fit the shared digit buffer every
*		  caller draws DigitsOut from.
*
* @note
*	DigitWidth must be <= 25, because the accumulator holds up to DigitWidth + 7 bits in a
*	u32 before a digit is extracted. Consumes Input bytes in big-endian order.
*
*************************************************************************************************/
static s32 XSlhDsa_Base2b(const u8 *Input, u32 DigitWidth, u32 OutLen, u32 *DigitsOut)
{
	s32 Status = XASUFW_SLHDSA_INVALID_PARAM;
	u32 InIdx = 0;
	u32 Bits = 0;
	u32 Total = 0;
	u32 OutIdx;
	u32 Mask;

	/** Reject NULL pointers and a width the accumulator cannot hold. */
	if ((Input == NULL) || (DigitsOut == NULL) ||
			(DigitWidth == 0U) || (DigitWidth > XSLHDSA_MAX_BASE2B_DIGIT_WIDTH)) {
		goto END;
	}
	Status = XSlhDsa_CheckIndexBufferFits(OutLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Mask = XSLHDSA_BIT_MASK(DigitWidth);

	/** - Step 4: for out from 0 to out_len - 1 do */
	for (OutIdx = 0; OutIdx < OutLen; OutIdx++) {
		/**
		 * - Step 5: while bits < b do
		 * - Step 6: total <- (total << 8) + X[in]
		 * - Step 7/8: in <- in + 1 and bits <- bits + 8
		 *
		 * Buffer whole bytes big-endian until DigitWidth bits are available.
		 */
		while (Bits < DigitWidth) {
			Total = (Total << XASUFW_ONE_BYTE_SHIFT_VALUE) | Input[InIdx++];
			Bits += XASUFW_BYTE_LEN_IN_BITS;
		}
		/**
		 * - Step 10: bits <- bits - b
		 * - Step 11: baseb[out] <- (total >> bits) mod 2^b
		 *
		 * Stale high bits are discarded by Mask.
		 */
		Bits -= DigitWidth;
		DigitsOut[OutIdx] = (Total >> Bits) & Mask;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Validate that a count of index slots fits the shared FORS/WOTS+ digit buffer.
*
* @param	Count	Number of index slots requested (fors_count or wots_len).
*
* @return
*		- XASUFW_SUCCESS, if Count is non-zero and fits the shared digit buffer.
*		- XASUFW_SLHDSA_INVALID_PARAM, otherwise.
*
* @note
*	Zero is rejected too. No FORS or WOTS+ geometry uses zero index slots, so a zero here
*	is a corrupted parameter set that would otherwise walk no trees or chains at all and
*	still report success.
*
*************************************************************************************************/
static s32 XSlhDsa_CheckIndexBufferFits(u32 Count)
{
	s32 Status = XASUFW_SLHDSA_INVALID_PARAM;

	if ((Count != 0U) && (Count <= XSLHDSA_WOTS_WEIGHT_WORDS)) {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}
/*************************************************************************************************/
/**
* @brief	Validate that Count hash-width elements fit the shared FORS/WOTS+ scratch area.
*
* @param	Count	Number of n-byte elements to be collected (fors_count or wots_len).
* @param	HashWidth	Hash width n in bytes.
*
* @return
*		- XASUFW_SUCCESS, if Count * HashWidth fits the shared scratch area.
*		- XASUFW_SLHDSA_INVALID_PARAM, otherwise.
*
*************************************************************************************************/
static s32 XSlhDsa_CheckScratchFits(u32 Count, u32 HashWidth)
{
	s32 Status = XASUFW_SLHDSA_INVALID_PARAM;

	if (((u64)Count * (u64)HashWidth) <= (u64)XSLHDSA_WORK_MEM_BYTES) {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}
/*************************************************************************************************/
/**
* @brief	Validate that the FORS tree indices fit the shared digit buffer and that the
*		digest bits they are drawn from fit the digest buffer
*		(fors_sign/fors_pkFromSig Step 1/2 precondition).
*
* @param	ParamPtr	Active parameter set.
*
* @return
*		- XASUFW_SUCCESS, if fors_count fits the shared digit buffer and the k * a
*		  digest bits base_2b splits off fit the digest buffer.
*		- XASUFW_SLHDSA_INVALID_PARAM, otherwise.
*
*************************************************************************************************/
static s32 XSlhDsa_ForsCheckDigestFits(const XSlhDsa_Param *ParamPtr)
{
	s32 Status = XASUFW_SLHDSA_INVALID_PARAM;
	u32 DigestBits;

	/**
	 * Bound fors_height first: it is the width every shift and multiply below assumes,
	 * so an oversized value here would make them overflow rather than reject cleanly.
	 */
	if ((ParamPtr->fors_height == 0U) || (ParamPtr->fors_height > XSLHDSA_MAX_FORS_HEIGHT)) {
		goto END;
	}

	/** One index slot per tree must fit the digit buffer FORS and WOTS+ both share. */
	Status = XSlhDsa_CheckIndexBufferFits(ParamPtr->fors_count);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * fors_count is now bounded to XSLHDSA_WOTS_WEIGHT_WORDS and fors_height to
	 * XSLHDSA_MAX_FORS_HEIGHT, so this product cannot overflow.
	 */
	DigestBits = ParamPtr->fors_count * ParamPtr->fors_height;
	if (((DigestBits + (XASUFW_BYTE_LEN_IN_BITS - 1U)) / XASUFW_BYTE_LEN_IN_BITS) >
			XSLHDSA_MSG_DIGEST_BYTES) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Emit the FORS portion of an SLH-DSA signature (FIPS 205 Algorithm 16: fors_sign).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	Md	Message digest buffer used to derive FORS indices.
* @param	Dst	Output buffer for FORS signature data.
*
* @return
*		- XASUFW_SUCCESS, if FORS signature emission succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if a required pointer is NULL, fors_count does not
*		  fit the shared digit buffer, the k * a digest bits do not fit the digest
*		  buffer, a flat leaf slot runs outside the FORS leaf space, or the walk stops
*		  before every tree is signed.
*		- XASUFW_CMD_IN_PROGRESS, if a cooperative yield slice expired and the walk will
*		  resume.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
s32 XSlhDsa_ForsEmitSig(XSlhDsa_Instance *InstPtr, const u8 *Md, u8 *Dst)
{
	s32 Status = XASUFW_FAILURE;
	s32 YieldStatus = XASUFW_FAILURE;
	u32 TreeStart = 0U;
	u32 LevelStart = 0U;
	u8 ResumedTree = 0U;
	const XSlhDsa_Param *ParamPtr;
	XSlhDsa_YieldCtx *YieldPtr;
	u32 HashWidth;
	u32 TreeIdx;
	u32 Pos;
	u32 LevelIdx = 0U;
	u32 SibBase;

	/** Validate input pointers, including the shared digit buffer used for the indices. */
	if ((InstPtr == NULL) || (InstPtr->Addr == NULL) ||
			(Dst == NULL) || (Md == NULL) || (InstPtr->WotsWeights == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	ParamPtr = &InstPtr->Param;
	HashWidth = ParamPtr->n;
	YieldPtr = &InstPtr->Yield;

	/**
	 * Reject a fors_count that does not fit the shared digit buffer and a k * a digest
	 * bit span that does not fit the digest buffer, before the digest is split.
	 */
	Status = XSlhDsa_ForsCheckDigestFits(ParamPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if ((YieldPtr->Fors.EmitResume != 0U) && (YieldPtr->Fors.EmitSigBase == Dst)) {
		TreeStart = YieldPtr->Fors.EmitTreeIdx;
		LevelStart = YieldPtr->Fors.EmitLevelIdx;
		Dst = YieldPtr->Fors.EmitDst;
		ResumedTree = 1U;
	} else {
		YieldPtr->Fors.EmitResume = 0U;
		YieldPtr->Fors.TreeResume = 0U;
		YieldPtr->Fors.EmitSigBase = Dst;
	}

	/**
	 * - Step 2: indices <- base_2b(md, a, k)
	 *
	 * Recompute on every slice so the indices do not depend on WotsWeights surviving a yield.
	 */
	Status = XSlhDsa_Base2b(Md, ParamPtr->fors_height, ParamPtr->fors_count,
			    InstPtr->WotsWeights);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Step 3: for i from 0 to k - 1 do
	 *
	 * The signature is k blocks of (1 + a) * n bytes: a revealed leaf secret followed by
	 * that tree's a authentication nodes.
	 */
	for (TreeIdx = TreeStart; TreeIdx < ParamPtr->fors_count; TreeIdx++) {
		Pos = InstPtr->WotsWeights[TreeIdx];

		/**
		 * Re-verify this tree's leaf number fits the a bits the flat slot reserves for
		 * it. base_2b already masked it to that width, but this is the point where the
		 * number is shifted into the tree's own slot and keyed into ADRS, so a wider
		 * value here would address a leaf belonging to a neighbouring tree.
		 */
		if (Pos > XSLHDSA_BIT_MASK(ParamPtr->fors_height)) {
			Status = XASUFW_SLHDSA_INVALID_PARAM;
			goto END;
		}

		if (TreeIdx != TreeStart) {
			LevelStart = 0U;
		}

		if ((TreeIdx != TreeStart) || (ResumedTree == 0U)) {
			/**
			 * - ADRS: index = i * 2^a + indices[i]; FORS leaves share one
			 *   flat space.
			 */
			XSlhDsa_AddrSetIndex(InstPtr->Addr,
					    (TreeIdx << ParamPtr->fors_height) + Pos);
			/**
			 * - Step 4: SIG_FORS <- SIG_FORS || fors_skGen(SK.seed, PK.seed, ADRS,
			 *   i * 2^a + indices[i])
			 *
			 * Emit the leaf secret itself; F is skipped so the verifier can apply it.
			 */
			Status = XSlhDsa_ForsLeafHash(InstPtr, 0U, Dst);
			if (Status != XASUFW_SUCCESS) {
				goto END;
			}
			Dst += HashWidth;
		}

		/** - Step 5: for j from 0 to a - 1 do */
		for (LevelIdx = LevelStart; LevelIdx < ParamPtr->fors_height; LevelIdx++) {
			/**
			 * - Step 6: s <- floor(indices[i] / 2^j) XOR 1
			 *
			 * Flip bit LevelIdx of Pos and re-attach the tree number with
			 * (a - LevelIdx) bits, as indices narrow a bit per level.
			 */
			SibBase = (TreeIdx << (ParamPtr->fors_height - LevelIdx)) +
				((Pos >> LevelIdx) ^ 1U);

			YieldStatus = XSlhDsa_YieldPoll(InstPtr,
							XSLHDSA_YIELD_UNIT_FORS_LEAF);
			if (YieldStatus == XASUFW_CMD_IN_PROGRESS) {
				YieldPtr->Fors.EmitResume = 1U;
				YieldPtr->Fors.EmitTreeIdx = TreeIdx;
				YieldPtr->Fors.EmitLevelIdx = LevelIdx;
				YieldPtr->Fors.EmitDst = Dst;
				Status = YieldStatus;
				goto END;
			}
			if (YieldStatus != XASUFW_SUCCESS) {
				Status = YieldStatus;
				goto END;
			}

			/**
			 * - Step 7: AUTH[j] <- fors_node(SK.seed, i * 2^(a-j) + s, j, PK.seed,
			 *   ADRS)
			 */
			Status = XSlhDsa_ForsSubtreeRoot(InstPtr, SibBase, LevelIdx, Dst);
			if (Status == XASUFW_CMD_IN_PROGRESS) {
				YieldPtr->Fors.EmitResume = 1U;
				YieldPtr->Fors.EmitTreeIdx = TreeIdx;
				YieldPtr->Fors.EmitLevelIdx = LevelIdx;
				YieldPtr->Fors.EmitDst = Dst;
				goto END;
			}
			if (Status != XASUFW_SUCCESS) {
				goto END;
			}
			Dst += HashWidth;
		}
	}

	/**
	 * Confirm the walk actually reached the last tree and the last level of it: a
	 * short-circuited loop would reach here too, leaving the remaining output blocks
	 * holding whatever Dst held before.
	 */
	if ((TreeIdx != ParamPtr->fors_count) || (LevelIdx != ParamPtr->fors_height)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	YieldPtr->Fors.EmitResume = 0U;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Reconstruct the FORS public key from a FORS signature and digest
*		(FIPS 205 Algorithm 17: fors_pkFromSig).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	SignFors	Pointer to FORS signature (k * (1 + a) * n bytes).
* @param	Md	Pointer to FORS digest prefix (k * a bits, stored in bytes).
* @param	PkFors	Output buffer for recovered FORS public key (n bytes).
*
* @return
*		- XASUFW_SUCCESS, if FORS public key recovery succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if any required pointer is NULL, fors_count does
*		  not fit the shared digit buffer, the k * a digest bits do not fit the digest
*		  buffer, the k recovered roots do not fit the shared scratch area, or the walk
*		  stops before every tree is recovered.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
*	InstPtr->WorkMem must provide at least k * n bytes of scratch space and
*	InstPtr->WotsWeights must hold at least k u32 elements.
*
*************************************************************************************************/
s32 XSlhDsa_ForsPkFromSig(XSlhDsa_Instance *InstPtr, const u8 *SignFors,
			       const u8 *Md, u8 *PkFors)
{
	s32 Status = XASUFW_FAILURE;
	const XSlhDsa_Param *ParamPtr;
	const u8 *SignPtr;
	u8 *NodePtr;
	u32 HashWidth;
	u32 TreeIdx;
	u32 NodeIdx = 0U;
	u32 Idx;

	/** Validate input pointers, including the root scratch and digit buffers. */
	if ((InstPtr == NULL) || (InstPtr->Addr == NULL) ||
			(SignFors == NULL) || (Md == NULL) || (PkFors == NULL) ||
			(InstPtr->WorkMem == NULL) || (InstPtr->WotsWeights == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	ParamPtr = &InstPtr->Param;
	HashWidth = ParamPtr->n;
	NodePtr = InstPtr->WorkMem;
	SignPtr = SignFors;

	/**
	 * Reject a fors_count that does not fit the shared digit buffer and a k * a digest
	 * bit span that does not fit the digest buffer, before the digest is split.
	 */
	Status = XSlhDsa_ForsCheckDigestFits(ParamPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Reject k recovered roots that would not fit the shared scratch area. */
	Status = XSlhDsa_CheckScratchFits(ParamPtr->fors_count, HashWidth);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Step 1: indices <- base_2b(md, a, k)
	 *
	 * Recover the same k indices of a bits each that the signer derived from the digest.
	 */
	Status = XSlhDsa_Base2b(Md, ParamPtr->fors_height, ParamPtr->fors_count,
			       InstPtr->WotsWeights);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** - Step 2: for i from 0 to k - 1 do */
	for (TreeIdx = 0; TreeIdx < ParamPtr->fors_count; TreeIdx++) {
		/**
		 * - Step 4: ADRS.setTreeHeight(0)
		 * - Step 5: ADRS.setTreeIndex(i * 2^a + indices[i])
		 *
		 * Type FORS_TREE is set by the caller. The flat slot must match the signer's,
		 * or the recovered root will not match.
		 */
		XSlhDsa_AddrSetHeight(InstPtr->Addr, 0);
		Idx = (TreeIdx << ParamPtr->fors_height) + InstPtr->WotsWeights[TreeIdx];
		XSlhDsa_AddrSetIndex(InstPtr->Addr, Idx);

		/**
		 * - Step 3: sk <- SIG_FORS.getSK(i)
		 * - Step 6: node[0] <- F(PK.seed, ADRS, sk)
		 */
		Status = XSlhDsa_HashF(InstPtr, SignPtr, NodePtr);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		SignPtr += HashWidth;

		/**
		 * - Step 7: auth <- SIG_FORS.getAUTH(i)
		 * - Step 8: for j from 0 to a - 1 do
		 */
		for (NodeIdx = 0; NodeIdx < ParamPtr->fors_height; NodeIdx++) {
			/**
			 * - Step 9: ADRS.setTreeHeight(j + 1)
			 * - Step 11/14: ADRS.setTreeIndex() to the parent position
			 */
			XSlhDsa_AddrSetHeight(InstPtr->Addr, NodeIdx + 1U);
			XSlhDsa_AddrSetIndex(InstPtr->Addr, Idx >> (NodeIdx + 1U));

			/**
			 * - Step 10: if floor(indices[i] / 2^j) is even then
			 * - Step 12: node[1] <- H(PK.seed, ADRS, node[0] || auth[j])
			 * - Step 15: node[1] <- H(PK.seed, ADRS, auth[j] || node[0])
			 * - Step 17: node[0] <- node[1]
			 *
			 * Bit NodeIdx of the leaf index picks the side: clear means the running
			 * node is the left child. The parent overwrites it in place.
			 */
			if (((InstPtr->WotsWeights[TreeIdx] >> NodeIdx) & 1U) == 0U) {
				Status = XSlhDsa_HashH(InstPtr, NodePtr, SignPtr, NodePtr);
			}
			else {
				Status = XSlhDsa_HashH(InstPtr, SignPtr, NodePtr, NodePtr);
			}
			if (Status != XASUFW_SUCCESS) {
				goto END;
			}
			SignPtr += HashWidth;
		}

		/** - Step 19: root[i] <- node[0], kept in WorkMem for the final compression. */
		NodePtr += HashWidth;
	}

	/**
	 * Confirm the walk actually recovered every tree and folded every level of each: a
	 * short-circuited loop would reach here too, leaving the compression to run over
	 * stale roots from an earlier request.
	 */
	if ((TreeIdx != ParamPtr->fors_count) || (NodeIdx != ParamPtr->fors_height)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * - Step 21 to 23: forspkADRS <- ADRS, setTypeAndClear(FORS_ROOTS) and
	 *   setKeyPairAddress(ADRS.getKeyPairAddress())
	 * - Step 24: pk <- T_k(PK.seed, forspkADRS, root)
	 *
	 * H_TL runs over the k * n bytes of recovered tree roots held in WorkMem.
	 */
	XSlhDsa_AddrSetTypeClearNotKp(InstPtr->Addr, XSLHDSA_ADRS_FORS_ROOTS);
	Status = XSlhDsa_HashTl(InstPtr, InstPtr->WorkMem, ParamPtr->fors_count * HashWidth,
			       PkFors);

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Run one WOTS+ hash chain from index zero (FIPS 205 Algorithm 5: chain).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	Steps	Number of chain steps to apply from index zero.
* @param	Out	Output buffer for the chain endpoint (n bytes).
*
* @return
*		- XASUFW_SUCCESS, if chain execution succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if a required pointer is NULL, wots_win_bits is
*		  zero or wider than a word, or Steps would walk past the chain top.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
s32 XSlhDsa_WotsChainFromZero(XSlhDsa_Instance *InstPtr, u32 Steps, u8 *Out)
{
	s32 Status = XASUFW_FAILURE;
	XSlhDsa_ChainCfg ChainCfg;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (InstPtr->Addr == NULL) || (Out == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** chain(sk, 0, Steps): the walk starts at the bottom of the window. */
	ChainCfg.StartIdx = 0;
	ChainCfg.Steps = Steps;

	/**
	 * Bound the step range against the chain top before the secret is derived, so an
	 * out-of-range digit cannot drive F past it. XSlhDsa_Chain applies the same check
	 * once the secret is in place.
	 */
	Status = XSlhDsa_WotsCheckChainRange(&InstPtr->Param, &ChainCfg);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - wots_sign Step 13: sk <- PRF(PK.seed, SK.seed, skADRS)
	 *
	 * ADRS: type = WOTS_NONCE, chain address and hash index = 0 for PRF. PRF regenerates
	 * the secret anchoring the chain already selected by the caller's chain address, and
	 * the nonce type keeps that derivation separate from the F steps below.
	 */
	XSlhDsa_AddrSetType(InstPtr->Addr, XSLHDSA_ADRS_WOTS_NONCE);
	XSlhDsa_AddrSetIndex(InstPtr->Addr, 0);
	Status = XSlhDsa_HashPrf(InstPtr, Out);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Step 1: tmp <- X
	 * - Step 2: for j from i to i + s - 1 do
	 * - Step 3: ADRS.setHashAddress(j)
	 * - Step 4: tmp <- F(PK.seed, ADRS, tmp)
	 *
	 * ADRS: type = WOTS_HASH for F chain steps from index 0. chain(sk, 0, Steps) applies F
	 * with the hash slot running 0..Steps-1, so Steps of 0 leaves the secret untouched and
	 * (2^lg_w - 1) reaches the chain top. Out is source and destination, so F runs in place.
	 */
	XSlhDsa_AddrSetType(InstPtr->Addr, XSLHDSA_ADRS_WOTS_HASH);
	XSlhDsa_AddrSetIndex(InstPtr->Addr, 0);
	Status = XSlhDsa_Chain(InstPtr, Out, &ChainCfg, Out);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Emit the WOTS+ portion of an XMSS signature (FIPS 205 Algorithm 7: wots_sign).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	Msg	Message buffer to sign (n bytes).
* @param	Dst	Output buffer for WOTS+ signature (wots_len * n bytes).
*
* @return
*		- XASUFW_SUCCESS, if WOTS+ signature emission succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if a required pointer is NULL, wots_len does
*		  not fit the shared digit buffer, or len_1 and len_2 do not split it.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_ZEROIZE_MEMSET_FAIL,
*		  XASUFW_SLHDSA_COPY_FAILED, XASUFW_SHA_START_FAILED,
*		  XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
s32 XSlhDsa_WotsEmitSig(XSlhDsa_Instance *InstPtr, const u8 *Msg, u8 *Dst)
{
	s32 Status = XASUFW_FAILURE;
	const XSlhDsa_Param *ParamPtr;

	/** Validate input pointers, including the shared digit buffer used for the indices. */
	if ((InstPtr == NULL) || (InstPtr->Addr == NULL) ||
			(Dst == NULL) || (Msg == NULL) || (InstPtr->WotsWeights == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	ParamPtr = &InstPtr->Param;

	/** Reject a wots_len that does not fit the shared digit buffer, before it is used. */
	Status = XSlhDsa_CheckIndexBufferFits(ParamPtr->wots_len);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Step 1: csum <- 0
	 * - Step 2: msg <- base_2b(M, lg_w, len_1)
	 * - Step 3 to 5: csum <- csum + w - 1 - msg[i], for i from 0 to len_1 - 1
	 * - Step 6: csum <- csum << ((8 - ((len_2 * lg_w) mod 8)) mod 8)
	 * - Step 7: msg <- msg || base_2b(toByte(csum, ceil(len_2*lg_w/8)), lg_w, len_2)
	 *
	 * Derive the wots_len chain stop indices: len_1 base-2^lg_w message digits followed by
	 * len_2 checksum digits, staged in the instance digit buffer.
	 */
	Status = XSlhDsa_WotsDeriveChainIndices(Msg, ParamPtr, InstPtr->WotsWeights);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Step 8 to 16: for i from 0 to len - 1, sig[i] <- chain(sk, 0, msg[i], PK.seed,
	 *   ADRS), where sk <- PRF(PK.seed, SK.seed, skADRS) for chain i.
	 */
	Status = XSlhDsa_WotsWriteSignChains(InstPtr, InstPtr->WotsWeights, ParamPtr, Dst);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Recover a WOTS+ public key from a WOTS+ signature and message digest
*		(FIPS 205 Algorithm 8: wots_pkFromSig).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	WotsSign	Pointer to WOTS+ signature (wots_len * n bytes).
* @param	Msg	Pointer to message digest used for chain index derivation.
* @param	WotsPk	Output buffer for recovered WOTS+ public key (n bytes).
*
* @return
*		- XASUFW_SUCCESS, if WOTS+ public key recovery succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if any required pointer is NULL, wots_len is
*		  zero or does not fit the shared digit buffer or the scratch area the chain
*		  tops are collected in, or len_1 and len_2 do not split it.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_ZEROIZE_MEMSET_FAIL,
*		  XASUFW_SLHDSA_COPY_FAILED, XASUFW_SHA_START_FAILED,
*		  XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
s32 XSlhDsa_WotsPkFromSign(XSlhDsa_Instance *InstPtr, const u8 *WotsSign,
				const u8 *Msg, u8 *WotsPk)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XSlhDsa_Param *ParamPtr;
	u32 HashWidth;
	u32 TotalChains;

	/** Validate input pointers, including the scratch and digit buffers on the instance. */
	if ((InstPtr == NULL) || (InstPtr->Addr == NULL) ||
			(WotsSign == NULL) || (Msg == NULL) || (WotsPk == NULL) ||
			(InstPtr->WorkMem == NULL) || (InstPtr->WotsWeights == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	ParamPtr = &InstPtr->Param;
	HashWidth = ParamPtr->n;
	TotalChains = ParamPtr->wots_len;

	/**
	 * Reject a wots_len that would not fit the shared digit buffer the decode below fills,
	 * or the scratch area that holds the recovered chain tops.
	 */
	Status = XSlhDsa_CheckIndexBufferFits(TotalChains);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Status = XSlhDsa_CheckScratchFits(TotalChains, HashWidth);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Step 1 to 7: same as wots_sign; msg <- base_2b(M, lg_w, len_1) followed by the
	 *   len_2 checksum digits.
	 *
	 * Re-derive the same chain indices the signer used, from the same message digest.
	 */
	Status = XSlhDsa_WotsDeriveChainIndices(Msg, ParamPtr, InstPtr->WotsWeights);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Step 8 to 11: for i from 0 to len - 1, ADRS.setChainAddress(i) and
	 *   tmp[i] <- chain(sig[i], msg[i], w - 1 - msg[i], PK.seed, ADRS)
	 *
	 * Walk each signature element to its chain top, collected contiguously in WorkMem.
	 */
	Status = XSlhDsa_WotsRebuildChainTops(InstPtr, WotsSign, InstPtr->WotsWeights,
					ParamPtr, InstPtr->WorkMem);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Step 12 to 14: wotspkADRS <- ADRS, setTypeAndClear(WOTS_PK), setKeyPairAddress()
	 * - Step 15: pk_sig <- T_len(PK.seed, wotspkADRS, tmp)
	 *
	 * ADRS: type = WOTS_PK for H_TL over len chain tops. T_len compresses the len * n
	 * chain-top bytes into the public key; the kept key-pair field binds it to this leaf.
	 * The status is re-armed to a failure value before the compression.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	XSlhDsa_AddrSetTypeClearNotKp(InstPtr->Addr, XSLHDSA_ADRS_WOTS_PK);
	Status = XSlhDsa_HashTl(InstPtr, InstPtr->WorkMem, TotalChains * HashWidth, WotsPk);

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Compute an XMSS subtree root (FIPS 205 xmss_node over 2^SubHeight leaves).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	FirstLeaf	First leaf index in the subtree.
* @param	SubHeight	Sub-tree height.
* @param	Root	Output root buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS, if subtree root computation succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if a required pointer is NULL, SubHeight exceeds
*		  the stack depth, the bound subtree height is zero or above
*		  XSLHDSA_MAX_XMSS_HEIGHT, the 2^SubHeight leaves starting at FirstLeaf run
*		  outside the bound XMSS tree, a restored checkpoint does not match the walk it
*		  resumes, or the node stack has not fully drained when the walk reports success.
*		- XASUFW_CMD_IN_PROGRESS, if a cooperative yield slice expired and the walk will
*		  resume.
*		- XASUFW_SLHDSA_COPY_FAILED, if restoring or saving the treehash node stack fails.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
s32 XSlhDsa_XmssSubtreeRoot(XSlhDsa_Instance *InstPtr, u32 FirstLeaf,
				 u32 SubHeight, u8 *Root)
{
	s32 Status = XASUFW_FAILURE;
	s32 YieldStatus = XASUFW_FAILURE;
	s32 StackDepth = -1;
	const XSlhDsa_Param *ParamPtr;
	XSlhDsa_YieldCtx *YieldPtr;
	u8 NodeStack[XSLHDSA_XMSS_STACK_DEPTH][XSLHDSA_MAX_N];
	u8 *ChainScratch;
	u32 LeafIdx;
	u32 LeafSeq;
	u32 LeafCount;

	/** Validate input pointers, including the WOTS+ chain scratch buffer. */
	if ((InstPtr == NULL) || (InstPtr->Addr == NULL) ||
			(Root == NULL) || (InstPtr->WorkMem == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** A walk of height SubHeight keeps that many nodes pending, so it must fit the stack. */
	if (SubHeight >= XSLHDSA_XMSS_STACK_DEPTH) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	ParamPtr = &InstPtr->Param;
	YieldPtr = &InstPtr->Yield;
	ChainScratch = InstPtr->WorkMem;
	/** The sub-tree spans the 2^SubHeight consecutive leaves starting at FirstLeaf. */
	LeafCount = (1U << SubHeight);

	/**
	 * Bound xmss_height before the shift below, and reject the zero-height reserved row.
	 * The leaf range itself is checked next.
	 */
	Status = XSlhDsa_XmssCheckLeaf(ParamPtr, FirstLeaf);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * The sub-tree must fit the bound tree, and its leaves must end inside it. The limit is
	 * formed by subtraction rather than FirstLeaf + LeafCount, which could wrap and let a
	 * start index near the top of the range pass.
	 */
	if ((SubHeight > ParamPtr->xmss_height) ||
			(FirstLeaf > ((1U << ParamPtr->xmss_height) - LeafCount))) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	if ((YieldPtr->Xmss.Resume != 0U) && (YieldPtr->Xmss.FirstLeaf == FirstLeaf) &&
			(YieldPtr->Xmss.SubHeight == SubHeight) &&
			(YieldPtr->Xmss.RootOut == Root)) {
		LeafSeq = YieldPtr->Xmss.LeafSeq;
		LeafIdx = YieldPtr->Xmss.LeafIdx;
		StackDepth = YieldPtr->Xmss.StackDepth;
		/**
		 * The merge pops from NodeStack at the restored depth, so a depth that does
		 * not match the restored position would step off either end of the array.
		 */
		if ((StackDepth < -1) || (StackDepth > (s32)SubHeight) ||
				(LeafSeq >= LeafCount)) {
			Status = XASUFW_SLHDSA_INVALID_PARAM;
			goto END;
		}
		Status = Xil_SMemCpy(NodeStack, sizeof(NodeStack), YieldPtr->Xmss.NodeStack,
				     sizeof(YieldPtr->Xmss.NodeStack),
				     sizeof(YieldPtr->Xmss.NodeStack));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_SLHDSA_COPY_FAILED;
			goto END;
		}
	} else {
		LeafSeq = 0U;
		LeafIdx = FirstLeaf;
		StackDepth = -1;
		YieldPtr->Xmss.Resume = 0U;
	}

	for (; LeafSeq < LeafCount; LeafSeq++) {
		YieldStatus = XSlhDsa_YieldPollLeaf(InstPtr, XSLHDSA_YIELD_UNIT_XMSS_LEAF);
		if (YieldStatus == XASUFW_CMD_IN_PROGRESS) {
			Status = Xil_SMemCpy(YieldPtr->Xmss.NodeStack,
					     sizeof(YieldPtr->Xmss.NodeStack), NodeStack,
					     sizeof(NodeStack), sizeof(NodeStack));
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_SLHDSA_COPY_FAILED;
				goto END;
			}
			YieldPtr->Xmss.Resume = 1U;
			YieldPtr->Xmss.FirstLeaf = FirstLeaf;
			YieldPtr->Xmss.SubHeight = SubHeight;
			YieldPtr->Xmss.RootOut = Root;
			YieldPtr->Xmss.LeafSeq = LeafSeq;
			YieldPtr->Xmss.LeafIdx = LeafIdx;
			YieldPtr->Xmss.StackDepth = StackDepth;
			Status = YieldStatus;
			goto END;
		}
		if (YieldStatus != XASUFW_SUCCESS) {
			Status = YieldStatus;
			goto END;
		}

		/** - Build this leaf and merge upward; the last leaf leaves the root in Root. */
		Status = XSlhDsa_XmssIntegrateLeaf(InstPtr, LeafIdx, LeafSeq, ParamPtr,
							NodeStack, &StackDepth, Root, ChainScratch);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		LeafIdx++;
	}

	/**
	 * A fully merged sub-tree always drains back to depth 0, with the root left in Root.
	 * Reject a walk that reports success with nodes still pending, which only an
	 * early-terminated loop could leave behind.
	 */
	if (StackDepth != 0) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	YieldPtr->Xmss.Resume = 0U;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Emit one XMSS signature bundle for a leaf slot (FIPS 205 Algorithm 10: xmss_sign).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	Msg	Message buffer to sign (n bytes).
* @param	IdxLeaf	Leaf index within the XMSS tree.
* @param	LayerSign	Output buffer for WOTS+ sig and auth path.
*
* @return
*		- XASUFW_SUCCESS, if XMSS signature emission succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if a required pointer is NULL or IdxLeaf is
*		  outside the bound subtree.
*		- XASUFW_CMD_IN_PROGRESS, if a cooperative yield slice expired and the walk will
*		  resume.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_ZEROIZE_MEMSET_FAIL,
*		  XASUFW_SLHDSA_COPY_FAILED, XASUFW_SHA_START_FAILED,
*		  XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
s32 XSlhDsa_XmssEmitSig(XSlhDsa_Instance *InstPtr, const u8 *Msg,
			     u32 IdxLeaf, u8 *LayerSign)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 ResumeAuthPath = 0U;
	const XSlhDsa_Param *ParamPtr;
	XSlhDsa_YieldCtx *YieldPtr;
	u32 WotsOctets;
	u8 *AuthPathPtr;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (InstPtr->Addr == NULL) ||
			(LayerSign == NULL) || (Msg == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	ParamPtr = &InstPtr->Param;

	Status = XSlhDsa_XmssCheckLeaf(ParamPtr, IdxLeaf);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	YieldPtr = &InstPtr->Yield;
	/**
	 * - Step 8: SIG_XMSS <- sig || AUTH
	 *
	 * A layer signature is SIG_WOTS || AUTH, so the auth path begins len * n bytes in.
	 */
	WotsOctets = ParamPtr->wots_len * ParamPtr->n;
	AuthPathPtr = LayerSign + WotsOctets;

	if ((YieldPtr->Xmss.EmitResume != 0U) &&
	    (YieldPtr->Xmss.EmitLayerSign == LayerSign) &&
	    (YieldPtr->Xmss.EmitIdxLeaf == IdxLeaf) &&
	    (YieldPtr->Xmss.EmitWotsDone != 0U)) {
		ResumeAuthPath = 1U;
	}

	if (ResumeAuthPath == 0U) {
		/**
		 * A fresh emit must not inherit a checkpoint from an abandoned walk: the
		 * treehash tag is only a leaf range, so a stale one can match another subtree.
		 */
		YieldPtr->Xmss.AuthPathResume = 0U;
		YieldPtr->Xmss.Resume = 0U;
		/**
		 * - Step 5: ADRS.setTypeAndClear(WOTS_HASH)
		 * - Step 6: ADRS.setKeyPairAddress(idx)
		 * - Step 7: sig <- wots_sign(M, SK.seed, PK.seed, ADRS)
		 */
		Status = XSlhDsa_XmssWriteWotsSignature(InstPtr, Msg, IdxLeaf, LayerSign);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		YieldPtr->Xmss.EmitLayerSign = LayerSign;
		YieldPtr->Xmss.EmitIdxLeaf = IdxLeaf;
		YieldPtr->Xmss.EmitWotsDone = 1U;
	}

	/** Re-arm the status to a failure value before the authentication path is built. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSlhDsa_XmssWriteAuthPathNodes(InstPtr, IdxLeaf, ParamPtr, AuthPathPtr);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		YieldPtr->Xmss.EmitResume = 1U;
		goto END;
	}
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	YieldPtr->Xmss.EmitResume = 0U;
	YieldPtr->Xmss.EmitWotsDone = 0U;
	YieldPtr->Xmss.AuthPathResume = 0U;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Recover an XMSS tree node from a WOTS+ signature and authentication path
*		(FIPS 205 Algorithm 11: xmss_pkFromSig).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	IdxLeaf	Leaf index within the XMSS subtree.
* @param	SignXmss	Pointer to XMSS signature (WOTS+ sig plus auth path).
* @param	Msg	Pointer to message digest passed to WOTS+ recovery.
* @param	Node	Output buffer for recovered XMSS node (n bytes).
*
* @return
*		- XASUFW_SUCCESS, if XMSS node recovery succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if any required pointer is NULL or IdxLeaf is
*		  outside the bound subtree.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_ZEROIZE_MEMSET_FAIL,
*		  XASUFW_SLHDSA_COPY_FAILED, XASUFW_SHA_START_FAILED,
*		  XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
s32 XSlhDsa_XmssPkFromSign(XSlhDsa_Instance *InstPtr, u32 IdxLeaf,
				const u8 *SignXmss, const u8 *Msg, u8 *Node)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XSlhDsa_Param *ParamPtr;
	const u8 *AuthPathPtr;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (InstPtr->Addr == NULL) ||
			(SignXmss == NULL) || (Msg == NULL) || (Node == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	ParamPtr = &InstPtr->Param;

	Status = XSlhDsa_XmssCheckLeaf(ParamPtr, IdxLeaf);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Step 3: sig <- SIG_XMSS.getWOTSSig(), SIG_XMSS[0 : len * n]
	 * - Step 4: AUTH <- SIG_XMSS.getXMSSAUTH(), SIG_XMSS[len * n : (len + h') * n]
	 */
	AuthPathPtr = SignXmss + (ParamPtr->wots_len * ParamPtr->n);

	/**
	 * - Step 1: ADRS.setTypeAndClear(WOTS_HASH)
	 * - Step 2: ADRS.setKeyPairAddress(idx)
	 * - Step 5: node[0] <- wots_pkFromSig(sig, M, PK.seed, ADRS)
	 *
	 * The recovered leaf node is the WOTS+ public key implied by the signature.
	 */
	Status = XSlhDsa_XmssRebuildLeafPk(InstPtr, SignXmss, Msg, IdxLeaf, Node);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Step 6 to 18: fold the h' auth path siblings in, hashing node[0] with AUTH[k] in
	 *   the order set by the parity of floor(idx / 2^k).
	 *
	 * Node ends holding the sub-tree root. The status is re-armed to a failure value
	 * before the ascent.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSlhDsa_XmssAscendAuthPath(InstPtr, IdxLeaf, AuthPathPtr, ParamPtr, Node);

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Parse hypertree tree and leaf indices from an H_msg digest (FIPS 205 Section 9).
*
* @param	ParamPtr	Pointer to SLH-DSA parameter set.
* @param	Digest	Pointer to H_msg digest buffer.
* @param	HtIndices	Output hypertree index structure.
*
*************************************************************************************************/
void XSlhDsa_HtParseDigest(const XSlhDsa_Param *ParamPtr, const u8 *Digest,
				XSlhDsa_HtIndices *HtIndices)
{
	u32 MdByteLen;
	u32 TreeFieldLen;
	u32 LeafFieldLen;

	/** The digest is laid out as md || tmp_idx_tree || tmp_idx_leaf; measure it first. */
	XSlhDsa_HtMeasureDigestLayout(ParamPtr, &MdByteLen, &TreeFieldLen, &LeafFieldLen);
	/** Tree index selects which XMSS tree of the bottom hypertree layer signs. */
	HtIndices->IdxTree =
			XSlhDsa_HtExtractTreeIndex(Digest, MdByteLen, TreeFieldLen, ParamPtr);
	/** Leaf index selects the one-time key pair inside that tree. */
	HtIndices->IdxLeaf = XSlhDsa_HtExtractLeafIndex(Digest, MdByteLen, TreeFieldLen,
						       LeafFieldLen, ParamPtr);
}
/*************************************************************************************************/
/**
* @brief	Emit hypertree XMSS signatures across all d layers (FIPS 205 Algorithm 12: ht_sign).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	HtIndices	Hypertree tree and leaf indices for the bottom layer.
* @param	LayerMsg	In/out n-byte buffer; FORS PK then successive layer roots.
* @param	SigHt	Output buffer for d XMSS layer signatures.
*
* @return
*		- XASUFW_SUCCESS, if hypertree signing succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if a required pointer is NULL, the tree/leaf index
*		  pair is outside the hypertree structure, or the hypertree signature section
*		  does not fit the registered signature buffer.
*		- XASUFW_CMD_IN_PROGRESS, if a cooperative yield slice expired and the walk will
*		  resume.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
s32 XSlhDsa_HtEmitSig(XSlhDsa_Instance *InstPtr,
			   const XSlhDsa_HtIndices *HtIndices, u8 *LayerMsg,
			   u8 *SigHt)
{
	s32 Status = XASUFW_FAILURE;
	u8 SkipBottom = 0U;
	const XSlhDsa_Param *ParamPtr;
	XSlhDsa_YieldCtx *YieldPtr;
	u64 IdxTree;
	u32 IdxLeaf;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (InstPtr->Addr == NULL) ||
			(SigHt == NULL) || (LayerMsg == NULL) || (HtIndices == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	ParamPtr = &InstPtr->Param;
	YieldPtr = &InstPtr->Yield;
	/** Local copies: the layer loop shifts the tree index and rewrites the leaf slot. */
	IdxTree = HtIndices->IdxTree;
	IdxLeaf = (u32)HtIndices->IdxLeaf;

	if ((YieldPtr->Ht.Resume != 0U) && (YieldPtr->Ht.SigBase == SigHt)) {
		SkipBottom = 1U;
		IdxTree = YieldPtr->Ht.IdxTree;
		IdxLeaf = YieldPtr->Ht.IdxLeaf;
	}

	if (SkipBottom == 0U) {
		/**
		 * Reject a tree/leaf pair outside the hypertree structure and a hypertree
		 * section that would run past the registered signature buffer, before the
		 * bottom layer is signed; reused unchanged from XSlhDsa_HtVerify's
		 * precondition, since SigHt resolves into the same registered signature
		 * buffer on the sign path. IdxTree/IdxLeaf are still the bottom-layer pair
		 * here: XSlhDsa_HtPeelLayerIndices only rewrites them once the upper-layer
		 * loop below starts, so checking this bound after a layer peel would be a
		 * no-op against the wrong field width.
		 */
		Status = XSlhDsa_HtCheckIndices(InstPtr, ParamPtr, IdxTree, IdxLeaf, SigHt);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		/**
		 * - Step 1: ADRS <- toByte(0, 32)
		 * - Step 2: ADRS.setTreeAddress(idxtree)
		 * - Step 3: SIG_tmp <- xmss_sign(M, SK.seed, idxleaf, PK.seed, ADRS)
		 * - Step 4: SIG_HT <- SIG_tmp
		 *
		 * Layer 0 signs the FORS public key that the caller staged in LayerMsg.
		 */
		if ((YieldPtr->Xmss.EmitResume != 0U) && (YieldPtr->Xmss.EmitLayerSign == SigHt)) {
			Status = XSlhDsa_XmssEmitSig(InstPtr, LayerMsg, YieldPtr->Xmss.EmitIdxLeaf,
						     SigHt);
		} else {
			Status = XSlhDsa_HtEmitBottomLayer(InstPtr, LayerMsg, IdxTree, IdxLeaf,
							    SigHt);
		}
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	/**
	 * - Step 5: root <- xmss_pkFromSig(idxleaf, SIG_tmp, M, PK.seed, ADRS)
	 * - Step 6 to 16: for j from 1 to d - 1, sign the root below and append SIG_tmp
	 *
	 * Layer signatures are appended to SigHt in layer order, bottom first.
	 */
	Status = XSlhDsa_HtEmitUpperLayers(InstPtr, ParamPtr, LayerMsg, &IdxTree, &IdxLeaf,
					SigHt);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Verify the hypertree component of an SLH-DSA signature
*		(FIPS 205 Algorithm 13: ht_verify).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	Msg	Pointer to message digest used for bottom-layer WOTS+ recovery.
* @param	SignHt	Pointer to hypertree signature (d XMSS signatures).
* @param	HtIndices	Pointer to hypertree tree and leaf index values.
*
* @return
*		- XASUFW_SUCCESS, if the recovered root matches PkRoot.
*		- XASUFW_SLHDSA_INVALID_PARAM, if any required pointer is NULL, the parsed
*		  tree/leaf index pair is outside the hypertree structure, or the hypertree
*		  signature section does not fit the registered signature buffer.
*		- XASUFW_SLHDSA_VERIFY_ROOT_MISMATCH, if the recovered root differs from PkRoot.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
* @note
*	InstPtr->NodeBuf must provide at least n bytes of scratch space for node recovery.
*
*	On a match this raises the global ReturnStatus to XASUFW_SLHDSA_SIGNATURE_VERIFIED, and
*	only after two independent root comparisons agree. It never lowers the indicator, so the
*	caller must clear ReturnStatus before the call and check it after, so that a value left
*	by an earlier operation cannot stand in for this verdict.
*
*************************************************************************************************/
s32 XSlhDsa_HtVerify(XSlhDsa_Instance *InstPtr, const u8 *Msg, const u8 *SignHt,
			  const XSlhDsa_HtIndices *HtIndices)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XSlhDsa_Param *ParamPtr;
	const u8 *SignCursor;
	u8 *NodePtr;
	u64 IdxTree;
	u64 IdxLeaf;

	/**
	 * Validate input pointers, including the n-byte node scratch and the staged PK.root the
	 * final comparison is made against, so a verdict is never produced from an unstaged key.
	 */
	if ((InstPtr == NULL) || (InstPtr->Addr == NULL) ||
			(Msg == NULL) || (SignHt == NULL) || (HtIndices == NULL) ||
			(InstPtr->NodeBuf == NULL) || (InstPtr->PkRoot == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	ParamPtr = &InstPtr->Param;
	NodePtr = InstPtr->NodeBuf;
	/** Local index copies and a local signature cursor: both are consumed layer by layer. */
	IdxTree = HtIndices->IdxTree;
	IdxLeaf = HtIndices->IdxLeaf;
	SignCursor = SignHt;

	/**
	 * Reject a tree/leaf pair outside the hypertree structure and a hypertree section that
	 * would run past the registered signature buffer, before any layer rebinds ADRS or
	 * reads a signature byte, so neither an out-of-range pair nor an over-long walk can be
	 * accepted under a tree, leaf, or buffer span the signer never used.
	 */
	Status = XSlhDsa_HtCheckIndices(InstPtr, ParamPtr, IdxTree, IdxLeaf, SignHt);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Step 1: ADRS <- toByte(0, 32)
	 * - Step 2: ADRS.setTreeAddress(idxtree)
	 * - Step 3: SIG_tmp <- SIG_HT.getXMSSSignature(0)
	 * - Step 4: node <- xmss_pkFromSig(idxleaf, SIG_tmp, M, PK.seed, ADRS)
	 *
	 * The FORS public key in Msg is the signed message; NodeBuf then carries the running
	 * node up the hypertree.
	 */
	Status = XSlhDsa_HtRecoverBottomNode(InstPtr, Msg, SignCursor, IdxTree, IdxLeaf,
						 NodePtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Step 5 to 12: for j from 1 to d - 1, peel h' bits off idxtree, rebind ADRS to the
	 *   layer and tree, and recover the node from SIG_tmp with the node below as message.
	 */
	Status = XSlhDsa_HtClimbUpperLayers(InstPtr, ParamPtr, &SignCursor, &IdxTree, NodePtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Step 13 to 17: if node == PK.root then return true else return false
	 *
	 * The comparison runs twice, independently: a single disturbed compare must not be
	 * able to turn a mismatch into the verdict, so both runs have to agree on a match
	 * before the verified indicator is raised. The status is re-armed to a failure value
	 * before each call.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSlhDsa_HtRootMatchesPk(NodePtr, InstPtr->PkRoot, ParamPtr->n);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSlhDsa_HtRootMatchesPk(NodePtr, InstPtr->PkRoot, ParamPtr->n);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Raise the verified indicator only after both independent compares agree. */
	ReturnStatus = XASUFW_SLHDSA_SIGNATURE_VERIFIED;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute a FORS leaf secret, and optionally its leaf hash, at the current
*		address index.
*
* Derives the leaf secret with PRF at FORS_NONCE, which is Algorithm 14 (fors_skGen) from
* FIPS 205. When ApplyF is non-zero it also applies F at FORS_TREE, completing the z = 0
* base case of Algorithm 15 (fors_node) that yields the leaf node used in authentication
* path construction.
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	ApplyF	Non-zero to apply F after PRF; zero for PRF output only (OTS emission).
* @param	Out	Output buffer for the leaf hash (n bytes).
*
* @return
*		- XASUFW_SUCCESS, if leaf hash computation succeeds.
*		- Error code received from called functions in case of other failure from the
*		  called function.
*
*************************************************************************************************/
static s32 XSlhDsa_ForsLeafHash(XSlhDsa_Instance *InstPtr, u32 ApplyF, u8 *Out)
{
	s32 Status = XASUFW_FAILURE;

	/**
	 * - Step 2: sk <- fors_skGen(SK.seed, PK.seed, ADRS, i)
	 * - Step 3: ADRS.setTreeHeight(0)
	 *
	 * PRF at FORS_NONCE regenerates the leaf secret, domain-separated from FORS_TREE on
	 * the same slot; the index is set by the caller.
	 */
	XSlhDsa_AddrSetType(InstPtr->Addr, XSLHDSA_ADRS_FORS_NONCE);
	XSlhDsa_AddrSetHeight(InstPtr->Addr, 0);
	Status = XSlhDsa_HashPrf(InstPtr, Out);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if (ApplyF != 0U) {
		/**
		 * - Step 5: node <- F(PK.seed, ADRS, sk)
		 *
		 * F runs in place, so the leaf secret is overwritten by its hash and does not
		 * linger in the output buffer.
		 */
		XSlhDsa_AddrSetType(InstPtr->Addr, XSLHDSA_ADRS_FORS_TREE);
		Status = XSlhDsa_HashF(InstPtr, Out, Out);
		goto END;
	}
	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute the root of a FORS authentication subtree
*		(FIPS 205 Algorithm 15: fors_node, iterative form).
*
* Iteratively builds a Merkle subtree of height SubHeight starting at BaseLeaf using
* fors_node leaf hashes and H parent merges.
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	BaseLeaf	Base leaf index for the sibling subtree.
* @param	SubHeight	Sub-tree height (level index in auth path).
* @param	Root	Output root buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS, if subtree root computation succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if SubHeight or stack depth is invalid, or a
*		  restored checkpoint does not match the walk it resumes.
*		- XASUFW_CMD_IN_PROGRESS, if a cooperative yield slice expired and the walk will
*		  resume.
*		- XASUFW_SLHDSA_COPY_FAILED, if restoring or saving the treehash node stack fails.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
static s32 XSlhDsa_ForsSubtreeRoot(XSlhDsa_Instance *InstPtr, u32 BaseLeaf,
					u32 SubHeight, u8 *Root)
{
	s32 Status = XASUFW_FAILURE;
	s32 YieldStatus = XASUFW_FAILURE;
	s32 StackDepth = -1;
	XSlhDsa_YieldCtx *YieldPtr;
	u8 NodeStack[XSLHDSA_FORS_STACK_DEPTH][XSLHDSA_MAX_N];
	u8 *NodeSlot;
	u32 LeafIdx;
	u32 Seq;
	u32 SeqCount;
	u32 MergeLevel;

	/** A walk of height SubHeight keeps that many nodes pending, so it must fit the stack. */
	if (SubHeight >= XSLHDSA_FORS_STACK_DEPTH) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	YieldPtr = &InstPtr->Yield;
	/** BaseLeaf is a level-SubHeight node, so shifting up gives its first leaf. */
	SeqCount = (1U << SubHeight);

	if ((YieldPtr->Fors.TreeResume != 0U) && (YieldPtr->Fors.TreeBaseLeaf == BaseLeaf) &&
			(YieldPtr->Fors.TreeSubHeight == SubHeight)) {
		Seq = YieldPtr->Fors.TreeSeq;
		LeafIdx = YieldPtr->Fors.TreeLeafIdx;
		StackDepth = YieldPtr->Fors.TreeStackDepth;
		/**
		 * The merge pops from NodeStack at the restored depth, so a depth that does
		 * not match the restored position would step off either end of the array.
		 */
		if ((StackDepth < -1) || (StackDepth > (s32)SubHeight) || (Seq >= SeqCount)) {
			Status = XASUFW_SLHDSA_INVALID_PARAM;
			goto END;
		}
		Status = Xil_SMemCpy(NodeStack, sizeof(NodeStack), YieldPtr->Fors.TreeNodeStack,
				     sizeof(YieldPtr->Fors.TreeNodeStack),
				     sizeof(YieldPtr->Fors.TreeNodeStack));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_SLHDSA_COPY_FAILED;
			goto END;
		}
	} else {
		Seq = 0U;
		LeafIdx = BaseLeaf << SubHeight;
		StackDepth = -1;
		YieldPtr->Fors.TreeResume = 0U;
	}

	for (; Seq < SeqCount; Seq++) {
		MergeLevel = 0U;

		YieldStatus = XSlhDsa_YieldPollLeaf(InstPtr, XSLHDSA_YIELD_UNIT_FORS_LEAF);
		if (YieldStatus == XASUFW_CMD_IN_PROGRESS) {
			Status = Xil_SMemCpy(YieldPtr->Fors.TreeNodeStack,
					     sizeof(YieldPtr->Fors.TreeNodeStack), NodeStack,
					     sizeof(NodeStack), sizeof(NodeStack));
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_SLHDSA_COPY_FAILED;
				goto END;
			}
			YieldPtr->Fors.TreeResume = 1U;
			YieldPtr->Fors.TreeBaseLeaf = BaseLeaf;
			YieldPtr->Fors.TreeSubHeight = SubHeight;
			YieldPtr->Fors.TreeSeq = Seq;
			YieldPtr->Fors.TreeLeafIdx = LeafIdx;
			YieldPtr->Fors.TreeStackDepth = StackDepth;
			Status = YieldStatus;
			goto END;
		}
		if (YieldStatus != XASUFW_SUCCESS) {
			Status = YieldStatus;
			goto END;
		}

		/** - Refuse to push past the end of the node stack. */
		if (StackDepth >= (s32)XSLHDSA_FORS_STACK_DEPTH) {
			Status = XASUFW_SLHDSA_INVALID_PARAM;
			goto END;
		}

		/**
		 * - ADRS: index = current leaf in sibling subtree. The first leaf goes straight
		 *   into the caller's Root buffer; later leaves are pushed on the node stack.
		 */
		XSlhDsa_AddrSetIndex(InstPtr->Addr, LeafIdx);
		NodeSlot = (StackDepth >= 0) ? NodeStack[StackDepth] : Root;
		StackDepth++;
		/** - Leaf nodes need F applied over the regenerated secret, hence ApplyF = 1. */
		Status = XSlhDsa_ForsLeafHash(InstPtr, 1U, NodeSlot);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		/**
		 * - Collapse one level per trailing set bit of Seq: each marks a right sub-tree
		 *   just completed, so its pending left sibling is popped and merged.
		 */
		while (((Seq >> MergeLevel) & 1U) != 0U) {
			/** - ADRS: height and index advance for internal FORS_TREE H merge. */
			XSlhDsa_AddrSetHeight(InstPtr->Addr, MergeLevel + 1U);
			XSlhDsa_AddrSetIndex(InstPtr->Addr, LeafIdx >> (MergeLevel + 1U));
			/**
			 * - Parent overwrites the left slot; when the stack drains it hits Root.
			 */
			StackDepth--;
			NodeSlot = (StackDepth > 0) ? NodeStack[StackDepth - 1] : Root;
			Status = XSlhDsa_HashH(InstPtr, NodeSlot, NodeStack[StackDepth],
						    NodeSlot);
			if (Status != XASUFW_SUCCESS) {
				goto END;
			}
			MergeLevel++;
		}
		LeafIdx++;
	}

	YieldPtr->Fors.TreeResume = 0U;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Decode packed bytes into 4-bit Winternitz digits (lg_w = 4 fast path for base_2b).
*
* @param	Src		Input byte stream.
* @param	Count		Number of digits to produce.
* @param	DigitsOut	Output digit buffer.
*
*************************************************************************************************/
static void XSlhDsa_WotsDecodeWin4Digits(const u8 *Src, u32 Count, u32 *DigitsOut)
{
	u32 DigitIdx = 0;
	u32 Consumed = 0;
	u32 Carry = 0;
	u32 CarryBits = 0;
	u8 Byte;

	/**
	 * lg_w = 4 fast path: one source byte carries exactly two digits, high nibble first,
	 * so digits are emitted in pairs without any bit accumulator.
	 */
	while ((DigitIdx + 1U) < Count) {
		Byte = Src[Consumed++];
		DigitsOut[DigitIdx++] = ((u32)Byte >> XSLHDSA_WOTS_LG_W);
		DigitsOut[DigitIdx++] = ((u32)Byte &
					 XSLHDSA_BIT_MASK(XSLHDSA_WOTS_LG_W));
	}

	/** Odd trailing digit: accumulate bits so the last digit lands on the right offset. */
	while (DigitIdx < Count) {
		while (CarryBits < XSLHDSA_WOTS_LG_W) {
			Carry = (Carry << XASUFW_ONE_BYTE_SHIFT_VALUE) | Src[Consumed++];
			CarryBits += XASUFW_BYTE_LEN_IN_BITS;
		}
		CarryBits -= XSLHDSA_WOTS_LG_W;
		DigitsOut[DigitIdx++] = (Carry >> CarryBits) &
					XSLHDSA_BIT_MASK(XSLHDSA_WOTS_LG_W);
	}
}
/*************************************************************************************************/
/**
* @brief	Decode the message portion of an n-byte string into base-2^lg_w digits
*		(FIPS 205 Algorithm 4: base_2b, message chains len_1).
*
* @param	Msg		Message buffer (n bytes).
* @param	ParamPtr	Active parameter set.
* @param	DigitsOut	Output buffer for len_1 message-chain digits.
*
* @return
*		- XASUFW_SUCCESS, if the message digits are decoded.
*		- Error code from XSlhDsa_Base2b() on failure (the win4 fast path cannot fail).
*
*************************************************************************************************/
static s32 XSlhDsa_WotsDecodeMsgDigits(const u8 *Msg,
					     const XSlhDsa_Param *ParamPtr,
					     u32 *DigitsOut)
{
	s32 Status = XASUFW_FAILURE;
	u32 MsgChains = ParamPtr->wots_len1;

	/**
	 * - Step 2: msg <- base_2b(M, lg_w, len_1)
	 *
	 * base_2b over the n-byte message yields len_1 = ceil(8n / lg_w) digits. Supported
	 * parameter sets all use lg_w = 4 and take the nibble fast path.
	 */
	if (ParamPtr->wots_win_bits == XSLHDSA_WOTS_LG_W) {
		XSlhDsa_WotsDecodeWin4Digits(Msg, MsgChains, DigitsOut);
		Status = XASUFW_SUCCESS;
	}
	else {
		Status = XSlhDsa_Base2b(Msg, ParamPtr->wots_win_bits, MsgChains, DigitsOut);
	}

	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute the WOTS+ checksum value over decoded message digits
*		(FIPS 205 Algorithm 7: wots_sign, steps 3 to 6).
*
* @param	MsgDigits	Decoded message-chain digits (len_1 elements).
* @param	MsgChains	Number of message-chain positions (len_1).
* @param	WinMax		(2^lg_w - 1).
* @param	PadBits		Left-shift applied before byte packing.
*
* @return
*		Checksum integer prior to base-2^lg_w expansion.
*
*************************************************************************************************/
static u32 XSlhDsa_WotsChecksumValue(const u32 *MsgDigits, u32 MsgChains,
					  u32 WinMax, u32 PadBits)
{
	u32 CsVal = 0;
	u32 Idx;

	/**
	 * - Step 3 to 5: for i from 0 to len_1 - 1 do csum <- csum + w - 1 - msg[i]
	 *
	 * Signing the checksum too is what stops a forger from raising a message digit: that
	 * always lowers the checksum, and no chain can be walked backwards.
	 */
	for (Idx = 0; Idx < MsgChains; Idx++) {
		CsVal += WinMax - MsgDigits[Idx];
	}

	/** - Step 6: csum <- csum << ((8 - ((len_2 * lg_w) mod 8)) mod 8) */
	return CsVal << PadBits;
}
/*************************************************************************************************/
/**
* @brief	Expand a packed checksum integer into len_2 base-2^lg_w digits
*		(FIPS 205 Algorithm 4: base_2b, checksum chains len_2).
*
* @param	CsVal		Checksum integer.
* @param	ParamPtr	Active parameter set.
* @param	CsDigitsOut	Output buffer for checksum-chain digits.
*
* @return
*		- XASUFW_SUCCESS, if checksum digits are expanded.
*		- XASUFW_ZEROIZE_MEMSET_FAIL, if the CsOctets staging buffer cannot be cleared.
*		- XASUFW_SLHDSA_INVALID_PARAM, if the base_2b expansion rejects wots_win_bits or
*		  CsChains (propagated from XSlhDsa_Base2b).
*
*************************************************************************************************/
static s32 XSlhDsa_WotsExpandChecksumDigits(u32 CsVal,
						 const XSlhDsa_Param *ParamPtr,
						 u32 *CsDigitsOut)
{
	s32 Status = XASUFW_FAILURE;
	u8 CsOctets[XASUFW_WORD_LEN_IN_BYTES];
	u32 CsChains = ParamPtr->wots_len2;
	u32 OctetLen = ((CsChains * ParamPtr->wots_win_bits) +
			XASUFW_MAX_BIT_INDEX_IN_BYTE) / XASUFW_BYTE_LEN_IN_BITS;

	/** Clear the staging buffer, since only its leading OctetLen bytes are written below. */
	Status = Xil_SMemSet(CsOctets, sizeof(CsOctets), 0U, sizeof(CsOctets));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/**
	 * - Step 7: msg <- msg || base_2b(toByte(csum, ceil(len_2 * lg_w / 8)), lg_w, len_2)
	 *
	 * Pack the shifted checksum big-endian into ceil(len_2 * lg_w / 8) octets, then split
	 * it into the len_2 checksum digits.
	 */
	XAsufw_U64ToBeBytes(CsVal, OctetLen, CsOctets);

	if (ParamPtr->wots_win_bits == XSLHDSA_WOTS_LG_W) {
		XSlhDsa_WotsDecodeWin4Digits(CsOctets, CsChains, CsDigitsOut);
		Status = XASUFW_SUCCESS;
	}
	else {
		Status = XSlhDsa_Base2b(CsOctets, ParamPtr->wots_win_bits, CsChains, CsDigitsOut);
	}

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Derive all WOTS+ chain start indices for a message (len_1 + len_2 digits).
*
* Combines base_2b message decoding and checksum expansion per FIPS 205 wots_sign /
* wots_pkFromSig index derivation.
*
* @param	Msg		Message buffer (n bytes).
* @param	ParamPtr	Active parameter set.
* @param	ChainIndicesOut	Output buffer for wots_len chain indices.
*
* @return
*		- XASUFW_SUCCESS, if chain indices are derived.
*		- XASUFW_SLHDSA_INVALID_PARAM, if len_1 and len_2 do not split the bound wots_len.
*		- Error code from XSlhDsa_WotsDecodeMsgDigits() or
*		  XSlhDsa_WotsExpandChecksumDigits() on failure.
*
* @note
*	The caller bounds wots_len against the digit buffer. Only the split checked here ties the
*	two writes below to that bound.
*
*************************************************************************************************/
static s32 XSlhDsa_WotsDeriveChainIndices(const u8 *Msg,
					       const XSlhDsa_Param *ParamPtr,
					       u32 *ChainIndicesOut)
{
	s32 Status = XASUFW_FAILURE;
	u32 MsgChains = ParamPtr->wots_len1;
	u32 CsChains = ParamPtr->wots_len2;
	u32 WinMax = XSLHDSA_BIT_MASK(ParamPtr->wots_win_bits);
	u32 PadBits = (XASUFW_BYTE_LEN_IN_BITS -
		       ((CsChains * ParamPtr->wots_win_bits) &
			XASUFW_MAX_BIT_INDEX_IN_BYTE)) &
		      XASUFW_MAX_BIT_INDEX_IN_BYTE;
	u32 CsVal;

	/**
	 * The two writes below are sized by len_1 and len_2, while the buffer was bounded
	 * against wots_len, so confirm they still describe that same chain count. The limit is
	 * formed by subtraction rather than len_1 + len_2, which could wrap and let a pair that
	 * overruns the buffer pass.
	 */
	if ((MsgChains > ParamPtr->wots_len) ||
			(CsChains != (ParamPtr->wots_len - MsgChains))) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * PadBits counts the spare low bits of the last packed checksum octet, so shifting by
	 * it keeps the len_2 digits byte aligned. The len_1 message digits land at the front of
	 * ChainIndicesOut and the checksum digits computed over them are appended right after,
	 * giving one start index per WOTS+ chain in chain order.
	 */
	Status = XSlhDsa_WotsDecodeMsgDigits(Msg, ParamPtr, ChainIndicesOut);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}
	CsVal = XSlhDsa_WotsChecksumValue(ChainIndicesOut, MsgChains, WinMax, PadBits);

	Status = XSlhDsa_WotsExpandChecksumDigits(CsVal, ParamPtr,
						     ChainIndicesOut + MsgChains);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Write WOTS+ signature chain endpoints using pre-derived chain indices
*		(wots_sign inner loop).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	ChainIndices	Chain start indices (wots_len elements).
* @param	ParamPtr	Active parameter set.
* @param	SigOut	Output cursor for WOTS+ signature bytes.
*
* @return
*		- XASUFW_SUCCESS, if all chains are written.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
static s32 XSlhDsa_WotsWriteSignChains(XSlhDsa_Instance *InstPtr,
					    const u32 *ChainIndices,
					    const XSlhDsa_Param *ParamPtr, u8 *SigOut)
{
	s32 Status = XASUFW_FAILURE;
	u32 HashWidth = ParamPtr->n;
	u32 TotalChains = ParamPtr->wots_len;
	u32 ChainIdx;

	/** - Step 11: for i from 0 to len - 1 do */
	for (ChainIdx = 0; ChainIdx < TotalChains; ChainIdx++) {
		/** - Step 12/14: skADRS.setChainAddress(i) and ADRS.setChainAddress(i) */
		XSlhDsa_AddrSetChainAddr(InstPtr->Addr, ChainIdx);
		/**
		 * - Step 15: sig[i] <- chain(sk, 0, msg[i], PK.seed, ADRS)
		 *
		 * sig[i] is the chain value ChainIndices[i] steps above the secret, leaving
		 * (2^lg_w - 1 - index) steps for the verifier, written straight to its slot.
		 */
		Status = XSlhDsa_WotsChainFromZero(InstPtr, ChainIndices[ChainIdx], SigOut);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		SigOut += HashWidth;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Advance each WOTS+ signature element to the chain top (wots_pkFromSig inner loop).
*
* Runs chain() from each signature element for (2^lg_w - 1 - index) steps per FIPS 205
* Algorithm 8.
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	WotsSign	WOTS+ signature bytes.
* @param	ChainIndices	Chain start indices (wots_len elements).
* @param	ParamPtr	Active parameter set.
* @param	ScratchOut	Scratch buffer for recovered chain tops (wots_len * n).
*
* @return
*		- XASUFW_SUCCESS, if all chain tops are recovered.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
static s32 XSlhDsa_WotsRebuildChainTops(XSlhDsa_Instance *InstPtr,
					     const u8 *WotsSign, const u32 *ChainIndices,
					     const XSlhDsa_Param *ParamPtr,
					     u8 *ScratchOut)
{
	s32 Status = XASUFW_FAILURE;
	XSlhDsa_ChainCfg ChainCfg;
	const u8 *SignCursor = WotsSign;
	u8 *NodeCursor = ScratchOut;
	u32 HashWidth = ParamPtr->n;
	u32 WinMax = XSLHDSA_BIT_MASK(ParamPtr->wots_win_bits);
	u32 TotalChains = ParamPtr->wots_len;
	u32 ChainIdx;

	/** - Step 8: for i from 0 to len - 1 do */
	for (ChainIdx = 0; ChainIdx < TotalChains; ChainIdx++) {
		/** - Step 9: ADRS.setChainAddress(i); WOTS_HASH type set by caller. */
		XSlhDsa_AddrSetChainAddr(InstPtr->Addr, ChainIdx);
		/**
		 * - Step 10: tmp[i] <- chain(sig[i], msg[i], w - 1 - msg[i], PK.seed, ADRS)
		 *
		 * Resume at the index the signer stopped on and run the remaining
		 * (2^lg_w - 1 - index) steps. StartIdx must be the signer's index because the
		 * hash slot is part of the F address, so only that offset reproduces the
		 * key generator's full-length chain.
		 */
		ChainCfg.StartIdx = ChainIndices[ChainIdx];
		ChainCfg.Steps = WinMax - ChainIndices[ChainIdx];

		Status = XSlhDsa_Chain(InstPtr, SignCursor, &ChainCfg, NodeCursor);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		/** Both cursors step n bytes: one signature element in, one chain top out. */
		SignCursor += HashWidth;
		NodeCursor += HashWidth;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Derive one XMSS leaf as a WOTS+ public key (FIPS 205 Algorithm 6: wots_pkGen).
*
* Runs all WOTS+ chains to their tops for the given key-pair address, then compresses
* with H_TL at WOTS_PK to produce the leaf node hash.
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	LeafIdx	Key-pair address for this leaf.
* @param	ParamPtr	Active parameter set.
* @param	ChainScratch	Work buffer for WOTS+ chain tops (len * n bytes).
* @param	LeafPkOut	Output leaf public key (n bytes).
*
* @return
*		- XASUFW_SUCCESS, if leaf PK build succeeds.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
static s32 XSlhDsa_XmssBuildLeafPk(XSlhDsa_Instance *InstPtr, u32 LeafIdx,
					const XSlhDsa_Param *ParamPtr, u8 *ChainScratch,
					u8 *LeafPkOut)
{
	s32 Status = XASUFW_FAILURE;
	u8 *ChainOut = ChainScratch;
	u32 HashWidth = ParamPtr->n;
	u32 WinTop = XSLHDSA_BIT_MASK(ParamPtr->wots_win_bits);
	u32 TotalChains = ParamPtr->wots_len;
	u32 ChainIdx;

	/**
	 * - Step 3: skADRS.setKeyPairAddress(ADRS.getKeyPairAddress())
	 * - Step 4: for i from 0 to len - 1 do
	 * - Step 5/7: skADRS.setChainAddress(i) and ADRS.setChainAddress(i)
	 * - Step 6: sk <- PRF(PK.seed, SK.seed, skADRS)
	 * - Step 8: tmp[i] <- chain(sk, 0, w - 1, PK.seed, ADRS)
	 *
	 * wots_pkGen runs every chain the full WinTop = (2^lg_w - 1) steps, and the per-chain
	 * address is what keeps their F calls domain-separated.
	 */
	XSlhDsa_AddrSetKeyPair(InstPtr->Addr, LeafIdx);
	for (ChainIdx = 0; ChainIdx < TotalChains; ChainIdx++) {
		XSlhDsa_AddrSetChainAddr(InstPtr->Addr, ChainIdx);
		Status = XSlhDsa_WotsChainFromZero(InstPtr, WinTop, ChainOut);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		ChainOut += HashWidth;
	}

	/**
	 * - Step 10 to 12: wotspkADRS <- ADRS, setTypeAndClear(WOTS_PK), setKeyPairAddress()
	 * - Step 13: pk <- T_len(PK.seed, wotspkADRS, tmp)
	 *
	 * Only height and index are cleared, so the key-pair set above still binds the
	 * compression to this leaf.
	 */
	XSlhDsa_AddrSetTypeClearAux(InstPtr->Addr, XSLHDSA_ADRS_WOTS_PK);
	Status = XSlhDsa_HashTl(InstPtr, ChainScratch, TotalChains * HashWidth, LeafPkOut);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Merge stacked XMSS leaf nodes upward while the leaf sequence carries
*		(FIPS 205 Algorithm 9: xmss_node internal merge).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	LeafIdx	Current leaf index within the sub-tree.
* @param	LeafSeq	Leaf sequence counter within the sub-tree walk.
* @param	NodeStack	Stack of pending internal nodes.
* @param	StackDepthPtr	In/out stack depth (reference p counter).
* @param	Root	Sub-tree root output buffer.
*
* @return
*		- XASUFW_SUCCESS, if merge succeeds.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_INVALID_PARAM,
*		  XASUFW_SLHDSA_COPY_FAILED, XASUFW_SHA_START_FAILED,
*		  XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
static s32 XSlhDsa_XmssCollapseCarryLevels(XSlhDsa_Instance *InstPtr, u32 LeafIdx,
						u32 LeafSeq, u8 NodeStack[][XSLHDSA_MAX_N],
						s32 *StackDepthPtr, u8 *Root)
{
	s32 Status = XASUFW_FAILURE;
	u32 MergeLevel = 0;
	s32 StackDepth = *StackDepthPtr;
	u8 *ParentSlot;

	/**
	 * Merge for every trailing set bit of LeafSeq: a run of set low bits means this leaf
	 * completed that many right sub-trees, so each iteration pops one level's pending left
	 * sibling and hashes the pair into their parent.
	 */
	while (((LeafSeq >> MergeLevel) & 1U) != 0U) {
		/**
		 * - Step 8: ADRS.setTypeAndClear(TREE)
		 * - Step 9: ADRS.setTreeHeight(z)
		 * - Step 10: ADRS.setTreeIndex(i)
		 *
		 * Height is the parent level and index its position there, so each H call is
		 * unique.
		 */
		XSlhDsa_AddrSetTypeClearTail(InstPtr->Addr, XSLHDSA_ADRS_XMSS_NODE);
		XSlhDsa_AddrSetHeight(InstPtr->Addr, MergeLevel + 1U);
		XSlhDsa_AddrSetIndex(InstPtr->Addr, LeafIdx >> (MergeLevel + 1U));
		/**
		 * - Step 11: node <- H(PK.seed, ADRS, lnode || rnode)
		 *
		 * Pop the left sibling; the parent takes its slot, or Root once drained.
		 */
		StackDepth--;
		ParentSlot = (StackDepth >= 1) ? NodeStack[StackDepth - 1] : Root;
		Status = XSlhDsa_HashH(InstPtr, ParentSlot, NodeStack[StackDepth], ParentSlot);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		MergeLevel++;
	}

	*StackDepthPtr = StackDepth;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Evaluate one XMSS leaf and fold it into the iterative Merkle stack
*		(FIPS 205 Algorithm 9: xmss_node).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	LeafIdx	Current leaf index.
* @param	LeafSeq	Leaf sequence counter within the sub-tree walk.
* @param	ParamPtr	Active parameter set.
* @param	NodeStack	Stack of pending internal nodes.
* @param	StackDepthPtr	In/out stack depth.
* @param	Root	Sub-tree root output buffer.
* @param	ChainScratch	WOTS+ chain scratch buffer.
*
* @return
*		- XASUFW_SUCCESS, if leaf integration succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if stack depth exceeds the maximum.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
static s32 XSlhDsa_XmssIntegrateLeaf(XSlhDsa_Instance *InstPtr, u32 LeafIdx,
					  u32 LeafSeq, const XSlhDsa_Param *ParamPtr,
					  u8 NodeStack[][XSLHDSA_MAX_N],
					  s32 *StackDepthPtr, u8 *Root, u8 *ChainScratch)
{
	s32 Status = XASUFW_FAILURE;
	u8 *LeafSlot;
	s32 StackDepth = *StackDepthPtr;

	/** Refuse to push past the end of the node stack. */
	if (StackDepth >= (s32)XSLHDSA_XMSS_STACK_DEPTH) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** The first leaf goes straight into Root; later leaves are pushed on the node stack. */
	LeafSlot = (StackDepth >= 0) ? NodeStack[StackDepth] : Root;
	StackDepth++;
	/**
	 * - Step 2 to 4: z = 0 base case; ADRS.setTypeAndClear(WOTS_HASH),
	 *   ADRS.setKeyPairAddress(i) and node <- wots_pkGen(SK.seed, PK.seed, ADRS)
	 */
	Status = XSlhDsa_XmssBuildLeafPk(InstPtr, LeafIdx, ParamPtr, ChainScratch, LeafSlot);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Publish the pushed depth before collapsing, since the merge pops from it. */
	*StackDepthPtr = StackDepth;
	Status = XSlhDsa_XmssCollapseCarryLevels(InstPtr, LeafIdx, LeafSeq, NodeStack,
					       StackDepthPtr, Root);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Validate that a leaf index is inside the bound XMSS subtree.
*
* @param	ParamPtr	Active parameter set.
* @param	IdxLeaf	Leaf index to check against the h'-bit subtree.
*
* @return
*		- XASUFW_SUCCESS, if the leaf slot is inside the subtree, which holds
*		  2^h' leaves.
*		- XASUFW_SLHDSA_INVALID_PARAM, if xmss_height is zero, above the widest FIPS
*		  205 XMSS subtree, or the leaf slot is outside the subtree.
*
*************************************************************************************************/
static s32 XSlhDsa_XmssCheckLeaf(const XSlhDsa_Param *ParamPtr, u32 IdxLeaf)
{
	s32 Status = XASUFW_SLHDSA_INVALID_PARAM;

	/**
	 * Reject xmss_height == 0 or above XSLHDSA_MAX_XMSS_HEIGHT first: the shift below is
	 * undefined once the shift amount reaches the width of IdxLeaf's u32 type.
	 */
	if ((ParamPtr->xmss_height == 0U) || (ParamPtr->xmss_height > XSLHDSA_MAX_XMSS_HEIGHT)) {
		goto END;
	}

	if (IdxLeaf < ((u32)1U << ParamPtr->xmss_height)) {
		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Write the WOTS+ portion of an XMSS signature bundle
*		(FIPS 205 Algorithm 10: xmss_sign, steps 5 to 7).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	Msg	Message buffer to sign (n bytes).
* @param	IdxLeaf	Leaf index within the XMSS tree.
* @param	WotsSigOut	Output buffer for WOTS+ signature (len * n bytes).
*
* @return
*		- XASUFW_SUCCESS, if WOTS+ signing succeeds.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_ZEROIZE_MEMSET_FAIL,
*		  XASUFW_SLHDSA_COPY_FAILED, XASUFW_SHA_START_FAILED,
*		  XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
static s32 XSlhDsa_XmssWriteWotsSignature(XSlhDsa_Instance *InstPtr,
					       const u8 *Msg, u32 IdxLeaf, u8 *WotsSigOut)
{
	s32 Status = XASUFW_FAILURE;

	/**
	 * - Step 5: ADRS.setTypeAndClear(WOTS_HASH)
	 * - Step 6: ADRS.setKeyPairAddress(idx)
	 * - Step 7: sig <- wots_sign(M, SK.seed, PK.seed, ADRS)
	 *
	 * Height and index are cleared so the chain address starts clean, while the layer and
	 * tree fields set by the hypertree caller survive and keep this signature bound to one
	 * tree.
	 */
	XSlhDsa_AddrSetTypeClearAux(InstPtr->Addr, XSLHDSA_ADRS_WOTS_HASH);
	XSlhDsa_AddrSetKeyPair(InstPtr->Addr, IdxLeaf);
	Status = XSlhDsa_WotsEmitSig(InstPtr, Msg, WotsSigOut);

	return Status;
}
/*************************************************************************************************/
/**
* @brief	Write the XMSS authentication path nodes for a leaf signature
*		(FIPS 205 Algorithm 10: xmss_sign, steps 1 to 4).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	IdxLeaf	Leaf index within the XMSS tree.
* @param	ParamPtr	Active parameter set.
* @param	AuthPathOut	Output buffer for h' sibling roots (h' * n bytes).
*
* @return
*		- XASUFW_SUCCESS, if auth path generation succeeds.
*		- XASUFW_CMD_IN_PROGRESS, if a cooperative yield slice expired and the walk will
*		  resume.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
static s32 XSlhDsa_XmssWriteAuthPathNodes(XSlhDsa_Instance *InstPtr, u32 IdxLeaf,
					       const XSlhDsa_Param *ParamPtr,
					       u8 *AuthPathOut)
{
	s32 Status = XASUFW_FAILURE;
	s32 YieldStatus = XASUFW_FAILURE;
	u32 HashWidth = ParamPtr->n;
	u32 LevelStart = 0U;
	XSlhDsa_YieldCtx *YieldPtr;
	u32 LevelIdx;
	u32 SibLeafBase;

	YieldPtr = &InstPtr->Yield;
	if ((YieldPtr->Xmss.AuthPathResume != 0U) && (YieldPtr->Xmss.AuthPathIdxLeaf == IdxLeaf) &&
			(YieldPtr->Xmss.AuthPathSigBase == AuthPathOut)) {
		LevelStart = YieldPtr->Xmss.AuthPathLevelIdx;
		AuthPathOut = YieldPtr->Xmss.AuthPathOut;
	} else {
		YieldPtr->Xmss.AuthPathResume = 0U;
		YieldPtr->Xmss.Resume = 0U;
		YieldPtr->Xmss.AuthPathSigBase = AuthPathOut;
	}

	/** - Step 1: for j from 0 to h' - 1 do */
	for (LevelIdx = LevelStart; LevelIdx < ParamPtr->xmss_height; LevelIdx++) {
		/**
		 * - Step 2: k <- floor(idx / 2^j) XOR 1
		 *
		 * Locate the sibling sub-tree: flip bit LevelIdx of the signing leaf index,
		 * then shift back up to clear the lower bits and land on its first leaf.
		 */
		SibLeafBase = ((IdxLeaf >> LevelIdx) ^ 1U) << LevelIdx;

		YieldStatus = XSlhDsa_YieldPoll(InstPtr, XSLHDSA_YIELD_UNIT_XMSS_LEAF);
		if (YieldStatus == XASUFW_CMD_IN_PROGRESS) {
			YieldPtr->Xmss.AuthPathResume = 1U;
			YieldPtr->Xmss.AuthPathIdxLeaf = IdxLeaf;
			YieldPtr->Xmss.AuthPathLevelIdx = LevelIdx;
			YieldPtr->Xmss.AuthPathOut = AuthPathOut;
			Status = YieldStatus;
			goto END;
		}
		if (YieldStatus != XASUFW_SUCCESS) {
			Status = YieldStatus;
			goto END;
		}

		/** - Step 3: AUTH[j] <- xmss_node(SK.seed, k, j, PK.seed, ADRS) */
		Status = XSlhDsa_XmssSubtreeRoot(InstPtr, SibLeafBase, LevelIdx, AuthPathOut);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			YieldPtr->Xmss.AuthPathResume = 1U;
			YieldPtr->Xmss.AuthPathIdxLeaf = IdxLeaf;
			YieldPtr->Xmss.AuthPathLevelIdx = LevelIdx;
			YieldPtr->Xmss.AuthPathOut = AuthPathOut;
			goto END;
		}
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		AuthPathOut += HashWidth;
	}

	YieldPtr->Xmss.AuthPathResume = 0U;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Reconstruct the WOTS+ leaf public key at the start of XMSS verification
*		(FIPS 205 Algorithm 11: xmss_pkFromSig, steps 1 to 5).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	SignXmss	Pointer to XMSS signature (WOTS+ sig plus auth path).
* @param	Msg	Message digest passed to WOTS+ recovery.
* @param	IdxLeaf	Leaf index within the XMSS subtree.
* @param	LeafPkOut	Output leaf node buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS, if WOTS+ recovery succeeds.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_ZEROIZE_MEMSET_FAIL,
*		  XASUFW_SLHDSA_COPY_FAILED, XASUFW_SHA_START_FAILED,
*		  XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
static s32 XSlhDsa_XmssRebuildLeafPk(XSlhDsa_Instance *InstPtr,
					  const u8 *SignXmss, const u8 *Msg, u32 IdxLeaf,
					  u8 *LeafPkOut)
{
	s32 Status = XASUFW_FAILURE;

	/**
	 * - Step 1: ADRS.setTypeAndClear(WOTS_HASH)
	 * - Step 2: ADRS.setKeyPairAddress(idx)
	 * - Step 5: node[0] <- wots_pkFromSig(sig, M, PK.seed, ADRS)
	 *
	 * The key-pair points at the leaf under verification, so the chain walk reproduces the
	 * signer's addresses.
	 */
	XSlhDsa_AddrSetTypeClearNotKp(InstPtr->Addr, XSLHDSA_ADRS_WOTS_HASH);
	XSlhDsa_AddrSetKeyPair(InstPtr->Addr, IdxLeaf);
	Status = XSlhDsa_WotsPkFromSign(InstPtr, SignXmss, Msg, LeafPkOut);

	return Status;
}
/*************************************************************************************************/
/**
* @brief	Walk the XMSS authentication path to ascend from a leaf to the sub-tree node
*		(FIPS 205 Algorithm 11: xmss_pkFromSig, steps 6 to 18).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	IdxLeaf	Leaf index within the XMSS subtree.
* @param	AuthPathPtr	Pointer to authentication path (h' * n bytes).
* @param	ParamPtr	Active parameter set.
* @param	Node	In/out node buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS, if auth path ascent succeeds.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_INVALID_PARAM,
*		  XASUFW_SLHDSA_COPY_FAILED, XASUFW_SHA_START_FAILED,
*		  XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
static s32 XSlhDsa_XmssAscendAuthPath(XSlhDsa_Instance *InstPtr, u32 IdxLeaf,
					   const u8 *AuthPathPtr,
					   const XSlhDsa_Param *ParamPtr, u8 *Node)
{
	s32 Status = XASUFW_FAILURE;
	u32 HashWidth = ParamPtr->n;
	u32 LevelIdx;

	/**
	 * - Step 6: ADRS.setTypeAndClear(TREE)
	 * - Step 7: ADRS.setTreeIndex(idx)
	 *
	 * Clearing the tail drops the WOTS+ key-pair, height and index left by the leaf
	 * recovery.
	 */
	XSlhDsa_AddrSetTypeClearTail(InstPtr->Addr, XSLHDSA_ADRS_XMSS_NODE);

	/** - Step 8: for k from 0 to h' - 1 do */
	for (LevelIdx = 0; LevelIdx < ParamPtr->xmss_height; LevelIdx++) {
		/**
		 * - Step 9: ADRS.setTreeHeight(k + 1)
		 * - Step 11/14: ADRS.setTreeIndex() to the parent position
		 */
		XSlhDsa_AddrSetHeight(InstPtr->Addr, LevelIdx + 1U);
		XSlhDsa_AddrSetIndex(InstPtr->Addr, IdxLeaf >> (LevelIdx + 1U));

		/**
		 * - Step 10: if floor(idx / 2^k) is even then
		 * - Step 12: node[1] <- H(PK.seed, ADRS, node[0] || AUTH[k])
		 * - Step 15: node[1] <- H(PK.seed, ADRS, AUTH[k] || node[0])
		 * - Step 17: node[0] <- node[1]
		 *
		 * Bit LevelIdx of the leaf index says whether the running node is the right
		 * child, in which case the sibling is hashed on the left. Node is operand and
		 * destination, so it becomes its own parent each level.
		 */
		if (((IdxLeaf >> LevelIdx) & 1U) != 0U) {
			Status = XSlhDsa_HashH(InstPtr, AuthPathPtr, Node, Node);
		}
		else {
			Status = XSlhDsa_HashH(InstPtr, Node, AuthPathPtr, Node);
		}
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		AuthPathPtr += HashWidth;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Measure byte offsets of hypertree index fields within an H_msg digest.
*
* @param	ParamPtr	Active parameter set.
* @param	MdByteLen	Output FORS message-digest prefix length in bytes.
* @param	TreeFieldLen	Output tree-index field length in bytes.
* @param	LeafFieldLen	Output leaf-index field length in bytes.
*
*************************************************************************************************/
static void XSlhDsa_HtMeasureDigestLayout(const XSlhDsa_Param *ParamPtr,
					       u32 *MdByteLen, u32 *TreeFieldLen,
					       u32 *LeafFieldLen)
{
	/** The FORS indices consume the leading ceil(k * a / 8) digest bytes. */
	*MdByteLen = XAsufw_CeilDivU32(ParamPtr->fors_count * ParamPtr->fors_height,
					      XASUFW_BYTE_LEN_IN_BITS);
	/** Tree index needs (h - h') bits, one per hypertree level above the bottom tree. */
	*TreeFieldLen = XAsufw_CeilDivU32(ParamPtr->h - ParamPtr->xmss_height,
						XASUFW_BYTE_LEN_IN_BITS);
	/** Leaf index needs h/d = h' bits to address a leaf inside one XMSS subtree. */
	*LeafFieldLen = XAsufw_CeilDivU32(ParamPtr->h,
						XASUFW_BYTE_LEN_IN_BITS * ParamPtr->d);
}
/*************************************************************************************************/
/**
* @brief	Decode and reduce the hypertree tree index from an H_msg digest suffix.
*
* @param	Digest	Pointer to H_msg digest buffer.
* @param	MdByteLen	FORS message-digest prefix length in bytes.
* @param	TreeFieldLen	Tree-index field length in bytes.
* @param	ParamPtr	Active parameter set.
*
* @return
*		Reduced tree index (idx_tree).
*
*************************************************************************************************/
static u64 XSlhDsa_HtExtractTreeIndex(const u8 *Digest, u32 MdByteLen, u32 TreeFieldLen,
					   const XSlhDsa_Param *ParamPtr)
{
	u64 IdxTree = XAsufw_BeBytesToU64(Digest + MdByteLen, TreeFieldLen);
	u32 TreeBits = ParamPtr->h - ParamPtr->xmss_height;

	/** Keep the (h - h') significant bits so idx_tree names a bottom-layer tree. */
	if (TreeBits != XASUFW_BITS_IN_U64) {
		IdxTree = XAsufw_ModPow2U64(IdxTree, (u64)TreeBits);
	}

	return IdxTree;
}
/*************************************************************************************************/
/**
* @brief	Decode and reduce the hypertree leaf index from an H_msg digest suffix.
*
* @param	Digest	Pointer to H_msg digest buffer.
* @param	MdByteLen	FORS message-digest prefix length in bytes.
* @param	TreeFieldLen	Tree-index field length in bytes.
* @param	LeafFieldLen	Leaf-index field length in bytes.
* @param	ParamPtr	Active parameter set.
*
* @return
*		Reduced leaf index (idx_leaf).
*
*************************************************************************************************/
static u64 XSlhDsa_HtExtractLeafIndex(const u8 *Digest, u32 MdByteLen, u32 TreeFieldLen,
					   u32 LeafFieldLen, const XSlhDsa_Param *ParamPtr)
{
	u64 IdxLeaf =
			XAsufw_BeBytesToU64(Digest + MdByteLen + TreeFieldLen, LeafFieldLen);
	u32 LeafBits = ParamPtr->h / ParamPtr->d;

	/** Leaf field follows the tree field; only its low h' = h/d bits are significant. */
	return XAsufw_ModPow2U64(IdxLeaf, (u64)LeafBits);
}
/*************************************************************************************************/
/**
* @brief	Return the byte size of one XMSS layer signature within SIG_HT.
*
* @param	ParamPtr	Active parameter set.
*
* @return
*		(wots_len + h') * n bytes per hypertree layer signature.
*
*************************************************************************************************/
static u32 XSlhDsa_HtLayerSigBytes(const XSlhDsa_Param *ParamPtr)
{
	return (ParamPtr->xmss_height + ParamPtr->wots_len) * ParamPtr->n;
}
/*************************************************************************************************/
/**
* @brief	Peel the next hypertree layer's leaf index from the tree index word
*		(ht_sign / ht_verify).
*
* @param	ParamPtr	Active parameter set.
* @param	IdxTree	In/out tree index (shifted down by h' bits).
* @param	IdxLeafOut	Output leaf slot for the next layer.
*
*************************************************************************************************/
static void XSlhDsa_HtPeelLayerIndices(const XSlhDsa_Param *ParamPtr,
					    u64 *IdxTree, u32 *IdxLeafOut)
{
	u32 LeafMaskBits = ParamPtr->xmss_height;
	u64 LeafMask = (((u64)1U << (u64)LeafMaskBits) - (u64)1U);
	u64 IdxLeaf = *IdxTree & LeafMask;

	/**
	 * Moving up one layer: the low h' bits of the tree index are the leaf slot this tree
	 * occupies in its parent, and shifting them out leaves the parent's tree index.
	 */
	*IdxLeafOut = (u32)IdxLeaf;
	*IdxTree >>= LeafMaskBits;
}
/*************************************************************************************************/
/**
* @brief	Emit the bottom hypertree layer XMSS signature (ht_sign layer 0).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	LayerMsg	Message buffer to sign (Pk_FORS, n bytes).
* @param	IdxTree	Tree index for layer 0.
* @param	IdxLeaf	Leaf index for layer 0.
* @param	LayerSigOut	Output buffer for the first XMSS layer signature.
*
* @return
*		- XASUFW_SUCCESS, if bottom-layer XMSS signing succeeds.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_ZEROIZE_MEMSET_FAIL and any
*		  failure reported by XSlhDsa_XmssEmitSig()).
*
*************************************************************************************************/
static s32 XSlhDsa_HtEmitBottomLayer(XSlhDsa_Instance *InstPtr, const u8 *LayerMsg,
					  u64 IdxTree, u32 IdxLeaf, u8 *LayerSigOut)
{
	s32 Status = XASUFW_FAILURE;

	/**
	 * - Step 1: ADRS <- toByte(0, 32)
	 *
	 * Zeroing the whole address selects layer 0 and drops any field left by an earlier
	 * operation; the key-pair field is set inside xmss_sign.
	 */
	Status = Xil_SMemSet(InstPtr->Addr, sizeof(*InstPtr->Addr), 0U, sizeof(*InstPtr->Addr));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** - Step 2: ADRS.setTreeAddress(idxtree) */
	XSlhDsa_AddrSetTree(InstPtr->Addr, IdxTree);

	/** - Step 3: SIG_tmp <- xmss_sign(M, SK.seed, idxleaf, PK.seed, ADRS) */
	Status = XSlhDsa_XmssEmitSig(InstPtr, LayerMsg, IdxLeaf, LayerSigOut);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Emit XMSS signatures for hypertree layers 1 through (d - 1) (ht_sign upper layers).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	ParamPtr	Active parameter set.
* @param	LayerMsg	In/out n-byte buffer; updated to each recovered root.
* @param	IdxTree	In/out tree index word.
* @param	IdxLeaf	In/out leaf index for the active layer.
* @param	LayerSigOut	Output buffer positioned at layer-1 signature.
*
* @return
*		- XASUFW_SUCCESS, if upper-layer signing succeeds.
*		- XASUFW_CMD_IN_PROGRESS, if a cooperative yield slice expired and the walk will
*		  resume.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
static s32 XSlhDsa_HtEmitUpperLayers(XSlhDsa_Instance *InstPtr,
					  const XSlhDsa_Param *ParamPtr, u8 *LayerMsg,
					  u64 *IdxTree, u32 *IdxLeaf, u8 *LayerSigOut)
{
	s32 Status = XASUFW_FAILURE;
	s32 YieldStatus = XASUFW_FAILURE;
	u32 LevelStart = 1U;
	u32 LayerBytes = XSlhDsa_HtLayerSigBytes(ParamPtr);
	u8 *LayerSigBase = LayerSigOut;
	XSlhDsa_YieldCtx *YieldPtr;
	u32 LevelIdx;

	YieldPtr = &InstPtr->Yield;
	if ((YieldPtr->Ht.Resume != 0U) && (YieldPtr->Ht.SigBase == LayerSigBase)) {
		LevelStart = YieldPtr->Ht.LevelIdx;
		LayerSigOut = YieldPtr->Ht.LayerSigOut;
		*IdxTree = YieldPtr->Ht.IdxTree;
		*IdxLeaf = YieldPtr->Ht.IdxLeaf;
	} else {
		YieldPtr->Ht.SigBase = LayerSigBase;
	}

	/** - Step 6: for j from 1 to d - 1 do */
	for (LevelIdx = LevelStart; LevelIdx < ParamPtr->d; LevelIdx++) {
		if (YieldPtr->Ht.EmitPending != 0U) {
			YieldPtr->Ht.EmitPending = 0U;
			Status = XSlhDsa_XmssEmitSig(InstPtr, LayerMsg, *IdxLeaf, LayerSigOut);
			if (Status == XASUFW_CMD_IN_PROGRESS) {
				YieldPtr->Ht.Resume = 1U;
				YieldPtr->Ht.EmitPending = 1U;
				YieldPtr->Ht.LevelIdx = LevelIdx;
				YieldPtr->Ht.IdxTree = *IdxTree;
				YieldPtr->Ht.IdxLeaf = *IdxLeaf;
				YieldPtr->Ht.LayerSigOut = LayerSigOut;
				goto END;
			}
			if (Status != XASUFW_SUCCESS) {
				goto END;
			}
			continue;
		}

		YieldStatus = XSlhDsa_YieldPoll(InstPtr, XSLHDSA_YIELD_UNIT_XMSS_LEAF);
		if (YieldStatus == XASUFW_CMD_IN_PROGRESS) {
			YieldPtr->Ht.Resume = 1U;
			YieldPtr->Ht.EmitPending = 0U;
			YieldPtr->Ht.LevelIdx = LevelIdx;
			YieldPtr->Ht.IdxTree = *IdxTree;
			YieldPtr->Ht.IdxLeaf = *IdxLeaf;
			YieldPtr->Ht.LayerSigOut = LayerSigOut;
			Status = YieldStatus;
			goto END;
		}
		if (YieldStatus != XASUFW_SUCCESS) {
			Status = YieldStatus;
			goto END;
		}

		/**
		 * - Step 5/14: root <- xmss_pkFromSig(idxleaf, SIG_tmp, M, PK.seed, ADRS)
		 *
		 * Recover the root of the tree just signed into LayerMsg, which is the message
		 * this layer signs. Source and destination are the same buffer.
		 */
		Status = XSlhDsa_XmssPkFromSign(InstPtr, *IdxLeaf, LayerSigOut, LayerMsg,
						     LayerMsg);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		/**
		 * - Step 7: idxleaf <- idxtree mod 2^h'
		 * - Step 8: idxtree <- idxtree >> h'
		 */
		XSlhDsa_HtPeelLayerIndices(ParamPtr, IdxTree, IdxLeaf);

		/**
		 * - Step 9: ADRS.setLayerAddress(j)
		 * - Step 10: ADRS.setTreeAddress(idxtree)
		 */
		XSlhDsa_AddrSetLayer(InstPtr->Addr, LevelIdx);
		XSlhDsa_AddrSetTree(InstPtr->Addr, *IdxTree);

		/**
		 * - Step 12: SIG_HT <- SIG_HT || SIG_tmp
		 *
		 * Advance the output cursor one layer signature of (len + h') * n bytes.
		 */
		LayerSigOut += LayerBytes;

		/** - Step 11: SIG_tmp <- xmss_sign(root, SK.seed, idxleaf, PK.seed, ADRS) */
		Status = XSlhDsa_XmssEmitSig(InstPtr, LayerMsg, *IdxLeaf, LayerSigOut);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			YieldPtr->Ht.Resume = 1U;
			YieldPtr->Ht.EmitPending = 1U;
			YieldPtr->Ht.LevelIdx = LevelIdx;
			YieldPtr->Ht.IdxTree = *IdxTree;
			YieldPtr->Ht.IdxLeaf = *IdxLeaf;
			YieldPtr->Ht.LayerSigOut = LayerSigOut;
			goto END;
		}
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	YieldPtr->Ht.Resume = 0U;
	YieldPtr->Ht.EmitPending = 0U;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Recover the bottom hypertree XMSS node from SIG_HT[0] (ht_verify layer 0).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	Msg	Message digest for bottom-layer WOTS+ recovery (Pk_FORS).
* @param	SignHt	Pointer to hypertree signature buffer (layer 0).
* @param	IdxTree	Tree index for layer 0.
* @param	IdxLeaf	Leaf index for layer 0.
* @param	NodeOut	Output node buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS, if bottom-node recovery succeeds.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_ZEROIZE_MEMSET_FAIL).
*
*************************************************************************************************/
static s32 XSlhDsa_HtRecoverBottomNode(XSlhDsa_Instance *InstPtr, const u8 *Msg,
					    const u8 *SignHt, u64 IdxTree, u64 IdxLeaf,
					    u8 *NodeOut)
{
	s32 Status = XASUFW_FAILURE;

	/**
	 * - Step 1: ADRS <- toByte(0, 32)
	 *
	 * Zeroing the address selects layer 0 and clears stale fields, rebuilding the address
	 * the signer used.
	 */
	Status = Xil_SMemSet(InstPtr->Addr, sizeof(*InstPtr->Addr), 0U, sizeof(*InstPtr->Addr));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** - Step 2: ADRS.setTreeAddress(idxtree) */
	XSlhDsa_AddrSetTree(InstPtr->Addr, IdxTree);

	/**
	 * - Step 3: SIG_tmp <- SIG_HT.getXMSSSignature(0)
	 * - Step 4: node <- xmss_pkFromSig(idxleaf, SIG_tmp, M, PK.seed, ADRS)
	 */
	Status = XSlhDsa_XmssPkFromSign(InstPtr, (u32)IdxLeaf, SignHt, Msg, NodeOut);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Climb hypertree layers 1 through (d - 1) using successive XMSS signatures
*		(ht_verify upper layers).
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	ParamPtr	Active parameter set.
* @param	SignHt	In/out pointer to hypertree signature (advanced by layer size).
* @param	IdxTree	In/out tree index word.
* @param	NodeInOut	In/out node buffer (n bytes); prior root becomes next message.
*
* @return
*		- XASUFW_SUCCESS, if upper-layer climb succeeds.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
static s32 XSlhDsa_HtClimbUpperLayers(XSlhDsa_Instance *InstPtr,
					   const XSlhDsa_Param *ParamPtr, const u8 **SignHt,
					   u64 *IdxTree, u8 *NodeInOut)
{
	s32 Status = XASUFW_FAILURE;
	u32 LevelIdx;
	u32 LeafSlot;
	u32 LayerBytes = XSlhDsa_HtLayerSigBytes(ParamPtr);

	/** - Step 5: for j from 1 to d - 1 do */
	for (LevelIdx = 1; LevelIdx < ParamPtr->d; LevelIdx++) {
		/**
		 * - Step 6: idxleaf <- idxtree mod 2^h'
		 * - Step 7: idxtree <- idxtree >> h'
		 * - Step 10: SIG_tmp <- SIG_HT.getXMSSSignature(j)
		 *
		 * Stepping past the signature already consumed mirrors the ascending layer
		 * order ht_sign wrote.
		 */
		XSlhDsa_HtPeelLayerIndices(ParamPtr, IdxTree, &LeafSlot);
		*SignHt += LayerBytes;

		/**
		 * - Step 8: ADRS.setLayerAddress(j)
		 * - Step 9: ADRS.setTreeAddress(idxtree)
		 */
		XSlhDsa_AddrSetLayer(InstPtr->Addr, LevelIdx);
		XSlhDsa_AddrSetTree(InstPtr->Addr, *IdxTree);

		/**
		 * - Step 11: node <- xmss_pkFromSig(idxleaf, SIG_tmp, node, PK.seed, ADRS)
		 *
		 * The node from the layer below is the message this layer signed, and the
		 * result overwrites it, so the loop ends holding the top XMSS root.
		 */
		Status = XSlhDsa_XmssPkFromSign(InstPtr, LeafSlot, *SignHt, NodeInOut,
						    NodeInOut);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Validate hypertree geometry, that a tree/leaf index pair fits it, and that
*		the hypertree signature section fits the registered signature buffer
*		(ht_verify Steps 1 to 4 precondition).
*
* @param	InstPtr	Pointer to SLH-DSA instance with the registered signature buffer.
* @param	ParamPtr	Active parameter set.
* @param	IdxTree	Tree index to check against the (h - h')-bit bottom-layer field.
* @param	IdxLeaf	Leaf index to check against the h'-bit leaf field.
* @param	SignHt	Pointer to the start of the hypertree signature section.
*
* @return
*		- XASUFW_SUCCESS, if the geometry is sane, both indices fit it, and the d
*		  layer signatures starting at SignHt fit the registered signature buffer.
*		- XASUFW_SLHDSA_INVALID_PARAM, otherwise.
*
*************************************************************************************************/
static s32 XSlhDsa_HtCheckIndices(const XSlhDsa_Instance *InstPtr,
			const XSlhDsa_Param *ParamPtr, u64 IdxTree, u64 IdxLeaf,
			const u8 *SignHt)
{
	s32 Status = XASUFW_SLHDSA_INVALID_PARAM;
	u64 SignAddr = (u64)(UINTPTR)SignHt;
	u64 HtBytes;

	/** Reject a geometry that cannot be shifted below without underflow or UB. */
	if ((ParamPtr->xmss_height == 0U) || (ParamPtr->xmss_height >= XASUFW_BITS_IN_U64) ||
			(ParamPtr->d == 0U) || (ParamPtr->h < ParamPtr->xmss_height)) {
		goto END;
	}

	/** Reject a tree index outside the bottom-layer field, skipping a full-width field. */
	if (((ParamPtr->h - ParamPtr->xmss_height) < XASUFW_BITS_IN_U64) &&
			(IdxTree >= ((u64)1U << (ParamPtr->h - ParamPtr->xmss_height)))) {
		goto END;
	}

	/** Reject a leaf index outside the one XMSS subtree it is meant to address. */
	if (IdxLeaf >= ((u64)1U << ParamPtr->xmss_height)) {
		goto END;
	}

	/** Reject a hypertree section that would read past the registered signature buffer. */
	HtBytes = (u64)ParamPtr->d * (u64)XSlhDsa_HtLayerSigBytes(ParamPtr);
	if ((InstPtr->SigAddr == 0U) || (SignAddr < InstPtr->SigAddr) ||
			((SignAddr + HtBytes) > (InstPtr->SigAddr + (u64)InstPtr->SigLen))) {
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compare a recovered hypertree root against the instance public-key root.
*
* Uses a constant-time compare instead of a short-circuit memcmp.
*
* @param	RecoveredRoot	Recovered root buffer (n bytes).
* @param	PkRoot	Expected public-key root (n bytes).
* @param	HashWidth	Hash width n in bytes.
*
* @return
*		- XASUFW_SUCCESS, if the roots match.
*		- XASUFW_SLHDSA_VERIFY_ROOT_MISMATCH, if the roots differ.
*
* @note
*	XSlhDsa_HtVerify calls this twice, independently, and raises the global ReturnStatus
*	verified indicator only after both calls agree; this function does not raise that
*	indicator itself, so a single call here can never bypass the double-compare contract.
*
*************************************************************************************************/
static s32 XSlhDsa_HtRootMatchesPk(const u8 *RecoveredRoot, const u8 *PkRoot, u32 HashWidth)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihRoot = XFih_VolatileAssignXfihVar(XFIH_FAILURE);

	/**
	 * - Step 13: if node == PK.root then return true, else return false
	 *
	 * The compare runs over the whole width, so the position of a mismatch does not leak.
	 */
	XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Xil_SMemCmp_CT, XASUFW_SLHDSA_VERIFY_ROOT_MISMATCH,
					   XFihRoot, Status, END, RecoveredRoot, HashWidth,
					   PkRoot, HashWidth, HashWidth);

END:
	return Status;
}
#endif /* XASU_SLHDSA_ENABLE */
/** @} */
