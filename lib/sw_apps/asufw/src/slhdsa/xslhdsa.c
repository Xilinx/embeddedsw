/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xslhdsa.c
*
* This file contains the SLH-DSA instance management and top-level operation
* implementation for ASUFW firmware.
*
* Covers the instance state and parameter-set binding, cooperative yield for long-running
* key generation and signing, the HashSLH-DSA pre-hash message preparation, and the key
* generation, signature generation and signature verification entry points.
*
* Requests arrive through XSlhDsa_KeyGen, XSlhDsa_Sign and XSlhDsa_Verify, which validate the
* client parameters, bind the instance to the requested parameter set, draw the randomness the
* operation needs from the TRNG and resolve the request buffers, then run the FIPS 205 algorithm
* through the corresponding XSlhDsa_*Internal function. Key generation then signs an empty
* message with the new secret key and verifies it with the new public key before the pair is
* exported (FIPS 140-3 IG 10.3.A pairwise consistency test).
*
* Implements FIPS 205 Algorithms 18 and 21 (slh_keygen), 19, 22 and 23 (slh_sign and
* hash_slh_sign), and 20, 24 and 25 (slh_verify and hash_slh_verify).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   kk   07/10/26 Initial release
*       kk   09/18/26 Size a yield slice by leaves built rather than by elapsed time
*       kk   09/23/26 Run a pairwise consistency test on every generated key pair
*       kk   09/29/26 Staged sign and verify signatures in the signature scratch and reused the
*                     caller's keygen parameters on every slice
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
#include "xasufw_perf.h"
#include "xasufw_util.h"
#include "xasufw_dma.h"
#include "xfih.h"
#include "xtrng.h"
#include "xasu_slhdsainfo.h"
#include "xasu_slhdsa_common.h"
#include "xslhdsa.h"
#include "xslhdsa_core.h"
#include "xslhdsa_hash.h"

#ifdef XASU_SLHDSA_ENABLE
/************************************ Constant Definitions ***************************************/
/*
 * The staged key region is bound to the pool reserved in lscript.ld, so widening it past what the
 * pool holds fails the build rather than silently running into the auxiliary buffer above it. It is
 * also bound to the largest key the client interface admits, so a parameter set whose SK the region
 * cannot hold fails the build rather than being rejected at run time as an oversized copy.
 */
_Static_assert((XSLHDSA_SCRATCH_END - XASUFW_CRYPTO_ALGO_DATA_BLOCK) <=
	       XASUFW_CRYPTO_SHARED_POOL_SIZE,
	       "SLH-DSA scratch region overruns the shared crypto pool reserved in lscript.ld");
_Static_assert(XSLHDSA_SCRATCH_LEN >= XASU_SLHDSA_MAX_SK_SIZE,
	       "SLH-DSA staged key region cannot hold the largest secret key in xasu_slhdsainfo.h");
_Static_assert(XASU_SLHDSA_SHAKE_256S_SIG_SIZE == XASU_SLHDSA_SHA2_256S_SIG_SIZE,
	       "Signature scratch must cover both 256s parameter sets");
_Static_assert(XSLHDSA_SIG_SCRATCH_BYTES >= XASU_SLHDSA_SHAKE_256S_SIG_SIZE,
	       "Signature scratch cannot hold the largest SLH-DSA signature");

/** DER OID for SHA-256, id-sha256 (FIPS 205 Algorithm 23, step 10): 2.16.840.1.101.3.4.2.1. */
static const u8 XSlhDsa_PreHashOidSha256[XASU_SLHDSA_PREHASH_OID_LEN] = {
	0x06U, 0x09U, 0x60U, 0x86U, 0x48U, 0x01U, 0x65U, 0x03U, 0x04U, 0x02U, 0x01U
};

/** DER OID for SHA-512, id-sha512 (FIPS 205 Algorithm 23, step 13): 2.16.840.1.101.3.4.2.3. */
static const u8 XSlhDsa_PreHashOidSha512[XASU_SLHDSA_PREHASH_OID_LEN] = {
	0x06U, 0x09U, 0x60U, 0x86U, 0x48U, 0x01U, 0x65U, 0x03U, 0x04U, 0x02U, 0x03U
};

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static void XSlhDsa_PublishStagedSkViews(XSlhDsa_Instance *InstPtr, u32 HashWidth);
static s32 XSlhDsa_KeyWriteSkPrefix(const u8 *SkSeed, const u8 *SkPrf,
					  const u8 *PkSeed, u32 HashWidth, u8 *SkOut);
static s32 XSlhDsa_KeyDrawSkPrefix(XSlhDsa_Instance *InstPtr, u32 HashWidth);
static s32 XSlhDsa_KeyBuildTopXmssRoot(XSlhDsa_Instance *InstPtr,
					    const XSlhDsa_Param *ParamPtr, u8 *RootOut);
static s32 XSlhDsa_KeyCommitHypertreeRoot(XSlhDsa_Instance *InstPtr,
					       const u8 *Root, u32 HashWidth);

static s32 XSlhDsa_SignDigestBody(XSlhDsa_Instance *InstPtr, const u8 *Digest,
				       u8 *SigBody);

static s32 XSlhDsa_ParamSetFromAsu(u8 ParamSet, XSlhDsa_ParamSet *TypePtr);
static s32 XSlhDsa_PrepareInstance(XSlhDsa_Instance *InstPtr, u8 ParamSet, XAsufw_Dma *DmaPtr,
					XSha *ShaPtr);
static s32 XSlhDsa_Pwct(XSlhDsa_Instance *InstPtr);

/************************************ Variable Definitions ***************************************/
/**
 * Signature scratch in overlay data, sized for the largest supported signature. Signing and the
 * pairwise consistency test build the signature here, and verification reads the client signature
 * from here.
 */
static u8 XSlhDsa_SigScratch[XSLHDSA_SIG_SCRATCH_BYTES] __attribute__((aligned(4)));

/** Non-NULL empty-message pointer; SignInternal rejects a NULL Msg even at MsgLen 0. */
static const u8 XSlhDsa_PwctEmptyMsg[1U] = { 0U };

/**
* FIPS 205 Table 2 parameter sets indexed by XSlhDsa_ParamSet.
*
* Each row stores the fixed constants (n, h, d, h', a, k, lg_w, m) for one supported
* SLH-DSA parameter set. Entries are selected at runtime by XSlhDsa_LookupParamSet()
* and copied into the instance by XSlhDsa_BindParam(). The reserved row is not a
* FIPS set; LookupParamSet primes the output to it until a supported identifier is
* confirmed.
*
* The two yield budgets are not FIPS constants. They set how many XMSS or FORS leaves a
* cooperative yield slice builds before the command parks, and are sized from the measured
* leaf cost of each set so a slice stays near 10 ms and well inside the keep-alive interval.
*/
static const XSlhDsa_Param XSlhDsa_ParamLookup[XSLHDSA_PARAM_SET_MAX + 1U] = {
	/* Dummy row: unused hash family and zero sizes, not a FIPS 205 parameter set. */
	[XSLHDSA_PARAM_IDX_RESERVED] = {
		.hash_family = XSLHDSA_HASH_UNSUPPORTED,
		.n = 0U,
		.h = 0U,
		.d = 0U,
		.xmss_height = 0U,
		.fors_height = 0U,
		.fors_count = 0U,
		.wots_win_bits = 0U,
		.m = 0U,
		.yield_xmss_leaves = 0U,
		.yield_fors_leaves = 0U,
	},
	/* SLH-DSA-SHAKE-256s: n=32, h=64, d=8, h'=8, a=14, k=22, lg_w=4, m=47 */
	[XSLHDSA_PARAM_SET_SHAKE_256S] = {
		.hash_family = XSLHDSA_HASH_SHAKE,
		.n = 32,
		.h = 64,
		.d = 8,
		.xmss_height = 8,
		.fors_height = 14,
		.fors_count = 22,
		.wots_win_bits = 4,
		.m = 47,
		.yield_xmss_leaves = 1U,
		.yield_fors_leaves = 300U,
	},
	/* SLH-DSA-SHA2-128s: n=16, h=63, d=7, h'=9, a=12, k=14, lg_w=4, m=30 */
	[XSLHDSA_PARAM_SET_SHA2_128S] = {
		.hash_family = XSLHDSA_HASH_SHA2,
		.n = 16,
		.h = 63,
		.d = 7,
		.xmss_height = 9,
		.fors_height = 12,
		.fors_count = 14,
		.wots_win_bits = 4,
		.m = 30,
		.yield_xmss_leaves = 1U,
		.yield_fors_leaves = 200U,
	},
	/* SLH-DSA-SHA2-256s: n=32, h=64, d=8, h'=8, a=14, k=22, lg_w=4, m=47 */
	[XSLHDSA_PARAM_SET_SHA2_256S] = {
		.hash_family = XSLHDSA_HASH_SHA2,
		.n = 32,
		.h = 64,
		.d = 8,
		.xmss_height = 8,
		.fors_height = 14,
		.fors_count = 22,
		.wots_win_bits = 4,
		.m = 47,
		.yield_xmss_leaves = 1U,
		.yield_fors_leaves = 200U,
	},
	/* SLH-DSA-SHAKE-128s: n=16, h=63, d=7, h'=9, a=12, k=14, lg_w=4, m=30 */
	[XSLHDSA_PARAM_SET_SHAKE_128S] = {
		.hash_family = XSLHDSA_HASH_SHAKE,
		.n = 16,
		.h = 63,
		.d = 7,
		.xmss_height = 9,
		.fors_height = 12,
		.fors_count = 14,
		.wots_win_bits = 4,
		.m = 30,
		.yield_xmss_leaves = 2U,
		.yield_fors_leaves = 300U,
	},
};

/*
 * Compile-time node stack sizing. A treehash push at depth d writes NodeStack[d], and a
 * walk over 2^SubHeight leaves reaches d == SubHeight - 1, so a depth greater than the
 * largest SubHeight leaves one spare slot. XMSS SubHeight can equal h' (full-tree keygen),
 * while FORS auth-path SubHeight is LevelIdx in 0..(a - 1).
 */
_Static_assert(XSLHDSA_XMSS_STACK_DEPTH > XSLHDSA_MAX_XMSS_HEIGHT,
	       "XMSS node stack depth must exceed max h'");
_Static_assert(XSLHDSA_FORS_STACK_DEPTH > (XSLHDSA_MAX_FORS_HEIGHT - 1U),
	       "FORS node stack depth must exceed max (a - 1)");

/*************************************************************************************************/
/**
* @brief	This function returns the parameter descriptor for an SLH-DSA parameter set.
*
* Resolves a supported parameter-set identifier to a const pointer into the static
* XSlhDsa_ParamLookup table. A volatile temporary copy of ParamSet is compared
* against the caller value after the switch to detect a skipped or glitched case.
*
* @param	ParamSet	SLH-DSA parameter set identifier.
* @param	ParamPtrPtr	Output pointer updated with the lookup-table entry on success.
*
* @return
*		- XASUFW_SUCCESS if ParamSet is supported.
*		- XASUFW_SLHDSA_INVALID_PARAM if ParamPtrPtr is NULL.
*		- XASUFW_SLHDSA_UNSUPPORTED_PARAM_SET if ParamSet is outside the four supported
*		  sets.
*		- XASUFW_SLHDSA_LOOKUP_GLITCH_ERROR if the redundant parameter-set
*		  comparison after the switch does not match.
*
* @note
*	ParamPtrPtr is primed to the reserved dummy row before validation completes.
*
*************************************************************************************************/
s32 XSlhDsa_LookupParamSet(XSlhDsa_ParamSet ParamSet,
				const XSlhDsa_Param **ParamPtrPtr)
{
	s32 Status = XASUFW_FAILURE;
	volatile XSlhDsa_ParamSet TmpParamSet = (XSlhDsa_ParamSet)XASUFW_ALLFS;

	/** Validate input pointer. */
	if (ParamPtrPtr == NULL) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/* Default to the reserved dummy until a supported ParamSet is confirmed. */
	*ParamPtrPtr = &XSlhDsa_ParamLookup[XSLHDSA_PARAM_IDX_RESERVED];

	/** Resolve the identifier to its Table 2 row, recording the case in TmpParamSet. */
	switch (ParamSet) {
		case XSLHDSA_PARAM_SET_SHAKE_256S: {
			*ParamPtrPtr =
				&XSlhDsa_ParamLookup[XSLHDSA_PARAM_SET_SHAKE_256S];
			TmpParamSet = XSLHDSA_PARAM_SET_SHAKE_256S;
			break;
		}
		case XSLHDSA_PARAM_SET_SHA2_128S: {
			*ParamPtrPtr = &XSlhDsa_ParamLookup[XSLHDSA_PARAM_SET_SHA2_128S];
			TmpParamSet = XSLHDSA_PARAM_SET_SHA2_128S;
			break;
		}
		case XSLHDSA_PARAM_SET_SHA2_256S: {
			*ParamPtrPtr = &XSlhDsa_ParamLookup[XSLHDSA_PARAM_SET_SHA2_256S];
			TmpParamSet = XSLHDSA_PARAM_SET_SHA2_256S;
			break;
		}
		case XSLHDSA_PARAM_SET_SHAKE_128S: {
			*ParamPtrPtr =
				&XSlhDsa_ParamLookup[XSLHDSA_PARAM_SET_SHAKE_128S];
			TmpParamSet = XSLHDSA_PARAM_SET_SHAKE_128S;
			break;
		}
		default: {
			/** - Reject identifiers outside the four supported parameter sets. */
			*ParamPtrPtr = &XSlhDsa_ParamLookup[XSLHDSA_PARAM_IDX_RESERVED];
			Status = XASUFW_SLHDSA_UNSUPPORTED_PARAM_SET;
			goto END;
		}
	}

	/**
	 * Confirm the resolved entry matches the requested parameter set. Only the matching case
	 * writes TmpParamSet, so a mismatch means the switch was skipped or glitched.
	 */
	if (TmpParamSet != ParamSet) {
		Status = XASUFW_SLHDSA_LOOKUP_GLITCH_ERROR;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	This function returns the WOTS+ message-chain count (FIPS 205 len_1, Eq. 5.2).
*
* @param	ParamPtr	Pointer to parameter descriptor.
*
* @return
*	len_1 value for parameter set ParamPtr.
*
*************************************************************************************************/
u32 XSlhDsa_WotsMsgChainCount(const XSlhDsa_Param *ParamPtr)
{
	/** Compute len_1 = ceil(8n / lg_w) using integer ceiling division. */
	return (((XASUFW_BYTE_LEN_IN_BITS * ParamPtr->n) + ParamPtr->wots_win_bits - 1U) /
		ParamPtr->wots_win_bits);
}
/*************************************************************************************************/
/**
* @brief	This function returns the WOTS+ checksum-chain count
*		(FIPS 205 len_2, Alg. 1 / Eq. 5.3).
*
* @param	ParamPtr	Pointer to parameter descriptor.
*
* @return
*	len_2 value for parameter set ParamPtr.
*
* @note
*	Shortcuts the Algorithm 1 loop when lg_w = 4 and 9 <= n <= 136, a range over which
*	len_2 is constant at 3. The bound is derived from Eq. 5.3, not stated in FIPS 205.
*
*************************************************************************************************/
u32 XSlhDsa_WotsCsChainCount(const XSlhDsa_Param *ParamPtr)
{
	u32 Len2;
	u32 Cap;
	u32 Maxs;

	/* For lg_w = XSLHDSA_WOTS_LG_W and n in [XSLHDSA_WOTS_LEN2_FAST_N_MIN,
	 * XSLHDSA_WOTS_LEN2_FAST_N_MAX], Eq. 5.3 always yields
	 * len_2 = XSLHDSA_WOTS_LEN2_FAST, so the Algorithm 1 search can be skipped. */
	if ((ParamPtr->wots_win_bits == XSLHDSA_WOTS_LG_W) &&
	    (ParamPtr->n >= XSLHDSA_WOTS_LEN2_FAST_N_MIN) &&
	    (ParamPtr->n <= XSLHDSA_WOTS_LEN2_FAST_N_MAX)) {
		Len2 = XSLHDSA_WOTS_LEN2_FAST;
		goto END;
	}

	/** General path: Maxs is the largest possible checksum, len_1 * (w - 1) for w = 2^lg_w. */
	Maxs = XSlhDsa_WotsMsgChainCount(ParamPtr) *
	 XSLHDSA_BIT_MASK(ParamPtr->wots_win_bits);
	Len2 = 1U;
	Cap = 1U << ParamPtr->wots_win_bits;

	/** Grow len_2 until Cap = w^len_2 covers Maxs, avoiding the Eq. 5.3 logarithm. */
	while (Cap < Maxs) {
		Len2++;
		Cap <<= ParamPtr->wots_win_bits;
	}

END:
	return Len2;
}
/*************************************************************************************************/
/**
* @brief	This function returns the total WOTS+ chain count (FIPS 205 len = len_1 + len_2).
*
* @param	ParamPtr	Pointer to parameter descriptor.
*
* @return
*	Total WOTS+ chain count for parameter set ParamPtr.
*
*************************************************************************************************/
u32 XSlhDsa_WotsChainCount(const XSlhDsa_Param *ParamPtr)
{
	return XSlhDsa_WotsMsgChainCount(ParamPtr) + XSlhDsa_WotsCsChainCount(ParamPtr);
}
/*************************************************************************************************/
/**
* @brief	This function returns the SLH-DSA signature size in bytes for a parameter set.
*
* @param	ParamPtr	Pointer to parameter descriptor.
*
* @return
*	(1 + k * (1 + a) + h + d * len) * n bytes per FIPS 205 signature encoding.
*
* @note
*	len is the total WOTS+ chain count from XSlhDsa_WotsChainCount().
*
*************************************************************************************************/
u32 XSlhDsa_SignatureBytes(const XSlhDsa_Param *ParamPtr)
{
	u32 WotsChains = XSlhDsa_WotsChainCount(ParamPtr);

	/**
	 * Sum the n-byte blocks of the signature encoding: randomizer R, the FORS signature
	 * (k private values plus k auth paths of a nodes), and the hypertree signature
	 * (h auth-path nodes plus one len-block WOTS+ signature per layer, over d layers).
	 */
	return (1U + (ParamPtr->fors_count * (1U + ParamPtr->fors_height)) + ParamPtr->h +
					(ParamPtr->d * WotsChains)) *
				 ParamPtr->n;
}
/*************************************************************************************************/
/**
* @brief	This function returns the singleton SLH-DSA instance.
*
* Lazily initializes static scratch buffers, the domain-separation address, and binds
* the default SLH-DSA-SHAKE-256s parameter set on first use.
*
* @return
*		- Pointer to the initialized SLH-DSA instance on success.
*		- NULL, if the default parameter set could not be bound.
*
* @note
*	The instance and its scratch buffers are allocated statically on first call rather than on
*	the stack. WorkMem alone is XSLHDSA_WORK_MEM_BYTES and call paths also place large
*	XMSS/FORS node stacks on the stack, which together exceed the ASUFW stack budget. A
*	stack-backed instance would overflow. Key material is the exception: it is staged in the
*	shared crypto pool that KeyScratch points at, not in these static buffers.
*
*************************************************************************************************/
XSlhDsa_Instance *XSlhDsa_GetInstance(void)
{
	static XSlhDsa_Adrs Adrs;
	static u8 WorkMem[XSLHDSA_WORK_MEM_BYTES];
	static u8 MsgDigest[XSLHDSA_MSG_DIGEST_BYTES];
	static u8 NodeBuf[XSLHDSA_NODE_BUF_BYTES];
	static u32 WotsWeights[XSLHDSA_WOTS_WEIGHT_WORDS];
	static XSlhDsa_Instance SlhDsaInst;
	static u8 Ready;
	XSlhDsa_Instance *InstPtr = NULL;

	/** Wire the singleton once, on the first call. */
	if (Ready == 0U) {
		/**
		 * Bind the static ADRS and scratch buffers. These pointers are owned here and
		 * stay valid for the lifetime of the firmware, so callers must not replace or
		 * release them; only the contents are reused and zeroized per operation. Sizes
		 * are the worst case across parameter sets, so a rebind never resizes them.
		 */
		SlhDsaInst.Addr = &Adrs;
		SlhDsaInst.WorkMem = WorkMem;
		SlhDsaInst.MsgDigest = MsgDigest;
		SlhDsaInst.NodeBuf = NodeBuf;
		SlhDsaInst.WotsWeights = WotsWeights;
		/*
		 * The staged key region is not owned here the way the buffers above are: the pool
		 * it points into belongs to whichever command holds XASUFW_OVERLAY, so only its
		 * address is fixed for the lifetime of the firmware. Its contents are the current
		 * command's, which is why the key views stay NULL until a load stages one and
		 * XSlhDsa_ClearSensitiveState() wipes it before the command releases the overlay.
		 */
		SlhDsaInst.KeyScratch = (u8 *)(UINTPTR)XSLHDSA_SCRATCH_ADDR;
		/* Default to SLH-DSA-SHAKE-256s until a request binds the set it asked for. */
		if (XSlhDsa_BindParam(&SlhDsaInst, XSLHDSA_PARAM_SET_SHAKE_256S) !=
				XASUFW_SUCCESS)
		{
			goto END;
		}
		/**
		 * Mark ready only after the default set and hash backend are bound, so a failed
		 * bind returns NULL and lets the next call retry instead of handing out an
		 * instance with no hash ops table.
		 */
		Ready = 1U;
	}

	InstPtr = &SlhDsaInst;

END:
	return InstPtr;
}
/*************************************************************************************************/
/**
* @brief	This function binds a parameter set to an SLH-DSA instance.
*
* Looks up FIPS 205 Table 2 constants for ParamSet, copies them into InstPtr->Param,
* and binds the hash ops table that matches the selected hash family.
*
* @param	InstPtr	Pointer to the SLH-DSA instance.
* @param	ParamSet	SLH-DSA parameter set identifier.
*
* @return
*		- XASUFW_SUCCESS, if the parameter set and hash backend were bound.
*		- XASUFW_SLHDSA_INVALID_PARAM, if InstPtr is NULL.
*		- XASUFW_SLHDSA_UNSUPPORTED_PARAM_SET, if ParamSet is unsupported or its hash ops
*		  table could not be resolved.
*		- XASUFW_SLHDSA_LOOKUP_GLITCH_ERROR, if the redundant parameter-set
*		  check during lookup does not match.
*
*************************************************************************************************/
s32 XSlhDsa_BindParam(XSlhDsa_Instance *InstPtr, XSlhDsa_ParamSet ParamSet)
{
	s32 Status = XASUFW_FAILURE;
	const XSlhDsa_Param *ParamPtr;

	/** Validate input pointer. */
	if (InstPtr == NULL) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/* Resolve Table 2 constants for the requested parameter set. */
	Status = XSlhDsa_LookupParamSet(ParamSet, &ParamPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * Copy the row by value so the instance owns its view of the supplied constants
	 * (n, h, d, h', a, k, lg_w, m) and the cooperative yield leaf budgets, and the lookup
	 * table stays immutable. Derived quantities (len_1, len_2, len, signature size) are
	 * recomputed on demand, not cached here.
	 */
	InstPtr->Param = *ParamPtr;

	/**
	 * Bind the backend named by hash_family so F/H/T_l, PRF and H_msg route to the SHA2 or
	 * SHAKE instantiation. This also fixes the ADRS form those calls use: SHA2 compresses
	 * InstPtr->Addr to 22 bytes (ADRS^c), SHAKE absorbs the full 32-byte ADRS.
	 */
	InstPtr->HashOps = XSlhDsa_HashOpsLookup(InstPtr->Param.hash_family);

	/* An unresolved table means no backend is built in for this hash family. */
	if (InstPtr->HashOps == NULL) {
		Status = XASUFW_SLHDSA_UNSUPPORTED_PARAM_SET;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	This function stores the message buffer address and length in the instance.
*
* @param	InstPtr	Pointer to the SLH-DSA instance.
* @param	Msg	Pointer to the message buffer.
* @param	MsgLen	Length of the message in bytes.
*
* @note
*	The message address is stored as a 64-bit DMA address.
*
*************************************************************************************************/
void XSlhDsa_SetMsg(XSlhDsa_Instance *InstPtr, const u8 *Msg,
			 u32 MsgLen)
{
	InstPtr->MsgAddr = (u64)(UINTPTR)Msg;
	InstPtr->MsgLen = MsgLen;
}
/*************************************************************************************************/
/**
* @brief	This function stores the signature buffer address and length in the instance.
*
* @param	InstPtr	Pointer to the SLH-DSA instance.
* @param	Sig	Pointer to the signature buffer.
* @param	SigLen	Length of the signature buffer in bytes.
*
* @note
*	The signature address is stored as a 64-bit DMA address.
*
*************************************************************************************************/
void XSlhDsa_SetSig(XSlhDsa_Instance *InstPtr, const u8 *Sig,
			 u32 SigLen)
{
	InstPtr->SigAddr = (u64)(UINTPTR)Sig;
	InstPtr->SigLen = SigLen;
}
/*************************************************************************************************/
/**
* @brief	This function stores the public key buffer address in the instance.
*
* @param	InstPtr	Pointer to the SLH-DSA instance.
* @param	Pk	Pointer to the public key buffer.
*
* @note
*	The public key address is stored as a 64-bit DMA address.
*
*************************************************************************************************/
void XSlhDsa_SetPkAddr(XSlhDsa_Instance *InstPtr, const u8 *Pk)
{
	InstPtr->PkAddr = (u64)(UINTPTR)Pk;
}
/*************************************************************************************************/
/**
* @brief	This function stores HashSLH-DSA message mode fields on the instance.
*
* @param	InstPtr	Pointer to the SLH-DSA instance.
* @param	PreHashMode	XASU_SLHDSA_MSG_PURE or XASU_SLHDSA_MSG_PREHASH.
* @param	HashId	Pre-hash function identifier.
*
* @note
*	Clears MsgLocalPtr when mode fields are updated.
*
*************************************************************************************************/
void XSlhDsa_SetMsgMode(XSlhDsa_Instance *InstPtr, u8 PreHashMode, u8 HashId)
{
	/** Validate input pointer. */
	if (InstPtr == NULL) {
		goto END;
	}

	/**
	 * Record the mode and, for the pre-hash variant, the pre-hash function whose OID goes
	 * into the M' prefix. Drop any PH(M) buffer left by an earlier command so the new
	 * command cannot consume a stale digest pointer.
	 */
	InstPtr->PreHashMode = PreHashMode;
	InstPtr->HashId = HashId;
	InstPtr->MsgLocalPtr = NULL;
	InstPtr->MsgLocalLen = 0U;

END:
	return;
}
/*************************************************************************************************/
/**
* @brief	This function loads a public key into the SLH-DSA instance.
*
* @param	InstPtr	Pointer to the SLH-DSA instance.
* @param	Pk	Pointer to the public key (seed || root).
*
* @return
*		- XASUFW_SUCCESS, if the public key is loaded.
*		- XASUFW_SLHDSA_INVALID_PARAM, if InstPtr, its staged key region or Pk is NULL.
*		- XASUFW_SLHDSA_COPY_FAILED, if staging the key fails.
*
* @note
*	Stages PK in the shared crypto pool and points PkSeed and PkRoot at it. SkSeed and SkPrf are
*	left NULL, since verification reads neither and must not see a secret key an earlier command
*	staged.
*
*************************************************************************************************/
s32 XSlhDsa_LoadPk(XSlhDsa_Instance *InstPtr, const u8 *Pk)
{
	s32 Status = XASUFW_FAILURE;
	u8 *KeyPtr;
	u32 HashWidth;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (InstPtr->KeyScratch == NULL) || (Pk == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Field width is n, so XSlhDsa_BindParam() must have run before this call. */
	HashWidth = InstPtr->Param.n;
	KeyPtr = InstPtr->KeyScratch;

	/** Clear key pointers before copying the public key into scratch. */
	InstPtr->PkSeed = NULL;
	InstPtr->PkRoot = NULL;
	InstPtr->SkSeed = NULL;
	InstPtr->SkPrf = NULL;

	/** Copy PK.seed || PK.root into the scratch region. */
	Status = Xil_SMemCpy(KeyPtr, XSLHDSA_SCRATCH_LEN, Pk,
				     XSLHDSA_PK_N_FIELDS * HashWidth,
				     XSLHDSA_PK_N_FIELDS * HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_COPY_FAILED;
		goto END;
	}

	/** Publish the public-key views onto the stable firmware-owned key image. */
	InstPtr->PkSeed = &KeyPtr[XSLHDSA_PK_SEED_FIELD * HashWidth];
	InstPtr->PkRoot = &KeyPtr[XSLHDSA_PK_ROOT_FIELD * HashWidth];

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	This function loads a secret key into the SLH-DSA instance.
*
* @param	InstPtr	Pointer to the SLH-DSA instance.
* @param	Sk	Pointer to the secret key (seed || prf || pk_seed || pk_root).
*
* @return
*		- XASUFW_SUCCESS, if the secret key is loaded.
*		- XASUFW_SLHDSA_INVALID_PARAM, if InstPtr, its staged key region or Sk is NULL.
*		- XASUFW_SLHDSA_COPY_FAILED, if staging the key fails.
*
* @note
*	Stages SK in the shared crypto pool and points SkSeed, SkPrf, PkSeed and PkRoot at its four
*	fields. A failed stage leaves all four NULL rather than pointing at the previous key.
*
*************************************************************************************************/
s32 XSlhDsa_LoadSk(XSlhDsa_Instance *InstPtr, const u8 *Sk)
{
	s32 Status = XASUFW_FAILURE;
	u8 *KeyPtr;
	u32 HashWidth;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (InstPtr->KeyScratch == NULL) || (Sk == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * The private key is four consecutive n-byte fields, so XSlhDsa_BindParam() must have run
	 * before this call.
	 */
	HashWidth = InstPtr->Param.n;
	KeyPtr = InstPtr->KeyScratch;

	/**
	 * Drop the views up front, so a failed stage below leaves none of them pointing into a
	 * region that holds a mix of this key and the last one.
	 */
	InstPtr->SkSeed = NULL;
	InstPtr->SkPrf = NULL;
	InstPtr->PkSeed = NULL;
	InstPtr->PkRoot = NULL;

	/**
	 * Take the firmware's own copy of SK.seed || SK.prf || PK.seed || PK.root. The four fields
	 * are n wide and consecutive in the caller buffer and in the staged copy alike, so one copy
	 * stages all of them. XSLHDSA_SCRATCH_LEN bounds the write, so a parameter set whose
	 * key does not fit the region is rejected instead of running past it.
	 */
	Status = Xil_SMemCpy(KeyPtr, XSLHDSA_SCRATCH_LEN, Sk,
				     XSLHDSA_SK_N_FIELDS * HashWidth,
				     XSLHDSA_SK_N_FIELDS * HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_COPY_FAILED;
		goto END;
	}

	/** Publish the views onto the staged key now that it holds the caller's key in full. */
	XSlhDsa_PublishStagedSkViews(InstPtr, HashWidth);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	This function clears the staged key material and the instance scratch buffers.
*
* @param	InstPtr	Pointer to the SLH-DSA instance.
*
* @return
*		- XASUFW_SUCCESS if all clears succeed.
*		- XASUFW_SLHDSA_INVALID_PARAM if InstPtr is NULL.
*		- Combined buffer-clear failure status via XAsufw_UpdateBufStatus otherwise.
*
* @note
*	Also resets external buffer addresses, lengths, and the context prefix length.
*	Callers should fold the return value into the command status with
*	XAsufw_UpdateBufStatus() so a failed wipe is reported.
*
*************************************************************************************************/
s32 XSlhDsa_ClearSensitiveState(XSlhDsa_Instance *InstPtr)
{
	s32 Status = XASUFW_SUCCESS;

	/** Validate input pointer. */
	if (InstPtr == NULL) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * Zeroize the staged SK.seed, SK.prf, PK.seed and PK.root, then drop the views onto them so
	 * nothing reads the region after the wipe. This is also what hands the shared pool back to
	 * the next command, so it has to happen before the overlay is released. The whole region is
	 * cleared rather than the 4n bytes the bound parameter set staged, so residue from a wider
	 * set staged earlier cannot survive; the NULL guard covers an instance never wired by
	 * XSlhDsa_GetInstance(). Clears are folded into Status rather than aborting on the first
	 * failure, so the remaining secrets are still wiped and the failure is still reported.
	 */
	if (InstPtr->KeyScratch != NULL) {
		Status = XAsufw_UpdateBufStatus(Status,
				Xil_SecureZeroize(InstPtr->KeyScratch,
						    XSLHDSA_SCRATCH_LEN));
	}

	InstPtr->SkSeed = NULL;
	InstPtr->SkPrf = NULL;
	InstPtr->PkSeed = NULL;
	InstPtr->PkRoot = NULL;

	/**
	 * Zeroize the scratch buffers, which hold WOTS+ and FORS private values and message
	 * digest material derived from SK.seed and are therefore as sensitive as the staged key.
	 * The singleton shares them across commands, so each is cleared to its declared size;
	 * the NULL guards cover an instance never wired by XSlhDsa_GetInstance().
	 */
	if (InstPtr->WorkMem != NULL) {
		Status = XAsufw_UpdateBufStatus(Status,
				Xil_SecureZeroize(InstPtr->WorkMem,
						    XSLHDSA_WORK_MEM_BYTES));
	}

	if (InstPtr->MsgDigest != NULL) {
		Status = XAsufw_UpdateBufStatus(Status,
				Xil_SecureZeroize(InstPtr->MsgDigest,
						    XSLHDSA_MSG_DIGEST_BYTES));
	}

	if (InstPtr->NodeBuf != NULL) {
		Status = XAsufw_UpdateBufStatus(Status,
				Xil_SecureZeroize(InstPtr->NodeBuf,
						    XSLHDSA_NODE_BUF_BYTES));
	}

	if (InstPtr->WotsWeights != NULL) {
		Status = XAsufw_UpdateBufStatus(Status,
				Xil_SecureZeroize((u8 *)InstPtr->WotsWeights,
					XSLHDSA_WOTS_WEIGHT_WORDS * sizeof(u32)));
	}

	/**
	 * Reset the per-operation bookkeeping so the next command cannot inherit the previous
	 * message mode, context-prefix length or buffer descriptors. The message, signature and
	 * public-key buffers belong to the caller, so only the cached addresses and lengths are
	 * dropped; the Addr and scratch pointers from XSlhDsa_GetInstance() stay installed.
	 */
	InstPtr->CtxPrefixLen = 0U;
	InstPtr->PreHashMode = 0U;
	InstPtr->HashId = 0U;
	InstPtr->MsgLocalPtr = NULL;
	InstPtr->MsgLocalLen = 0U;
	InstPtr->MsgAddr = 0U;
	InstPtr->MsgLen = 0U;
	InstPtr->SigAddr = 0U;
	InstPtr->SigLen = 0U;
	InstPtr->PkAddr = 0U;

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function zeroizes the signature scratch.
*
* @return
*		- XASUFW_SUCCESS if the buffer is cleared.
*		- A buffer-clear failure status from Xil_SecureZeroize otherwise.
*
* @note
*	Call only from a terminating key-generation path. The pairwise consistency test signs
*	under a key the client has not received yet, so its signature is wiped with the rest of
*	the key-generation state. A client signature left in the scratch is public and needs no
*	wipe.
*
*************************************************************************************************/
s32 XSlhDsa_ZeroizeSigScratch(void)
{
	s32 Status = XASUFW_FAILURE;

	/** Wipe the scratch so the PWCT signature does not outlive the key-generation command. */
	Status = Xil_SecureZeroize(XSlhDsa_SigScratch, sizeof(XSlhDsa_SigScratch));

	return Status;
}

/*************************************************************************************************/
/**
* @brief	Return the DER OID bytes for a supported pre-hash HashId.
*
* @param	HashId	Pre-hash function identifier (XASU_SLHDSA_HASHID_SHA256/SHA512).
*
* @return
*	Pointer to static OID bytes, or NULL if HashId is unsupported.
*
*************************************************************************************************/
const u8 *XSlhDsa_PreHashGetOid(u8 HashId)
{
	const u8 *OidPtr = NULL;

	/**
	 * Map the HashId to its DER OID. An unrecognised id leaves the pointer NULL so callers
	 * reject the request rather than framing PH(M) with a wrong OID.
	 */
	if (HashId == XASU_SLHDSA_HASHID_SHA256) {
		OidPtr = XSlhDsa_PreHashOidSha256;
	}
	else if (HashId == XASU_SLHDSA_HASHID_SHA512) {
		OidPtr = XSlhDsa_PreHashOidSha512;
	}

	return OidPtr;
}
/*************************************************************************************************/
/**
* @brief	Build the HashSLH-DSA context prefix in WorkMem.
*
* Writes 0x01 || ctx_len || ctx || OID into InstPtr->WorkMem and sets CtxPrefixLen.
*
* @param	InstPtr	SLH-DSA instance with WorkMem allocated.
* @param	Ctx	Context bytes when CtxLen is 1..255 (may be NULL when CtxLen is 0).
* @param	CtxLen	FIPS context length (0..255).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM, if InstPtr or its work memory is NULL, CtxLen exceeds
*		  the FIPS 205 limit, Ctx is NULL for a non-zero CtxLen, or the resulting prefix
*		  does not fit work memory.
*		- XASUFW_SLHDSA_UNSUPPORTED_PREHASH_ALG, if the instance HashId has no pre-hash OID.
*		- XASUFW_SLHDSA_COPY_FAILED, if the context string or pre-hash OID copy
*		  fails.
*
*************************************************************************************************/
s32 XSlhDsa_PreHashBuildPrefix(XSlhDsa_Instance *InstPtr, const u8 *Ctx, u32 CtxLen)
{
	s32 Status = XASUFW_FAILURE;
	const u8 *OidPtr;
	u32 PrefixLen;

	/**
	 * Validate input parameters. CtxLen must fit the one-octet context-length field, and a
	 * non-zero context length without a context buffer is inconsistent.
	 */
	if ((InstPtr == NULL) || (InstPtr->WorkMem == NULL) ||
			(CtxLen > XASU_SLHDSA_MAX_CTX_LEN) || ((CtxLen > 0U) && (Ctx == NULL))) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Resolve the OID of the pre-hash function; an unsupported HashId is rejected here. */
	OidPtr = XSlhDsa_PreHashGetOid(InstPtr->HashId);
	if (OidPtr == NULL) {
		Status = XASUFW_SLHDSA_UNSUPPORTED_PREHASH_ALG;
		goto END;
	}

	/** Prefix is 0x01 || ctx_len || ctx || OID; check it fits WorkMem before writing a byte. */
	PrefixLen = XASU_SLHDSA_PREFIX_HDR_LEN + CtxLen + XASU_SLHDSA_PREHASH_OID_LEN;
	if (PrefixLen > XSLHDSA_WORK_MEM_BYTES) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * Write the HashSLH-DSA domain separator (0x01) and context length. The separator is what
	 * stops a HashSLH-DSA signature from validating as a pure one.
	 */
	InstPtr->WorkMem[XASU_SLHDSA_PREFIX_DS_OFFSET] = XASU_SLHDSA_DS_PREHASH;
	InstPtr->WorkMem[XASU_SLHDSA_PREFIX_CTXLEN_OFFSET] = (u8)CtxLen;
	if (CtxLen > 0U) {
		/** - Copy the context string immediately after the two-octet header. */
		Status = Xil_SMemCpy(InstPtr->WorkMem + XASU_SLHDSA_PREFIX_CTX_OFFSET,
					     XSLHDSA_WORK_MEM_BYTES -
					     XASU_SLHDSA_PREFIX_HDR_LEN, Ctx,
					     CtxLen, CtxLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_SLHDSA_COPY_FAILED;
			goto END;
		}
	}

	/**
	 * Append the OID after the context, so the hash paths stream 0x01 || ctx_len || ctx || OID
	 * followed by PH(M) and bind the digest to the pre-hash function used.
	 */
	Status = Xil_SMemCpy(InstPtr->WorkMem + XASU_SLHDSA_PREFIX_CTX_OFFSET + CtxLen,
				     XSLHDSA_WORK_MEM_BYTES - XASU_SLHDSA_PREFIX_HDR_LEN -
				     CtxLen, OidPtr,
				     XASU_SLHDSA_PREHASH_OID_LEN,
				     XASU_SLHDSA_PREHASH_OID_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_COPY_FAILED;
		goto END;
	}

	/** Publish the prefix length only after the full prefix is in place. */
	InstPtr->CtxPrefixLen = PrefixLen;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Prepare HashSLH-DSA message inputs on the instance.
*
* Builds the pre-hash context prefix and sets MsgLocalPtr/MsgLocalLen to client PH(M).
*
* @param	InstPtr	SLH-DSA instance with pre-hash mode fields set.
* @param	Msg	Client-supplied PH(M) digest buffer.
* @param	MsgLen	Length of Msg in bytes (must match HashId digest length).
* @param	Ctx	Context bytes when CtxLen is 1..255.
* @param	CtxLen	FIPS context length (0..255).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM, if InstPtr or Msg is NULL, the instance is not in
*		  HashSLH-DSA mode, or MsgLen does not equal the pre-hash digest length.
*		- XASUFW_SLHDSA_UNSUPPORTED_PREHASH_ALG, if HashId is not the pre-hash function
*		  paired with the bound parameter set.
*		- XASUFW_SLHDSA_UNSUPPORTED_PARAM_SET, if the bound hash width is neither 128s
*		  nor 256s.
*		- XASUFW_SLHDSA_COPY_FAILED, if the pre-hash context or OID prefix copy
*		  fails.
*
*************************************************************************************************/
s32 XSlhDsa_PreparePreHashMsg(XSlhDsa_Instance *InstPtr, const u8 *Msg, u32 MsgLen,
				   const u8 *Ctx, u32 CtxLen)
{
	s32 Status = XASUFW_FAILURE;
	u32 DigestLen;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (Msg == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Reject the call unless the instance was explicitly placed in HashSLH-DSA mode. */
	if (InstPtr->PreHashMode != XASU_SLHDSA_MSG_PREHASH) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * Each set is paired with exactly one pre-hash function: SHA-512 for n = 32 (256s) and
	 * SHA-256 for n = 16 (128s). Any other pairing or hash width is invalid.
	 */
	if (InstPtr->Param.n == XASU_SLHDSA_256S_N) {
		if (InstPtr->HashId != XASU_SLHDSA_HASHID_SHA512) {
			Status = XASUFW_SLHDSA_UNSUPPORTED_PREHASH_ALG;
			goto END;
		}
	}
	else if (InstPtr->Param.n == XASU_SLHDSA_128S_N) {
		if (InstPtr->HashId != XASU_SLHDSA_HASHID_SHA256) {
			Status = XASUFW_SLHDSA_UNSUPPORTED_PREHASH_ALG;
			goto END;
		}
	}
	else {
		Status = XASUFW_SLHDSA_UNSUPPORTED_PARAM_SET;
		goto END;
	}

	/** PH(M) arrives pre-computed, so MsgLen must equal the selected digest length exactly. */
	DigestLen = XAsu_SlhdsaGetPreHashDigestLen(InstPtr->HashId);
	if ((DigestLen == 0U) || (MsgLen != DigestLen)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Assemble 0x01 || ctx_len || ctx || OID in WorkMem. */
	Status = XSlhDsa_PreHashBuildPrefix(InstPtr, Ctx, CtxLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Point the hash paths at PH(M); in pre-hash mode the digest is what gets signed. */
	InstPtr->MsgLocalPtr = Msg;
	InstPtr->MsgLocalLen = DigestLen;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Generate an SLH-DSA key pair from caller-supplied seeds
*		(FIPS 205 Algorithm 18: slh_keygen_internal).
*
* @param	InstPtr	Pointer to SLH-DSA instance bound to a parameter set.
* @param	SkSeed	Secret seed (n bytes), or NULL to draw all three seeds from the TRNG.
* @param	SkPrf	PRF secret (n bytes), NULL only when SkSeed is NULL.
* @param	PkSeed	Public seed (n bytes), NULL only when SkSeed is NULL.
*
* @return
*		- XASUFW_SUCCESS, if key pair generation succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if InstPtr, its ADRS buffer or its staged key region
*		  is NULL, or if SkSeed, SkPrf and PkSeed are not either all supplied or all NULL.
*		- XASUFW_SLHDSA_COPY_FAILED, if writing a generated field of the key pair fails.
*		- XASUFW_CMD_IN_PROGRESS, if a cooperative yield slice expired and key
*		  generation will resume.
*		- XASUFW_SLHDSA_KEYGEN_FAILED, if seed generation from the TRNG or the top XMSS
*		  tree root build fails, combined with the underlying callee status via
*		  XAsufw_UpdateErrorStatus.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_ZEROIZE_MEMSET_FAIL,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
* @note
*	Caller-supplied seeds must come from a TRNG or approved entropy source, and are copied
*	into the staged key. Passing all three as NULL draws them here instead, which needs the
*	TRNG resource and leaves the secret prefix with no second copy. InstPtr must already be
*	bound to the target parameter set. The key pair is built in the staged key region of the
*	instance, where SK is the 4n-byte image and PK is its trailing 2n bytes, so the caller
*	reads both from there once this returns XASUFW_SUCCESS.
*
*************************************************************************************************/
s32 XSlhDsa_KeyGenInternal(XSlhDsa_Instance *InstPtr, const u8 *SkSeed,
				 const u8 *SkPrf, const u8 *PkSeed)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XSlhDsa_Param *ParamPtr;
	XSlhDsa_YieldCtx *YieldPtr;
	u32 HashWidth;

	/** Validate input pointers; ADRS is required by the top XMSS root build below. */
	if ((InstPtr == NULL) || (InstPtr->Addr == NULL) || (InstPtr->KeyScratch == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * SkSeed selects the seed source, so the three seeds arrive together or not at all. A
	 * partial set would either stage a key whose fields come from two different sources, or
	 * silently draw fresh seeds over the ones the caller did supply.
	 */
	if (((SkSeed != NULL) && ((SkPrf == NULL) || (PkSeed == NULL))) ||
			((SkSeed == NULL) && ((SkPrf != NULL) || (PkSeed != NULL)))) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	YieldPtr = &InstPtr->Yield;
	/** The bound parameter set fixes the hash width n, which sizes every SK and PK field. */
	ParamPtr = &InstPtr->Param;
	HashWidth = ParamPtr->n;

	if (YieldPtr->Keygen.Stage == XSLHDSA_KEYGEN_STAGE_NONE) {
		/** Capture key-generation perf start when measurement is enabled. */
		XASUFW_MEASURE_PERF_START(XASU_MODULE_SLHDSA_ID);

		/**
		 * - Step 4 (SK layout): SK <- (SK.seed, SK.prf, PK.seed, PK.root)
		 * - Step 4 (PK layout): PK <- (PK.seed, PK.root)
		 *
		 * PK is the trailing 2n bytes of SK, so staging the secret key lays out both.
		 * Caller-supplied seeds are copied in, otherwise they are drawn straight into the
		 * fields they occupy. PK.root stays zeroed until the root is committed below.
		 */
		if (SkSeed != NULL) {
			Status = XSlhDsa_KeyWriteSkPrefix(SkSeed, SkPrf, PkSeed, HashWidth,
							  InstPtr->KeyScratch);
		} else {
			Status = XSlhDsa_KeyDrawSkPrefix(InstPtr, HashWidth);
		}
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		/**
		 * Name the staged fields on the instance; the root build derives every
		 * WOTS+ secret from SK.seed, so this must precede the xmss_node call.
		 */
		XSlhDsa_PublishStagedSkViews(InstPtr, HashWidth);

		YieldPtr->Keygen.Stage = XSLHDSA_KEYGEN_STAGE_PREFIX_DONE;
	}

	/**
	 * - Step 1: ADRS <- toByte(0, 32)
	 * - Step 2: ADRS.setLayerAddress(d - 1)
	 * - Step 3: PK.root <- xmss_node(SK.seed, 0, h', PK.seed, ADRS)
	 *
	 * The root lives on Yield so it survives a cooperative yield unwind.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSlhDsa_KeyBuildTopXmssRoot(InstPtr, ParamPtr, YieldPtr->Keygen.Root);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		goto END;
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SLHDSA_KEYGEN_FAILED);
		goto END;
	}

	/** - Step 4: commit the root into the PK.root field of the staged secret key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSlhDsa_KeyCommitHypertreeRoot(InstPtr, YieldPtr->Keygen.Root, HashWidth);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	YieldPtr->Keygen.Stage = XSLHDSA_KEYGEN_STAGE_NONE;
	Status = XASUFW_SUCCESS;

END:
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		/** Capture key-generation perf stop when measurement is enabled. */
		XASUFW_MEASURE_PERF_STOP(XASU_MODULE_SLHDSA_ID);
		if (InstPtr != NULL) {
			/** Zeroize the yield root buffer and merge the clear result into Status. */
			Status = XAsufw_UpdateBufStatus(Status,
					Xil_SecureZeroize(InstPtr->Yield.Keygen.Root,
							  sizeof(InstPtr->Yield.Keygen.Root)));
		}
	}
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Generate an SLH-DSA signature over a message and context string.
*
* Implements Algorithm 22 (slh_sign) from FIPS 205 when PreHashMode is PURE, and
* Algorithm 23 (hash_slh_sign) when it is PREHASH.
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	Msg	Message buffer.
* @param	MsgLen	Message length in bytes.
* @param	Ctx	Context bytes when CtxLen is 1..255.
* @param	CtxLen	FIPS context length 0..255.
* @param	Sk	Secret key buffer.
* @param	OptRand	Optional additional randomness (add_rand); NULL for deterministic mode
*			(PK.seed used as opt_rand per FIPS 205 Algorithm 19, step 2).
* @param	Sig	Output signature buffer; must be a local buffer.
* @param	SigLen	Length of the signature output buffer in bytes; must equal
*			XSlhDsa_SignatureBytes() for the bound parameter set.
*
* @return
*		- XASUFW_SUCCESS, if signature generation succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if InstPtr, its work memory, Sk, Sig or Msg is NULL,
*		  SigLen does not match the bound parameter set signature size, CtxLen exceeds the
*		  FIPS 205 limit or Ctx is NULL for a non-zero CtxLen.
*		- XASUFW_SLHDSA_COPY_FAILED, if loading the secret key into the instance
*		  fails.
*		- XASUFW_SLHDSA_PRF_MSG_FAILED, if PRF_msg fails to produce the randomizer R,
*		  combined with the underlying callee status via XAsufw_UpdateErrorStatus.
*		- XASUFW_SLHDSA_HASH_MSG_FAILED, if H_msg fails, combined with the underlying
*		  callee status via XAsufw_UpdateErrorStatus.
*		- XASUFW_CMD_IN_PROGRESS, if a cooperative yield slice expired and signing
*		  will resume.
*		- XASUFW_SLHDSA_FORS_FAILED or XASUFW_SLHDSA_HT_FAILED, if the FORS or
*		  hypertree stage fails, combined with the underlying callee status via
*		  XAsufw_UpdateErrorStatus.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
*	Hedged signing (OptRand != NULL): FIPS recommended; each signature uses fresh add_rand.
*	Deterministic signing (OptRand == NULL): uses PK.seed as add_rand; repeated signs of the
*	same message yield identical signatures.
*
*	Context handling when PreHashMode == PURE:
*	- CtxLen == 0: FIPS slh_sign empty context — prefix 0x00 || 0x00 in WorkMem.
*	- CtxLen 1..255: FIPS slh_sign — prefix 0x00 || len || ctx in WorkMem.
*
*************************************************************************************************/
s32 XSlhDsa_SignInternal(XSlhDsa_Instance *InstPtr, const u8 *Msg, u32 MsgLen,
		      const u8 *Ctx, u32 CtxLen, const u8 *Sk, const u8 *OptRand,
		      u8 *Sig, u32 SigLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const u8 *EntropyPtr;
	XSlhDsa_YieldCtx *YieldPtr;
	u32 HashWidth;
	u32 PrefixLen;

	/**
	 * Validate input parameters. SigLen must be the exact signature size of the bound set so
	 * the R, SIG_FORS and SIG_HT regions stay inside the caller buffer.
	 */
	if ((InstPtr == NULL) || (InstPtr->WorkMem == NULL) || (Sk == NULL) || (Sig == NULL) ||
			(Msg == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** The length checks follow the NULL checks because they read the bound parameter set. */
	if ((SigLen != XSlhDsa_SignatureBytes(&InstPtr->Param)) ||
			(CtxLen > XASU_SLHDSA_MAX_CTX_LEN) || ((CtxLen > 0U) && (Ctx == NULL))) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	YieldPtr = &InstPtr->Yield;

	if (YieldPtr->Sign.Stage == XSLHDSA_SIGN_STAGE_NONE) {
		/** Capture signing perf start when measurement is enabled. */
		XASUFW_MEASURE_PERF_START(XASU_MODULE_SLHDSA_ID);

		/** Stage the caller secret key. */
		if (Sk == InstPtr->KeyScratch) {
			XSlhDsa_PublishStagedSkViews(InstPtr, InstPtr->Param.n);
			Status = XASUFW_SUCCESS;
		} else {
			Status = XSlhDsa_LoadSk(InstPtr, Sk);
		}
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		/**
		 * - Step 2: opt_rand <- addrnd, substituting opt_rand <- PK.seed for the
		 *   deterministic variant.
		 *
		 * Bind the message and signature buffers, then pick the PRF_msg randomness.
		 */
		XSlhDsa_SetMsg(InstPtr, Msg, MsgLen);
		XSlhDsa_SetSig(InstPtr, Sig, SigLen);
		EntropyPtr = (OptRand != NULL) ? OptRand : InstPtr->PkSeed;

		if (InstPtr->PreHashMode == XASU_SLHDSA_MSG_PREHASH) {
			/**
			 * HashSLH-DSA: 0x01 || ctx_len || ctx || OID over the
			 * client-supplied PH(M).
			 */
			Status = XSlhDsa_PreparePreHashMsg(InstPtr, Msg, MsgLen, Ctx, CtxLen);
			if (Status != XASUFW_SUCCESS) {
				goto END;
			}
		} else {
			/**
			 * - slh_sign Step 8: M' <- toByte(0, 1) || toByte(|ctx|, 1) || ctx || M
			 *
			 * The domain separator and context length are staged in WorkMem.
			 */
			InstPtr->WorkMem[XASU_SLHDSA_PREFIX_DS_OFFSET] = XASU_SLHDSA_DS_PURE;
			InstPtr->WorkMem[XASU_SLHDSA_PREFIX_CTXLEN_OFFSET] = (u8)CtxLen;
			if (CtxLen > 0U) {
				/** - Append the context string after the two-octet header. */
				Status = Xil_SMemCpy(
						InstPtr->WorkMem + XASU_SLHDSA_PREFIX_CTX_OFFSET,
						XSLHDSA_WORK_MEM_BYTES -
						XASU_SLHDSA_PREFIX_HDR_LEN, Ctx, CtxLen, CtxLen);
				if (Status != XASUFW_SUCCESS) {
					goto END;
				}
			}
			/**
			 * - Record how many WorkMem bytes the hash paths must stream before the
			 *   message.
			 */
			InstPtr->CtxPrefixLen = XASU_SLHDSA_PREFIX_HDR_LEN + CtxLen;
		}

		/**
		 * - Step 3: R <- PRF_msg(SK.prf, opt_rand, M)
		 * - Step 4: SIG <- R
		 *
		 * R is written straight into the head of the signature buffer.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSlhDsa_HashPrfMsg(InstPtr, EntropyPtr, Msg, MsgLen, Ctx, CtxLen, Sig);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SLHDSA_PRF_MSG_FAILED);
			goto END;
		}

		YieldPtr->Sign.Stage = XSLHDSA_SIGN_STAGE_PRF_DONE;
	}

	if (YieldPtr->Sign.Stage == XSLHDSA_SIGN_STAGE_PRF_DONE) {
		/**
		 * - Step 5: digest <- H_msg(R, PK.seed, PK.root, M)
		 *
		 * R is read back from the signature.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSlhDsa_HashMsg(InstPtr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SLHDSA_HASH_MSG_FAILED);
			goto END;
		}

		YieldPtr->Sign.Stage = XSLHDSA_SIGN_STAGE_HMSG_DONE;
	}

	/**
	 * - Step 15: SIG <- SIG || SIG_FORS
	 * - Step 18: SIG <- SIG || SIG_HT
	 *
	 * Signature layout is R || SIG_FORS || SIG_HT, so the body starts n bytes past R.
	 */
	HashWidth = InstPtr->Param.n;
	PrefixLen = HashWidth;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSlhDsa_SignDigestBody(InstPtr, InstPtr->MsgDigest, Sig + PrefixLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	YieldPtr->Sign.Stage = XSLHDSA_SIGN_STAGE_NONE;
	Status = XASUFW_SUCCESS;

END:
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		/** Capture signing perf stop when measurement is enabled. */
		XASUFW_MEASURE_PERF_STOP(XASU_MODULE_SLHDSA_ID);
	}
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Verify an SLH-DSA signature using buffers bound on the instance
*		(H_msg digest, FORS recovery, hypertree verification).
*
* Implements Algorithm 20 (slh_verify_internal) from FIPS 205.
*
* @param	InstPtr	Pointer to initialized SLH-DSA instance with message, signature,
*			public key, and scratch buffers already set.
*
* @return
*		- XASUFW_SUCCESS, if signature verification succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if instance state is invalid or SigLen does not match
*		  the bound parameter set signature size.
*		- XASUFW_SLHDSA_VERIFY_ROOT_MISMATCH, if the hypertree root recovered from the
*		  signature does not match PK.root. This is the verdict for a bad signature and is
*		  reported on its own, without a stage tag.
*		- XASUFW_SLHDSA_HASH_MSG_FAILED, if H_msg fails, combined with the
*		  underlying callee status via XAsufw_UpdateErrorStatus.
*		- XASUFW_SLHDSA_FORS_FAILED or XASUFW_SLHDSA_HT_FAILED, if the FORS
*		  or hypertree stage fails for any reason other than a root mismatch, combined with
*		  the underlying callee status via XAsufw_UpdateErrorStatus.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_ZEROIZE_MEMSET_FAIL,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or
*		  XASUFW_SHA_FINISH_FAILED).
*
* @note
*	On success, sets ReturnStatus to XASUFW_SLHDSA_SIGNATURE_VERIFIED, which XSlhDsa_Verify
*	confirms as a redundant check on the returned status.
*
*************************************************************************************************/
s32 XSlhDsa_VerifyCore(XSlhDsa_Instance *InstPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XSlhDsa_Param *ParamPtr;
	const u8 *SignForsPtr;
	const u8 *SignHtPtr;
	XSlhDsa_HtIndices HtIndices;
	u32 HashWidth;
	u32 ForsSignOffset;
	u32 HtSignOffset;

	/**
	 * - Step 1: if |SIG| != (1 + k(1 + a) + h + d * len) * n then return false
	 *
	 * Instance state is checked alongside the length, because the FORS and hypertree
	 * offsets computed below derive from the bound parameter set.
	 */
	if ((InstPtr == NULL) || (InstPtr->Addr == NULL) ||
			(InstPtr->SigAddr == 0U) || (InstPtr->MsgDigest == NULL) ||
			(InstPtr->NodeBuf == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	if (InstPtr->SigLen != XSlhDsa_SignatureBytes(&InstPtr->Param)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	ParamPtr = &InstPtr->Param;
	HashWidth = ParamPtr->n;

	/** - Step 8: digest <- H_msg(R, PK.seed, PK.root, M); R is read from the signature. */
	Status = XSlhDsa_HashMsg(InstPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SLHDSA_HASH_MSG_FAILED);
		goto END;
	}

	/**
	 * - Step 9:  md <- digest[0 : ceil(k * a / 8)]
	 * - Step 10: tmp_idxtree <- digest[ceil(k*a/8) : ceil(k*a/8) + ceil((h - h/d)/8)]
	 * - Step 11: tmp_idxleaf <- digest[... : ... + ceil(h / (8 * d))]
	 * - Step 12: idxtree <- toInt(tmp_idxtree, ceil((h - h/d)/8)) mod 2^(h - h/d)
	 * - Step 13: idxleaf <- toInt(tmp_idxleaf, ceil(h / (8 * d))) mod 2^(h/d)
	 *
	 * Split the digest as the signer did; a tampered message shifts idx_tree / idx_leaf.
	 */
	XSlhDsa_HtParseDigest(ParamPtr, InstPtr->MsgDigest, &HtIndices);

	/** - Step 4: ADRS <- toByte(0, 32) */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(InstPtr->Addr, sizeof(*InstPtr->Addr), 0U, sizeof(*InstPtr->Addr));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** - Step 14: ADRS.setTreeAddress(idxtree) */
	XSlhDsa_AddrSetTree(InstPtr->Addr, HtIndices.IdxTree);

	/** - Step 15: ADRS.setTypeAndClear(FORS_TREE) */
	XSlhDsa_AddrSetTypeClearTail(InstPtr->Addr, XSLHDSA_ADRS_FORS_TREE);

	/** - Step 16: ADRS.setKeyPairAddress(idxleaf) */
	XSlhDsa_AddrSetKeyPair(InstPtr->Addr, (u32)HtIndices.IdxLeaf);

	/** - Step 5: R <- SIG.getR(), SIG[0 : n]; SIG_FORS therefore starts at offset n. */
	ForsSignOffset = HashWidth;

	/**
	 * - Step 6: SIG_FORS <- SIG.getSIG_FORS(), SIG[n : (1 + k(1 + a)) * n]
	 * - Step 7: SIG_HT <- SIG.getSIG_HT(), SIG[(1 + k(1 + a)) * n : end]
	 */
	HtSignOffset = (1U + (ParamPtr->fors_count * (1U + ParamPtr->fors_height))) * HashWidth;
	SignForsPtr = ((const u8 *)(UINTPTR)InstPtr->SigAddr) + ForsSignOffset;
	SignHtPtr = ((const u8 *)(UINTPTR)InstPtr->SigAddr) + HtSignOffset;

	/** - Step 17: PK_FORS <- fors_pkFromSig(SIG_FORS, md, PK.seed, ADRS) */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSlhDsa_ForsPkFromSig(InstPtr, SignForsPtr, InstPtr->MsgDigest,
					   InstPtr->NodeBuf);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SLHDSA_FORS_FAILED);
		goto END;
	}

	/**
	 * - Step 18: return ht_verify(PK_FORS, SIG_HT, PK.seed, idxtree, idxleaf, PK.root)
	 *
	 * The walk ends in the root comparison against PK.root, so this is the accept/reject
	 * decision. The status is re-armed to a failure value first, so that the success left
	 * by the FORS stage cannot stand in for the root comparison result.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSlhDsa_HtVerify(InstPtr, InstPtr->NodeBuf, SignHtPtr, &HtIndices);
	if (Status == XASUFW_SUCCESS) {
		/*
		 * Root matched PK.root: raise the additional verified indication that
		 * XSlhDsa_Verify checks alongside the returned status.
		 */
		ReturnStatus = XASUFW_SLHDSA_SIGNATURE_VERIFIED;
	} else if (Status != XASUFW_SLHDSA_VERIFY_ROOT_MISMATCH) {
		/*
		 * A root mismatch is the verification verdict for a bad signature and is
		 * reported on its own. Any other status is an internal failure, so it keeps its
		 * cause and gains the hypertree stage tag.
		 */
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SLHDSA_HT_FAILED);
	} else {
		/* Do nothing */
	}

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Verify an SLH-DSA signature for a message, context string and public key.
*
* Implements Algorithm 24 (slh_verify) from FIPS 205 when PreHashMode is PURE, and
* Algorithm 25 (hash_slh_verify) when it is PREHASH.
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	Msg	Pointer to message buffer.
* @param	MsgLen	Length of message in bytes.
* @param	Sig	Pointer to signature buffer.
* @param	SigLen	Length of the signature buffer in bytes; must equal
*			XSlhDsa_SignatureBytes() for the bound parameter set.
* @param	Ctx	Context bytes when CtxLen is 1..255.
* @param	CtxLen	FIPS context length 0..255.
* @param	Pk	Pointer to public key (PK.seed || PK.root).
*
* @return
*		- XASUFW_SUCCESS, if signature verification succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if InstPtr, its work memory, Pk, Sig or Msg is NULL,
*		  SigLen does not match the bound parameter set signature size, CtxLen exceeds the
*		  FIPS 205 limit or Ctx is NULL for a non-zero CtxLen.
*		- XASUFW_SLHDSA_VERIFY_ROOT_MISMATCH, if the hypertree root recovered from the
*		  signature does not match PK.root. This is the verdict for a bad signature and is
*		  reported on its own, without a stage tag.
*		- XASUFW_SLHDSA_HASH_MSG_FAILED, XASUFW_SLHDSA_FORS_FAILED or
*		  XASUFW_SLHDSA_HT_FAILED, if the H_msg, FORS or hypertree stage fails for
*		  any reason other than a root mismatch, combined with the underlying callee status
*		  via XAsufw_UpdateErrorStatus.
*		- XASUFW_SLHDSA_COPY_FAILED, if loading the public key into the instance
*		  fails.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
*	Context handling when PreHashMode == PURE:
*	- CtxLen == 0: FIPS slh_verify empty context — prefix 0x00 || 0x00 in WorkMem.
*	- CtxLen 1..255: FIPS slh_verify — prefix 0x00 || len || ctx in WorkMem.
*
*************************************************************************************************/
s32 XSlhDsa_VerifyInternal(XSlhDsa_Instance *InstPtr, const u8 *Msg, u32 MsgLen,
			const u8 *Sig, u32 SigLen, const u8 *Ctx, u32 CtxLen,
			const u8 *Pk)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Capture verification perf start when measurement is enabled. */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_SLHDSA_ID);

	/**
	 * - Step 1: if |ctx| > 255 then return false
	 *
	 * A wrong SigLen is rejected here too, before any hashing is done.
	 */
	if ((InstPtr == NULL) || (InstPtr->WorkMem == NULL) || (Pk == NULL) || (Sig == NULL) ||
			(Msg == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** The length checks follow the NULL checks because they read the bound parameter set. */
	if ((SigLen != XSlhDsa_SignatureBytes(&InstPtr->Param)) ||
			(CtxLen > XASU_SLHDSA_MAX_CTX_LEN) || ((CtxLen > 0U) && (Ctx == NULL))) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Split the caller public key into PK.seed and PK.root on the instance. */
	Status = XSlhDsa_LoadPk(InstPtr, Pk);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Bind the buffers; H_msg reads PK.seed, PK.root and R from the head of the signature. */
	XSlhDsa_SetPkAddr(InstPtr, Pk);
	XSlhDsa_SetMsg(InstPtr, Msg, MsgLen);
	XSlhDsa_SetSig(InstPtr, Sig, SigLen);

	/**
	 * Rebuild the prefix the signer used. The domain-separator octet differs between pure and
	 * pre-hash, so a signature made in one mode cannot verify in the other.
	 */
	if (InstPtr->PreHashMode == XASU_SLHDSA_MSG_PREHASH) {
		/** HashSLH-DSA: 0x01 || ctx_len || ctx || OID over the client-supplied PH(M). */
		Status = XSlhDsa_PreparePreHashMsg(InstPtr, Msg, MsgLen, Ctx, CtxLen);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	} else {
		/**
		 * - Step 4: M' <- toByte(0, 1) || toByte(|ctx|, 1) || ctx || M
		 *
		 * The domain separator and context length are staged in WorkMem.
		 */
		InstPtr->WorkMem[XASU_SLHDSA_PREFIX_DS_OFFSET] = XASU_SLHDSA_DS_PURE;
		InstPtr->WorkMem[XASU_SLHDSA_PREFIX_CTXLEN_OFFSET] = (u8)CtxLen;
		if (CtxLen > 0U) {
			/** - Append the context string after the two-octet header. */
			Status = Xil_SMemCpy(
					InstPtr->WorkMem + XASU_SLHDSA_PREFIX_CTX_OFFSET,
					XSLHDSA_WORK_MEM_BYTES -
					XASU_SLHDSA_PREFIX_HDR_LEN, Ctx, CtxLen, CtxLen);
			if (Status != XASUFW_SUCCESS) {
				goto END;
			}
		}
		/**
		 * - Record how many WorkMem bytes the hash paths must stream before the
		 *   message.
		 */
		InstPtr->CtxPrefixLen = XASU_SLHDSA_PREFIX_HDR_LEN + CtxLen;
	}

	/**
	 * - Step 5: return slh_verify_internal(M', SIG, PK)
	 *
	 * Recompute the digest, recover Pk_FORS, and check the hypertree root against PK.root.
	 * The status is re-armed to a failure value first, so that a success left by the prefix
	 * staging above cannot stand in for the verification verdict.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSlhDsa_VerifyCore(InstPtr);

END:
	/** Capture verification perf stop when measurement is enabled. */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_SLHDSA_ID);
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Generate an SLH-DSA key pair for a key manager key-generation request.
*
* @param	DmaPtr	Pointer to the DMA instance allocated for the request.
* @param	ShaPtr	Pointer to the SHA instance matching the requested parameter set.
* @param	ParamsPtr	Pointer to XSlhDsa_KeyGenParams, which the key manager fills in
*				from the request snapshot on the instance. ParamSet selects the
*				key sizes and the hash family, and the two key addresses name
*				the buffers that receive the pair. The same values are passed on
*				every slice.
*
* @return
*		- XASUFW_SUCCESS, if key generation completes successfully.
*		- XASUFW_SLHDSA_INVALID_PARAM, if any input pointer is NULL, the singleton instance
*		  is unavailable, an output address is zero, the parameter set is unsupported or an
*		  output buffer does not match the key size.
*		- XASUFW_SLHDSA_KEYGEN_FAILED, if seed generation from the TRNG or the top XMSS tree
*		  root build fails, combined with the underlying callee status via
*		  XAsufw_UpdateErrorStatus.
*		- XASUFW_SLHDSA_PWCT_SIGN_GEN_FAIL, if the pairwise consistency test cannot produce
*		  a signature over the empty message with the new secret key.
*		- XASUFW_SLHDSA_PWCT_SIGN_VER_FAIL, if that signature does not verify under the new
*		  public key, or if the redundant verified indicator does not confirm success.
*		- XASUFW_SLHDSA_KEY_OUT_DMA_FAILED, if the secret or public key DMA transfer fails.
*		- XASUFW_CMD_IN_PROGRESS, if a cooperative yield slice expired and key
*		  generation or the pairwise consistency test sign will resume.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_ZEROIZE_MEMSET_FAIL,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
* @note
*	The three FIPS 205 seeds are drawn from the TRNG, so the caller must hold the TRNG
*	resource. They land directly in the staged key region, which holds the finished pair until
*	it is transferred to PrivKeyAddr and PubKeyAddr, so a failure never leaves partial key
*	material with the caller. After the pair is built, a FIPS 140-3 IG 10.3.A pairwise
*	consistency test signs an empty message with the new secret key and verifies it with the
*	new public key; the pair is transferred only if that test passes. Long parameter sets yield
*	cooperatively and return XASUFW_CMD_IN_PROGRESS until the top XMSS root and the PWCT
*	signature are complete.
*
*************************************************************************************************/
s32 XSlhDsa_KeyGen(XAsufw_Dma *DmaPtr, XSha *ShaPtr, const XSlhDsa_KeyGenParams *ParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var FihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XSlhDsa_Instance *InstPtr = XSlhDsa_GetInstance();
	u32 ExpPubKeyLen;
	u32 ExpPrivKeyLen;
	u32 N;

	/** Validate input pointers and the mandatory output addresses. */
	if ((DmaPtr == NULL) || (ShaPtr == NULL) || (ParamsPtr == NULL) || (InstPtr == NULL) ||
	    (ParamsPtr->PubKeyAddr == 0U) || (ParamsPtr->PrivKeyAddr == 0U)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Confirm the output buffers hold the key sizes of the requested parameter set. */
	ExpPubKeyLen = XAsu_SlhdsaGetPkSize(ParamsPtr->ParamSet);
	ExpPrivKeyLen = XAsu_SlhdsaGetSkSize(ParamsPtr->ParamSet);
	if ((ExpPubKeyLen == 0U) || (ExpPrivKeyLen == 0U) ||
	    (ParamsPtr->PubKeyLen != ExpPubKeyLen) || (ParamsPtr->PrivKeyLen != ExpPrivKeyLen)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Bind the singleton instance to the requested parameter set and to the engines. */
	Status = XSlhDsa_PrepareInstance(InstPtr, ParamsPtr->ParamSet, DmaPtr, ShaPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** The bound parameter set fixes the seed length n required by FIPS 205 key generation. */
	N = InstPtr->Param.n;

	/**
	 * Draw the three FIPS 205 seeds and generate the key pair from them, computing the
	 * top-layer XMSS root (FIPS 205 Algorithm 18: slh_keygen_internal). NULL seeds place the
	 * TRNG output straight into the staged key, so the secret prefix has no second copy to
	 * survive a yield or to clear afterwards. This is the long-running step. Skip it when
	 * resuming a PWCT sign so the hypertree is not rebuilt.
	 *
	 * The status is returned unchanged: the key generation stage already tags its own
	 * failures, and only three error fields are available, so re-tagging here would
	 * overwrite the stage that identifies where the failure happened.
	 */
	if (InstPtr->Yield.Keygen.Stage != XSLHDSA_KEYGEN_STAGE_PWCT_SIGN) {
		XFIH_CALL(XSlhDsa_KeyGenInternal, FihVar, Status, InstPtr, NULL, NULL, NULL);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		/**
		 * Mark the pair complete and drop the KeyGen tree checkpoints before the PWCT
		 * sign reuses the same XMSS/FORS/hypertree resume slots.
		 */
		InstPtr->Yield.Keygen.Stage = XSLHDSA_KEYGEN_STAGE_PWCT_SIGN;
		InstPtr->Yield.Sign.Stage = XSLHDSA_SIGN_STAGE_NONE;
		InstPtr->Yield.Xmss.Resume = 0U;
		InstPtr->Yield.Xmss.EmitResume = 0U;
		InstPtr->Yield.Xmss.AuthPathResume = 0U;
		InstPtr->Yield.Xmss.EmitWotsDone = 0U;
		InstPtr->Yield.Fors.TreeResume = 0U;
		InstPtr->Yield.Fors.EmitResume = 0U;
		InstPtr->Yield.Ht.Resume = 0U;
		InstPtr->Yield.Ht.EmitPending = 0U;
	}

	/**
	 * Sign an empty message with the new secret key and verify it with the new public key
	 * before the pair is exported, so an inconsistent pair is never handed to the client.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSlhDsa_Pwct(InstPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Transfer the staged secret key to the caller output buffer. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)InstPtr->KeyScratch, ParamsPtr->PrivKeyAddr,
			       ExpPrivKeyLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_KEY_OUT_DMA_FAILED;
		goto END;
	}

	/** Transfer the public key, which is the trailing 2n bytes of the staged secret key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr,
			       (u64)(UINTPTR)&InstPtr->KeyScratch[XSLHDSA_SK_PKSEED_FIELD * N],
			       ParamsPtr->PubKeyAddr, ExpPubKeyLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_KEY_OUT_DMA_FAILED;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Generate an SLH-DSA signature for a client sign request.
*
* @param	DmaPtr	Pointer to the DMA instance allocated for the request.
* @param	ShaPtr	Pointer to the SHA instance matching the requested parameter set.
* @param	ParamsPtr	Pointer to the client supplied sign parameters.
*
* @return
*		- XASUFW_SUCCESS, if signing completes successfully.
*		- XASUFW_SLHDSA_INVALID_PARAM, if any input pointer is NULL, the singleton instance
*		  is unavailable, the sign parameters are invalid or the requested signature does
*		  not fit the signature scratch.
*		- XASUFW_SLHDSA_SIGN_FAILED, if add_rand generation from the TRNG fails, combined
*		  with the underlying callee status via XAsufw_UpdateErrorStatus.
*		- XASUFW_SLHDSA_SIGN_OUT_DMA_FAILED, if the finished signature cannot be written to
*		  the client output buffer.
*		- XASUFW_SLHDSA_PRF_MSG_FAILED, XASUFW_SLHDSA_HASH_MSG_FAILED,
*		  XASUFW_SLHDSA_FORS_FAILED or XASUFW_SLHDSA_HT_FAILED, identifying the
*		  signing stage that failed, combined with the underlying callee status via
*		  XAsufw_UpdateErrorStatus.
*		- XASUFW_CMD_IN_PROGRESS, if a cooperative yield slice expired and signing
*		  will resume.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or
*		  XASUFW_SHA_FINISH_FAILED).
*
* @note
*	The message, secret key and context buffers are read in place, as they already reside in
*	memory visible to both client and ASUFW. The signature is built in the signature scratch and
*	written to the client buffer only once it is complete. SignMode selects FIPS hedged
*	signing, which draws a fresh add_rand from the TRNG and so needs the TRNG resource, or
*	deterministic signing. Hedged signing draws add_rand only on the first slice, but the
*	TRNG remains allocated until the command finishes, so other TRNG users stay blocked
*	for the full operation. Long parameter sets yield cooperatively and return
*	XASUFW_CMD_IN_PROGRESS until FORS and hypertree signing complete.
*
*************************************************************************************************/
s32 XSlhDsa_Sign(XAsufw_Dma *DmaPtr, XSha *ShaPtr, const XAsu_SlhdsaSignParams *ParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(SignStatus, XASUFW_FAILURE);
	XFih_Var FihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XSlhDsa_Instance *InstPtr = XSlhDsa_GetInstance();
	const u8 *OptRandPtr = NULL;
	const u8 *MsgPtr;
	const u8 *SkPtr;
	const u8 *CtxPtr;
	XSlhDsa_YieldCtx *YieldPtr;
	u8 *SigPtr;

	/** Validate input pointers. */
	if ((DmaPtr == NULL) || (ShaPtr == NULL) || (ParamsPtr == NULL) || (InstPtr == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	YieldPtr = &InstPtr->Yield;

	/**
	 * Validate the sign parameters. This bounds MsgLen and confirms SignatureLen and SkLen
	 * match the parameter set, so the algorithm code needs no further length checks.
	 */
	if (XAsu_SlhdsaValidateSignParams(ParamsPtr) != XST_SUCCESS) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Bind the singleton instance to the requested parameter set and to the engines. */
	Status = XSlhDsa_PrepareInstance(InstPtr, ParamsPtr->ParamSet, DmaPtr, ShaPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Bound the signature to the scratch it is built in before anything is written there. */
	if (ParamsPtr->SignatureLen > (u32)sizeof(XSlhDsa_SigScratch)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * Select pure or pre-hash signing, and resolve the message, secret key and context
	 * addresses. CtxAddr is zero only when CtxLen is zero, so CtxPtr is NULL exactly for
	 * the FIPS empty context (prefix 0x00 || 0x00). The signature is built in the signature
	 * scratch and copied out once it is complete.
	 */
	XSlhDsa_SetMsgMode(InstPtr, ParamsPtr->PreHashMode, ParamsPtr->HashId);
	MsgPtr = (const u8 *)(UINTPTR)ParamsPtr->MsgAddr;
	SkPtr = (const u8 *)(UINTPTR)ParamsPtr->SkObj.SkAddr;
	SigPtr = XSlhDsa_SigScratch;
	CtxPtr = (const u8 *)(UINTPTR)ParamsPtr->CtxAddr;

	/**
	 * Deterministic signing derives the randomizer from SK.prf alone. Hedged signing, the
	 * FIPS recommended default, draws a fresh n-byte add_rand from TRNG per signature. The
	 * randomizer lives on Yield so it is reused across cooperative yield slices.
	 */
	if (ParamsPtr->SignMode == XASU_SLHDSA_SIGN_MODE_DETERMINISTIC) {
		OptRandPtr = NULL;
	} else {
		if (YieldPtr->Sign.Stage == XSLHDSA_SIGN_STAGE_NONE) {
			XFIH_CALL(XTrng_GetRandomNumbers, FihVar, Status, YieldPtr->Sign.AddRand,
				  InstPtr->Param.n);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
								  XASUFW_SLHDSA_SIGN_FAILED);
				goto END;
			}
		}
		OptRandPtr = YieldPtr->Sign.AddRand;
	}

	/**
	 * Generate the signature (FIPS 205 Algorithm 22: slh_sign, or Algorithm 23:
	 * hash_slh_sign in pre-hash mode), keeping the callee status in its own variable so a
	 * glitched Status cannot stand in for the signing result.
	 *
	 * The status is returned unchanged: the signing stage already tags its own failures,
	 * and only three error fields are available, so re-tagging here would overwrite the
	 * stage that identifies where the failure happened.
	 */
	XFIH_CALL(XSlhDsa_SignInternal, FihVar, SignStatus, InstPtr, MsgPtr, ParamsPtr->MsgLen,
		  CtxPtr, ParamsPtr->CtxLen, SkPtr, OptRandPtr, SigPtr,
		  ParamsPtr->SignatureLen);
	if (SignStatus != XASUFW_SUCCESS) {
		Status = SignStatus;
		goto END;
	}

	/** Transfer the finished signature to the caller output buffer. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)XSlhDsa_SigScratch,
			       ParamsPtr->SignatureAddr, ParamsPtr->SignatureLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_SIGN_OUT_DMA_FAILED;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Verify an SLH-DSA signature for a client verify request.
*
* @param	DmaPtr	Pointer to the DMA instance allocated for the request.
* @param	ShaPtr	Pointer to the SHA instance matching the requested parameter set.
* @param	ParamsPtr	Pointer to the client supplied verify parameters.
*
* @return
*		- XASUFW_SUCCESS, if signature verification succeeds.
*		- XASUFW_SLHDSA_INVALID_PARAM, if any input pointer is NULL, the singleton instance
*		  is unavailable, the verify parameters are invalid or the supplied signature does
*		  not fit the signature scratch.
*		- XASUFW_SLHDSA_SIG_IN_DMA_FAILED, if the client signature cannot be copied into
*		  the signature scratch.
*		- XASUFW_SLHDSA_VERIFY_ROOT_MISMATCH, if the signature does not verify against the
*		  public key.
*		- XASUFW_SLHDSA_SIGN_VERIFY_FAILED, if the redundant verified indicator does not
*		  confirm the returned status.
*		- XASUFW_SLHDSA_HASH_MSG_FAILED, XASUFW_SLHDSA_FORS_FAILED or
*		  XASUFW_SLHDSA_HT_FAILED, identifying the verification stage that failed for
*		  a reason other than a root mismatch, combined with the underlying callee status
*		  via XAsufw_UpdateErrorStatus.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or
*		  XASUFW_SHA_FINISH_FAILED).
*
* @note
*	PreHashMode, HashId and CtxLen must repeat the values used at signing time, or the
*	recomputed digest will not match. The message and context buffers are read in place, as
*	they already reside in memory visible to both client and ASUFW. The public key is staged
*	by XSlhDsa_LoadPk and the signature is copied into the signature scratch, so both are read
*	from local buffers.
*
*************************************************************************************************/
s32 XSlhDsa_Verify(XAsufw_Dma *DmaPtr, XSha *ShaPtr, const XAsu_SlhdsaSignVerifyParams *ParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(VerifyStatus, XASUFW_FAILURE);
	XFih_Var FihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XSlhDsa_Instance *InstPtr = XSlhDsa_GetInstance();
	const u8 *MsgPtr;
	const u8 *PkPtr;
	const u8 *SigPtr;
	const u8 *CtxPtr;

	/** Validate input pointers. */
	if ((DmaPtr == NULL) || (ShaPtr == NULL) || (ParamsPtr == NULL) || (InstPtr == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Validate the verify parameters, so a wrong-length signature is never parsed. */
	if (XAsu_SlhdsaValidateSignVerifyParams(ParamsPtr) != XST_SUCCESS) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Bind the singleton instance to the requested parameter set and to the engines. */
	Status = XSlhDsa_PrepareInstance(InstPtr, ParamsPtr->ParamSet, DmaPtr, ShaPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Bound the signature to the scratch it is copied into before the transfer runs. */
	if (ParamsPtr->SignatureLen > (u32)sizeof(XSlhDsa_SigScratch)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Copy the client signature into the signature scratch before verification reads it. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, ParamsPtr->SignatureAddr,
			       (u64)(UINTPTR)XSlhDsa_SigScratch, ParamsPtr->SignatureLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_SIG_IN_DMA_FAILED;
		goto END;
	}

	/**
	 * Select the message mode and resolve the message, public key, signature and context
	 * addresses. CtxAddr is zero only when CtxLen is zero, so CtxPtr is NULL exactly for
	 * the FIPS empty context (prefix 0x00 || 0x00).
	 */
	XSlhDsa_SetMsgMode(InstPtr, ParamsPtr->PreHashMode, ParamsPtr->HashId);
	MsgPtr = (const u8 *)(UINTPTR)ParamsPtr->MsgAddr;
	PkPtr = (const u8 *)(UINTPTR)ParamsPtr->PkObj.PubKeyAddr;
	SigPtr = XSlhDsa_SigScratch;
	CtxPtr = (const u8 *)(UINTPTR)ParamsPtr->CtxAddr;

	/**
	 * Clear the global ReturnStatus, the redundant indicator that XSlhDsa_VerifyCore sets
	 * to XASUFW_SLHDSA_SIGNATURE_VERIFIED, so a stale value from an earlier verification
	 * is not mistaken for this one succeeding.
	 */
	ReturnStatus = XASUFW_FAILURE;
	XFIH_CALL(XSlhDsa_VerifyInternal, FihVar, VerifyStatus, InstPtr, MsgPtr,
		  ParamsPtr->MsgLen, SigPtr, ParamsPtr->SignatureLen, CtxPtr, ParamsPtr->CtxLen,
		  PkPtr);
	if (VerifyStatus != XASUFW_SUCCESS) {
		/*
		 * Returned unchanged: a root mismatch is already reported as
		 * XASUFW_SLHDSA_VERIFY_ROOT_MISMATCH and an internal failure already carries its
		 * verification stage tag, so re-tagging here would overwrite one of only three
		 * available error fields.
		 */
		Status = VerifyStatus;
		goto END;
	}

	/**
	 * Confirm the redundant indicator as well as the returned status, so that a single
	 * glitched status value cannot force a verification pass.
	 */
	if (ReturnStatus != XASUFW_SLHDSA_SIGNATURE_VERIFIED) {
		Status = XASUFW_SLHDSA_SIGN_VERIFY_FAILED;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	/**
	 * Clear the instance state. Verification touches only public data, but the instance is
	 * shared, so it is cleared here too to keep the contract identical on every exit path.
	 */
	Status = XAsufw_UpdateBufStatus(Status, XSlhDsa_ClearSensitiveState(InstPtr));

	return Status;
}
/*************************************************************************************************/
/**
* @brief	Point the instance key views at the four fields of the staged key.
*
* @param	InstPtr	Pointer to SLH-DSA instance with a staged key region.
* @param	HashWidth	Hash width n in bytes.
*
* @note
*	The caller must have staged a complete SK.seed || SK.prf || PK.seed || PK.root image, or
*	be about to fill the remaining fields in place, because the views alias the region rather
*	than copying from it.
*
*************************************************************************************************/
static void XSlhDsa_PublishStagedSkViews(XSlhDsa_Instance *InstPtr, u32 HashWidth)
{
	u8 *KeyPtr = InstPtr->KeyScratch;

	/**
	 * Name the four fields of the staged key:
	 *
	 * - SK.seed, the secret seed every WOTS+ and FORS private value is derived from.
	 * - SK.prf, the key the message randomizer R is derived from.
	 * - PK.seed and PK.root, the embedded public key.
	 */
	InstPtr->SkSeed = &KeyPtr[XSLHDSA_SK_SEED_FIELD * HashWidth];
	InstPtr->SkPrf = &KeyPtr[XSLHDSA_SK_PRF_FIELD * HashWidth];
	InstPtr->PkSeed = &KeyPtr[XSLHDSA_SK_PKSEED_FIELD * HashWidth];
	InstPtr->PkRoot = &KeyPtr[XSLHDSA_SK_PKROOT_FIELD * HashWidth];
}
/*************************************************************************************************/
/**
* @brief	Write the secret-key prefix from caller-supplied seeds
*		(FIPS 205 Algorithm 18: slh_keygen_internal, SK layout of step 4).
*
* Populates SK.seed, SK.prf, and PK.seed fields; clears the PK.root slot pending root build.
*
* @param	SkSeed	Secret seed (n bytes).
* @param	SkPrf	PRF secret (n bytes).
* @param	PkSeed	Public seed (n bytes).
* @param	HashWidth	Hash width n in bytes.
* @param	SkOut	Output secret key buffer (4n bytes).
*
* @return
*		- XASUFW_SUCCESS, if the secret-key prefix is written.
*		- XASUFW_SLHDSA_COPY_FAILED, if the SK.seed, SK.prf or PK.seed
*		  copy fails.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_ZEROIZE_MEMSET_FAIL).
*
*************************************************************************************************/
static s32 XSlhDsa_KeyWriteSkPrefix(const u8 *SkSeed, const u8 *SkPrf,
					  const u8 *PkSeed, u32 HashWidth, u8 *SkOut)
{
	s32 Status = XASUFW_FAILURE;

	/**
	 * SK.seed occupies the first n bytes of the 4n-byte secret key. Each copy passes the
	 * remaining field count as destination bound, so a wrong offset cannot overrun SkOut.
	 */
	Status = Xil_SMemCpy(SkOut, XSLHDSA_SK_N_FIELDS * HashWidth, SkSeed,
				     HashWidth, HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_COPY_FAILED;
		goto END;
	}

	/** SK.prf follows SK.seed and keys PRF_msg when the randomizer R is generated. */
	Status = Xil_SMemCpy(SkOut + (XSLHDSA_SK_PRF_FIELD * HashWidth),
				     (XSLHDSA_SK_N_FIELDS - XSLHDSA_SK_PRF_FIELD) *
				     HashWidth, SkPrf, HashWidth, HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_COPY_FAILED;
		goto END;
	}

	/** PK.seed is duplicated inside SK so signing needs the secret key alone. */
	Status = Xil_SMemCpy(SkOut + (XSLHDSA_SK_PKSEED_FIELD * HashWidth),
				     (XSLHDSA_SK_N_FIELDS - XSLHDSA_SK_PKSEED_FIELD) *
				     HashWidth, PkSeed, HashWidth, HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_COPY_FAILED;
		goto END;
	}

	/** Clear PK.root until the top XMSS root has been generated and committed. */
	Status = Xil_SMemSet(SkOut + (XSLHDSA_SK_PKROOT_FIELD * HashWidth),
				     HashWidth, 0U, HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Draw the secret-key prefix from the TRNG
*		(FIPS 205 Algorithm 21: slh_keygen, steps 1 to 3).
*
* Fills the SK.seed, SK.prf and PK.seed fields of the staged key in place and clears the PK.root
* slot pending root build.
*
* @param	InstPtr	Pointer to SLH-DSA instance with a staged key region.
* @param	HashWidth	Hash width n in bytes.
*
* @return
*		- XASUFW_SUCCESS, if all three seeds are drawn.
*		- XASUFW_SLHDSA_KEYGEN_FAILED, if a TRNG draw fails, combined with the underlying
*		  callee status via XAsufw_UpdateErrorStatus.
*		- XASUFW_ZEROIZE_MEMSET_FAIL, if the PK.root clear fails.
*
* @note
*	The caller must hold the TRNG resource. Drawing into the staged key leaves the secret
*	prefix with no second copy to clear, so a failed draw is covered by the same staged key
*	clear as a completed one.
*
*************************************************************************************************/
static s32 XSlhDsa_KeyDrawSkPrefix(XSlhDsa_Instance *InstPtr, u32 HashWidth)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var FihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 *KeyPtr = InstPtr->KeyScratch;

	/**
	 * Generate the three n-byte seeds required by FIPS 205 key generation. XFIH_CALL detects
	 * a skipped TRNG call instead of leaving a field holding whatever preceded it.
	 */
	/** - Generate SK.seed, the seed for all secret WOTS+ and FORS key material. */
	XFIH_CALL(XTrng_GetRandomNumbers, FihVar, Status,
		  &KeyPtr[XSLHDSA_SK_SEED_FIELD * HashWidth], HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SLHDSA_KEYGEN_FAILED);
		goto END;
	}

	/** - Generate SK.prf, the secret PRF key used to randomize signatures. */
	XFIH_CALL(XTrng_GetRandomNumbers, FihVar, Status,
		  &KeyPtr[XSLHDSA_SK_PRF_FIELD * HashWidth], HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SLHDSA_KEYGEN_FAILED);
		goto END;
	}

	/** - Generate PK.seed, the public seed that domain-separates the hash functions. */
	XFIH_CALL(XTrng_GetRandomNumbers, FihVar, Status,
		  &KeyPtr[XSLHDSA_SK_PKSEED_FIELD * HashWidth], HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SLHDSA_KEYGEN_FAILED);
		goto END;
	}

	/** Clear PK.root until the top XMSS root has been generated and committed. */
	Status = Xil_SMemSet(&KeyPtr[XSLHDSA_SK_PKROOT_FIELD * HashWidth], HashWidth, 0U,
			     HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute the top hypertree XMSS root at layer (d - 1)
*		(FIPS 205 Algorithm 18: slh_keygen_internal, steps 1 to 3).
*
* Builds the full XMSS tree at the top hypertree layer; the root becomes PK.root.
*
* @param	InstPtr	Pointer to SLH-DSA instance with partial SK loaded.
* @param	ParamPtr	Active parameter set.
* @param	RootOut	Output root buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS, if top XMSS root build succeeds.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_ZEROIZE_MEMSET_FAIL,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or
*		  XASUFW_SHA_FINISH_FAILED).
*
*************************************************************************************************/
static s32 XSlhDsa_KeyBuildTopXmssRoot(XSlhDsa_Instance *InstPtr,
					    const XSlhDsa_Param *ParamPtr, u8 *RootOut)
{
	s32 Status = XASUFW_FAILURE;

	/** - Step 1: ADRS <- toByte(0, 32); tree address stays 0 for the single top tree. */
	Status = Xil_SMemSet(InstPtr->Addr, sizeof(*InstPtr->Addr), 0U, sizeof(*InstPtr->Addr));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** - Step 2: ADRS.setLayerAddress(d - 1) */
	XSlhDsa_AddrSetLayer(InstPtr->Addr, ParamPtr->d - 1U);

	/**
	 * - Step 3: PK.root <- xmss_node(SK.seed, 0, h', PK.seed, ADRS)
	 *
	 * Treehash from leaf 0 over the full h' height, so the returned node is the tree root
	 * rather than an intermediate subtree node.
	 */
	Status = XSlhDsa_XmssSubtreeRoot(InstPtr, 0, ParamPtr->xmss_height, RootOut);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Store the hypertree root into the staged key
*		(FIPS 205 Algorithm 18: slh_keygen_internal, step 4).
*
* @param	InstPtr	Pointer to SLH-DSA instance with the secret-key prefix staged.
* @param	Root	Hypertree root (n bytes).
* @param	HashWidth	Hash width n in bytes.
*
* @return
*		- XASUFW_SUCCESS, if the root is committed.
*		- XASUFW_SLHDSA_COPY_FAILED, if the root copy fails.
*
*************************************************************************************************/
static s32 XSlhDsa_KeyCommitHypertreeRoot(XSlhDsa_Instance *InstPtr,
					       const u8 *Root, u32 HashWidth)
{
	s32 Status = XASUFW_FAILURE;

	/** Fill the PK.root slot that the secret-key prefix write left zeroed. */
	Status = Xil_SMemCpy(&InstPtr->KeyScratch[XSLHDSA_SK_PKROOT_FIELD * HashWidth],
				     HashWidth, Root, HashWidth, HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_COPY_FAILED;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Sign a message digest using FORS and hypertree layers
*		(FIPS 205 Algorithms 16 + 12 within slh_sign_internal).
*
* Parses hypertree indices from the H_msg digest, emits the FORS signature, reconstructs
* Pk_FORS into the yield-session layer-message buffer, then signs through the hypertree.
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	Digest	H_msg digest to sign (m bytes).
* @param	SigBody	Output buffer for FORS and hypertree signature data (after randomizer R).
*
* @return
*		- XASUFW_SUCCESS, if digest signing succeeds.
*		- XASUFW_CMD_IN_PROGRESS, if a cooperative yield slice expired and digest
*		  signing will resume.
*		- XASUFW_SLHDSA_FORS_FAILED, if FORS signature emission or the Pk_FORS
*		  reconstruction fails, combined with the underlying callee status via
*		  XAsufw_UpdateErrorStatus.
*		- XASUFW_SLHDSA_HT_FAILED, if hypertree signing fails, combined with the
*		  underlying callee status via XAsufw_UpdateErrorStatus.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_ZEROIZE_MEMSET_FAIL,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or
*		  XASUFW_SHA_FINISH_FAILED).
*
* @note
*	Advances SigBody past the FORS region before hypertree signing (FIPS SIG_HT layout).
*
*************************************************************************************************/
static s32 XSlhDsa_SignDigestBody(XSlhDsa_Instance *InstPtr, const u8 *Digest,
				       u8 *SigBody)
{
	s32 Status = XASUFW_FAILURE;
	u8 SkipFors = 0U;
	const XSlhDsa_Param *ParamPtr;
	XSlhDsa_YieldCtx *YieldPtr;
	XSlhDsa_HtIndices HtIndices;
	u32 ForsBytes;
	u8 *SigHt;

	/** SIG_FORS spans k * (1 + a) * n bytes, so SIG_HT starts at that offset in the body. */
	ParamPtr = &InstPtr->Param;
	YieldPtr = &InstPtr->Yield;
	ForsBytes = ParamPtr->fors_count * (1U + ParamPtr->fors_height) * ParamPtr->n;
	SigHt = SigBody + ForsBytes;

	/**
	 * - Step 6:  md <- digest[0 : ceil(k * a / 8)]
	 * - Step 7 to 10: extract tmp_idxtree / tmp_idxleaf and reduce them to idxtree and
	 *   idxleaf.
	 */
	XSlhDsa_HtParseDigest(ParamPtr, Digest, &HtIndices);

	if ((YieldPtr->Ht.Resume != 0U) && (YieldPtr->Ht.SigBase == SigHt)) {
		SkipFors = 1U;
	} else if ((YieldPtr->Xmss.EmitResume != 0U) && (YieldPtr->Xmss.EmitLayerSign == SigHt)) {
		SkipFors = 1U;
	}

	if (SkipFors == 0U) {
		/** - Step 1: ADRS <- toByte(0, 32) */
		Status = Xil_SMemSet(InstPtr->Addr, sizeof(*InstPtr->Addr), 0U,
				     sizeof(*InstPtr->Addr));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_ZEROIZE_MEMSET_FAIL;
			goto END;
		}

		/**
		 * - Step 11: ADRS.setTreeAddress(idxtree)
		 * - Step 12: ADRS.setTypeAndClear(FORS_TREE)
		 * - Step 13: ADRS.setKeyPairAddress(idxleaf)
		 */
		XSlhDsa_AddrSetTree(InstPtr->Addr, HtIndices.IdxTree);
		XSlhDsa_AddrSetTypeClearTail(InstPtr->Addr, XSLHDSA_ADRS_FORS_TREE);
		XSlhDsa_AddrSetKeyPair(InstPtr->Addr, (u32)HtIndices.IdxLeaf);

		/**
		 * - Step 14: SIG_FORS <- fors_sign(md, SK.seed, PK.seed, ADRS)
		 *
		 * Emits the k FORS secrets and authentication paths into SIG_FORS.
		 */
		Status = XSlhDsa_ForsEmitSig(InstPtr, Digest, SigBody);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			goto END;
		}
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SLHDSA_FORS_FAILED);
			goto END;
		}

		/** Re-init ADRS to the steps 11 to 13 state that fors_pkFromSig expects. */
		Status = Xil_SMemSet(InstPtr->Addr, sizeof(*InstPtr->Addr), 0U,
				     sizeof(*InstPtr->Addr));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_ZEROIZE_MEMSET_FAIL;
			goto END;
		}

		XSlhDsa_AddrSetTree(InstPtr->Addr, HtIndices.IdxTree);
		XSlhDsa_AddrSetTypeClearTail(InstPtr->Addr, XSLHDSA_ADRS_FORS_TREE);
		XSlhDsa_AddrSetKeyPair(InstPtr->Addr, (u32)HtIndices.IdxLeaf);

		/**
		 * - Step 16: PK_FORS <- fors_pkFromSig(SIG_FORS, md, PK.seed, ADRS)
		 *
		 * Recomputing Pk_FORS from the emitted signature makes the hypertree sign exactly
		 * what a verifier will reconstruct. The result is stored on Yield so it survives
		 * a cooperative yield unwind.
		 */
		Status = XSlhDsa_ForsPkFromSig(InstPtr, SigBody, Digest, YieldPtr->Ht.LayerMsg);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SLHDSA_FORS_FAILED);
			goto END;
		}
	}

	/**
	 * - Step 17: SIG_HT <- ht_sign(PK_FORS, SK.seed, PK.seed, idxtree, idxleaf)
	 */
	Status = XSlhDsa_HtEmitSig(InstPtr, &HtIndices, YieldPtr->Ht.LayerMsg, SigHt);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		goto END;
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SLHDSA_HT_FAILED);
	}

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Map an ASU parameter set identifier to the internal parameter set type.
*
* @param	ParamSet	ASU SLH-DSA parameter set identifier.
* @param	TypePtr	Pointer to receive the mapped parameter set type.
*
* @return
*		- XASUFW_SUCCESS, if ParamSet maps to a supported parameter set.
*		- XASUFW_SLHDSA_INVALID_PARAM, if TypePtr is NULL or ParamSet is unrecognized.
*		- XASUFW_SLHDSA_LOOKUP_GLITCH_ERROR, if the redundant parameter-set check after
*		  the switch does not match.
*
*************************************************************************************************/
static s32 XSlhDsa_ParamSetFromAsu(u8 ParamSet, XSlhDsa_ParamSet *TypePtr)
{
	s32 Status = XASUFW_FAILURE;
	CREATE_VOLATILE(TmpParamSet, (s32)XASUFW_ALLFS);

	/** Validate input pointer. */
	if (TypePtr == NULL) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * Default to the out-of-range parameter set, so a glitch that skips the switch below
	 * leaves an unusable value rather than a valid parameter set.
	 */
	*TypePtr = XSLHDSA_PARAM_SET_MAX;

	/** Map the ASU parameter set to the internal type, recording it in a volatile copy. */
	switch (ParamSet) {
		case XASU_SLHDSA_PARAM_SHA2_128S: {
			*TypePtr = XSLHDSA_PARAM_SET_SHA2_128S;
			TmpParamSet = XASU_SLHDSA_PARAM_SHA2_128S;
			break;
		}
		case XASU_SLHDSA_PARAM_SHA2_256S: {
			*TypePtr = XSLHDSA_PARAM_SET_SHA2_256S;
			TmpParamSet = XASU_SLHDSA_PARAM_SHA2_256S;
			break;
		}
		case XASU_SLHDSA_PARAM_SHAKE_128S: {
			*TypePtr = XSLHDSA_PARAM_SET_SHAKE_128S;
			TmpParamSet = XASU_SLHDSA_PARAM_SHAKE_128S;
			break;
		}
		case XASU_SLHDSA_PARAM_SHAKE_256S: {
			*TypePtr = XSLHDSA_PARAM_SET_SHAKE_256S;
			TmpParamSet = XASU_SLHDSA_PARAM_SHAKE_256S;
			break;
		}
		default: {
			Status = XASUFW_SLHDSA_INVALID_PARAM;
			goto END;
		}
	}

	/**
	 * Re-check the volatile copy against the input to confirm the case actually taken
	 * matches the request, detecting a glitched or skipped branch.
	 */
	if (TmpParamSet != ParamSet) {
		Status = XASUFW_SLHDSA_LOOKUP_GLITCH_ERROR;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Run the FIPS 140-3 IG 10.3.A pairwise consistency test on a freshly generated
*		SLH-DSA key pair: sign a fixed empty message with the new secret key and verify
*		that signature with the new public key. Unlike the KAT, which validates the
*		algorithm against a fixed vector, this validates the specific pair about to be
*		handed to the client.
*
* @param	InstPtr	Pointer to the SLH-DSA instance holding the staged key pair.
*
* @return
*		- XASUFW_SUCCESS, if the signature verifies under the generated public key.
*		- XASUFW_SLHDSA_INVALID_PARAM, if InstPtr, its staged key or work memory is NULL,
*		  or the bound parameter set signature size does not fit the PWCT scratch.
*		- XASUFW_CMD_IN_PROGRESS, if a cooperative yield slice expired and the PWCT sign
*		  will resume.
*		- XASUFW_SLHDSA_PWCT_SIGN_GEN_FAIL, if the test signature cannot be produced,
*		  combined with the underlying callee status via XAsufw_UpdateErrorStatus.
*		- XASUFW_SLHDSA_PWCT_SIGN_VER_FAIL, if the test signature does not verify, or if
*		  the redundant verified indicator does not confirm success, combined with the
*		  underlying callee status via XAsufw_UpdateErrorStatus.
*
* @note
*	Pure SLH-DSA, empty context, empty message, deterministic add_rand (PK.seed). The test
*	needs no extra TRNG entropy, and key generation has already drawn the three FIPS seeds.
*	Verification does not call XSlhDsa_LoadPk, which would overwrite SK.seed and SK.prf in
*	the staged key before the pair is exported.
*
*************************************************************************************************/
static s32 XSlhDsa_Pwct(XSlhDsa_Instance *InstPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(SignStatus, XASUFW_FAILURE);
	CREATE_VOLATILE(VerifyStatus, XASUFW_FAILURE);
	XFih_Var FihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	const u8 *PkPtr;
	u32 SigLen;
	u32 HashWidth;

	/** Validate the instance and the staged key the test signs with. */
	if ((InstPtr == NULL) || (InstPtr->KeyScratch == NULL) || (InstPtr->WorkMem == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * Bound the test signature to the overlay scratch. A zero size is an unbound instance;
	 * a size past the scratch would write past .overlay_slhdsa_data.
	 */
	SigLen = XSlhDsa_SignatureBytes(&InstPtr->Param);
	if ((SigLen == 0U) || (SigLen > (u32)sizeof(XSlhDsa_SigScratch))) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	HashWidth = InstPtr->Param.n;
	PkPtr = &InstPtr->KeyScratch[XSLHDSA_SK_PKSEED_FIELD * HashWidth];

	/** Pure SLH-DSA, empty context, empty message, deterministic add_rand. */
	XSlhDsa_SetMsgMode(InstPtr, XASU_SLHDSA_MSG_PURE, 0U);

	ASSIGN_VOLATILE(SignStatus, XASUFW_FAILURE);
	XFIH_CALL(XSlhDsa_SignInternal, FihVar, SignStatus, InstPtr, XSlhDsa_PwctEmptyMsg,
		  0U, NULL, 0U, InstPtr->KeyScratch, NULL, XSlhDsa_SigScratch, SigLen);
	if (SignStatus == XASUFW_CMD_IN_PROGRESS) {
		Status = SignStatus;
		goto END;
	}
	if (SignStatus != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(SignStatus, XASUFW_SLHDSA_PWCT_SIGN_GEN_FAIL);
		goto END;
	}

	/**
	 * Verify against the PK half of the staged SK. LoadPk would copy PK onto the start of
	 * KeyScratch and overwrite SK.seed and SK.prf before the pair is exported.
	 */
	XSlhDsa_SetPkAddr(InstPtr, PkPtr);
	XSlhDsa_SetMsg(InstPtr, XSlhDsa_PwctEmptyMsg, 0U);
	XSlhDsa_SetSig(InstPtr, XSlhDsa_SigScratch, SigLen);

	/** Stage the FIPS empty-context prefix 0x00 || 0x00 in WorkMem. */
	InstPtr->WorkMem[XASU_SLHDSA_PREFIX_DS_OFFSET] = XASU_SLHDSA_DS_PURE;
	InstPtr->WorkMem[XASU_SLHDSA_PREFIX_CTXLEN_OFFSET] = 0U;
	InstPtr->CtxPrefixLen = XASU_SLHDSA_PREFIX_HDR_LEN;

	ReturnStatus = XASUFW_FAILURE;
	ASSIGN_VOLATILE(VerifyStatus, XASUFW_FAILURE);
	XFIH_CALL(XSlhDsa_VerifyCore, FihVar, VerifyStatus, InstPtr);
	if (VerifyStatus != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(VerifyStatus, XASUFW_SLHDSA_PWCT_SIGN_VER_FAIL);
		goto END;
	}

	/**
	 * Confirm the redundant indicator as well as the returned status, so that a single
	 * glitched status value cannot force a pairwise consistency pass.
	 */
	if (ReturnStatus != XASUFW_SLHDSA_SIGNATURE_VERIFIED) {
		Status = XASUFW_SLHDSA_PWCT_SIGN_VER_FAIL;
		goto END;
	}

	/**
	 * The verified indicator was checked above. The command layer copies ReturnStatus into
	 * the keygen response, so clear it before this test returns.
	 */
	ReturnStatus = XASUFW_FAILURE;

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Prepare the singleton SLH-DSA instance for one request.
*
* @param	InstPtr	Pointer to the SLH-DSA instance.
* @param	ParamSet	ASU SLH-DSA parameter set identifier.
* @param	DmaPtr	Pointer to the DMA instance allocated for the request.
* @param	ShaPtr	Pointer to the SHA instance matching the requested parameter set.
*
* @return
*		- XASUFW_SUCCESS, if the instance is prepared successfully.
*		- XASUFW_SLHDSA_INVALID_PARAM, if ParamSet is unrecognized or the bind fails.
*		- XASUFW_SLHDSA_LOOKUP_GLITCH_ERROR, if a redundant parameter-set check during
*		  mapping or lookup does not match.
*		- Error code received from called functions in case of other failure from the
*		  called function.
*
* @note
*	Binds the parameter set and attaches the SHA and DMA engines reserved for the request.
*	The instance is a singleton, so both are refreshed per command.
*
*************************************************************************************************/
static s32 XSlhDsa_PrepareInstance(XSlhDsa_Instance *InstPtr, u8 ParamSet, XAsufw_Dma *DmaPtr,
					XSha *ShaPtr)
{
	s32 Status = XASUFW_FAILURE;
	XSlhDsa_ParamSet Type;

	/** Map the client supplied parameter set identifier to the internal type. */
	Status = XSlhDsa_ParamSetFromAsu(ParamSet, &Type);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Bind the parameter set, filling in the FIPS 205 parameters all sizes derive from. */
	Status = XSlhDsa_BindParam(InstPtr, Type);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Attach the SHA and DMA engines the resource handler reserved for this request. */
	InstPtr->ShaInst = ShaPtr;
	InstPtr->DmaInst = DmaPtr;

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
#endif /* XASU_SLHDSA_ENABLE */
/** @} */
