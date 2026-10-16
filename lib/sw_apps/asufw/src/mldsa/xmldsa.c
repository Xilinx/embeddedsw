/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xmldsa.c
 *
 * This file contains the implementation of the ML-DSA (FIPS 204) core wrapper for ASUFW over the
 * liboqs "mldsa-native" backend. It supports the full FIPS 204 surface:
 *   - Pure ML-DSA and HashML-DSA (pre-hash) signing/verification,
 *   - Hedged (randomized) and deterministic signing,
 *   - Context strings (0..255 bytes),
 *   - Seed-based key generation (explicit xi seed) or TRNG-sourced seed.
 *
 * The wrapper takes the client request parameter structure directly and a DMA instance. Because
 * the ASU is a 32-bit core and clients pass 64-bit DDR addresses, the fixed-size buffers (keys,
 * signature, context, seed) are marshalled through DMA into the ASU-addressable PQC scratch
 * region (see xasufw_memory.h). Arbitrary-length messages (up to the DMA hardware maximum) are
 * never buffered locally: they are streamed directly from the client address through the ASU
 * hardware SHA engines.
 *
 * liboqs's generic OQS_SIG API exposes only pure + context signing; the extended surface is
 * implemented here by calling the lower-level internal entry points declared in
 * oqs_mldsa_internal.h. For the pure path the FIPS 204 message representative (mu) is computed on
 * the ASU hardware SHAKE256 engine so the message can be streamed, and mu is handed to the
 * backend using its "external mu" variant (ExternalMu = 1). For HashML-DSA the message pre-hash
 * is computed on the ASU hardware SHA-2/SHA-3 engines (so liboqs's software hashes are not
 * needed) and the (small) digest is handed to the backend's pre-hash internal entry point.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/20/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xmldsa_server_apis ML-DSA Server APIs
 * @{
 */
/*************************************** Include Files *******************************************/
#include "xmldsa.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_dma.h"
#include "xasufw_memory.h"
#include "xtrng.h"
#include "xil_util.h"
#include "xfih.h"
#include "xsha.h"
#include "xsha_hw.h"
#include "xasu_def.h"
#include "xasu_mldsa_common.h"
#include "xasufw_perf.h"

#ifdef XASU_MLDSA_ENABLE
/* Ships with the liboqs drop, which the BSP only provides when ML-DSA is enabled. */
#include "oqs_mldsa_internal.h"

/************************************ Constant Definitions ***************************************/
#define XMLDSA_MAX_PREHASH_LEN		(64U) /**< Maximum supported pre-hash digest length */
/*
 * SHAKE256 is an extendable-output function, so unlike the fixed SHA2/SHA3 digests its length is
 * not implied by the mode. FIPS 204 HashML-DSA fixes the SHAKE256 pre-hash at 64 bytes.
 */
#define XMLDSA_PREHASH_SHAKE256_LEN	(64U) /**< FIPS 204 SHAKE256 pre-hash output length */
#define XMLDSA_BACKEND_SUCCESS		(0)   /**< mldsa-native success return value */
#define XMLDSA_BACKEND_FAILURE		(-1)  /**< Local failure value: dispatcher default, taken
						for an unsupported parameter set */
#define XMLDSA_TR_LEN			(MLDSA_CRHBYTES) /**< tr length in bytes (64) */

/*
 * Non-blocking resume stages for the Sign/Verify state machine. When a large client message is
 * streamed larger than XASUFW_DMA_BLOCKING_SIZE, XSha runs the DMA non-blocking and returns
 * XASUFW_CMD_IN_PROGRESS. The command is then re-dispatched on DMA completion; CmdStage records
 * whether the one-time prelude (secret/context DMA, signing randomness) has already run so it is
 * not repeated on the resume pass, and which streaming operation must be finished:
 *   - DIGEST_RESUME: HashML-DSA message pre-hash (XSha_Digest) must be finished.
 *   - MU_RESUME: pure-mode mu = SHAKE256(tr || pre || msg) must be finished (XSha_Finish).
 */
#define XMLDSA_STAGE_INIT		(0U) /**< Initial call: run the one-time prelude */
#define XMLDSA_STAGE_DIGEST_RESUME	(1U) /**< Resume: pre-hash DMA completed, finish digest */
#define XMLDSA_STAGE_MU_RESUME		(2U) /**< Resume: mu message DMA completed, finish mu */

/*
 * Offset of tr within the packed secret key sk = (rho || key || tr || s1 || s2 || t0). rho and
 * key are MLDSA_SEEDBYTES (32) each for all parameter sets, so tr always starts at byte 64.
 */
#define XMLDSA_SK_TR_OFFSET		(MLDSA_SEEDBYTES + MLDSA_SEEDBYTES) /**< tr offset = 64 */

/*
 * Layout of the scratch region declared in xmldsa.h, for signing and verification. The three slots
 * are sized to the worst-case field placed in each and are reused across those two operations:
 *   - KEY slot: secret key (sign) / public key (verify).
 *   - SIG slot: signature (sign/verify).
 *   - CTX slot: context string (sign/verify).
 * Key generation carves the same region its own way, into the four XMLDSA_KEYGEN_* offsets in
 * xmldsa.h; the two layouts never coexist because the operations are mutually exclusive.
 */
#define XMLDSA_SLOT_KEY_OFFSET		(0U)
#define XMLDSA_SLOT_SIG_OFFSET		(XASU_MLDSA_MAX_PRIV_KEY_LEN)
#define XMLDSA_SLOT_CTX_OFFSET		(XASU_MLDSA_MAX_PRIV_KEY_LEN + XASU_MLDSA_MAX_SIGN_LEN)

/*
 * Each carve is bound to a budget that does not grow with it, so adding a buffer to either fails
 * the build rather than silently running off the end of the region. The slot layout above is bound
 * to the sign footprint declared in xmldsa.h, and the key generation carve, which is in xmldsa.h
 * since the caller reads the pair out of it, to the pool reserved in lscript.ld. Bounding either by
 * XMLDSA_SCRATCH_LEN would catch nothing, as that is the larger of the two footprints and so grows
 * with whichever carve is being checked.
 */
_Static_assert((XMLDSA_SLOT_CTX_OFFSET + XASU_MLDSA_MAX_CTX_LEN) <= XMLDSA_SIGN_SCRATCH_LEN,
	       "ML-DSA sign/verify carve overruns the sign footprint declared in xmldsa.h");
_Static_assert(XMLDSA_KEYGEN_SCRATCH_LEN <= XASUFW_CRYPTO_SHARED_POOL_SIZE,
	       "ML-DSA key generation carve overruns the shared crypto pool reserved in lscript.ld");
_Static_assert((XMLDSA_SCRATCH_END - XASUFW_CRYPTO_ALGO_DATA_BLOCK) <=
	       XASUFW_CRYPTO_SHARED_POOL_SIZE,
	       "ML-DSA scratch region overruns the shared crypto pool reserved in lscript.ld");

/************************************** Type Definitions *****************************************/
/**
 * @brief Location of a DMA-streamed message for the ML-DSA hashing helpers.
 */
typedef struct {
	u64 Addr;	/**< Message address (may be a 64-bit client address; always non-zero). */
	u32 Len;	/**< Message length in bytes (0 is permitted). */
} XMldsa_MsgRef;

/**
 * @brief Inputs absorbed while computing the FIPS 204 message representative
 *	  mu = SHAKE256(tr || pre || msg). Ignored on a non-blocking resume pass, where the
 *	  SHAKE256 engine has already absorbed them.
 */
typedef struct {
	const u8 *Tr;		/**< 64-byte tr (sign: sk[64:128]; verify: SHAKE256(pk)). */
	const u8 *Pre;		/**< Domain-separation prefix absorbed after tr. */
	u32 PreLen;		/**< Length of the domain-separation prefix in bytes. */
	XMldsa_MsgRef Msg;	/**< Message streamed after the prefix. */
} XMldsa_MuInput;

/**
 * @brief Result of the HashML-DSA message pre-hash. Also describes the pre-hash inputs to the
 *	  domain-separation prefix builder, where a NULL pointer selects pure (non-pre-hash) mode.
 */
typedef struct {
	u8 *Digest;	/**< Output digest buffer (>= XMLDSA_MAX_PREHASH_LEN bytes). */
	u32 DigestLen;	/**< Produced digest length in bytes. */
	s32 MldHashAlg;	/**< mldsa-native MLD_PREHASH_* constant for the digest. */
} XMldsa_PreHashOut;

/*
 * Request descriptors for the mldsa-native dispatchers below. mldsa-native is a third-party API
 * whose entry points take size_t/int and up to nine positional arguments, several of them
 * adjacent pointer/length pairs. Grouping them here keeps the ASUFW side on fixed-width types and
 * removes the risk of transposing two same-typed arguments at a call site; each dispatcher
 * converts to the third-party types at the single point where it calls into the backend.
 */

/**
 * @brief Inputs to the pure / external-mu ML-DSA signature dispatch.
 */
typedef struct {
	u8 *Sig;		/**< Output signature buffer. */
	u32 *SigLen;		/**< Out: produced signature length in bytes. */
	const u8 *Msg;		/**< Message, or the precomputed mu when ExternalMu is XASU_TRUE. */
	u32 MsgLen;		/**< Length of Msg in bytes. */
	const u8 *Pre;		/**< Domain-separation prefix (NULL in external-mu mode). */
	u32 PreLen;		/**< Length of Pre in bytes. */
	const u8 *Rnd;		/**< Signing randomness (MLDSA_RNDBYTES; all-zero = deterministic). */
	const u8 *PrivKey;	/**< Packed secret key. */
	u32 ExternalMu;		/**< XASU_TRUE when Msg holds a precomputed mu. */
} XMldsa_SignReq;

/**
 * @brief Inputs to the HashML-DSA (pre-hash) signature dispatch.
 */
typedef struct {
	u8 *Sig;		/**< Output signature buffer. */
	u32 *SigLen;		/**< Out: produced signature length in bytes. */
	const u8 *PreHash;	/**< Pre-hash digest of the message. */
	u32 PreHashLen;		/**< Length of PreHash in bytes. */
	const u8 *Ctx;		/**< Context string (NULL when CtxLen is 0). */
	u32 CtxLen;		/**< Length of the context string in bytes. */
	const u8 *Rnd;		/**< Signing randomness (MLDSA_RNDBYTES; all-zero = deterministic). */
	const u8 *PrivKey;	/**< Packed secret key. */
	s32 HashAlg;		/**< mldsa-native MLD_PREHASH_* constant. */
} XMldsa_SignPreHashReq;

/**
 * @brief Inputs to the pure / external-mu ML-DSA verification dispatch.
 */
typedef struct {
	const u8 *Sig;		/**< Signature to verify. */
	u32 SigLen;		/**< Length of Sig in bytes. */
	const u8 *Msg;		/**< Message, or the precomputed mu when ExternalMu is XASU_TRUE. */
	u32 MsgLen;		/**< Length of Msg in bytes. */
	const u8 *Pre;		/**< Domain-separation prefix (NULL in external-mu mode). */
	u32 PreLen;		/**< Length of Pre in bytes. */
	const u8 *PubKey;	/**< Packed public key. */
	u32 ExternalMu;		/**< XASU_TRUE when Msg holds a precomputed mu. */
} XMldsa_VerifyReq;

/**
 * @brief Inputs to the HashML-DSA (pre-hash) verification dispatch.
 */
typedef struct {
	const u8 *Sig;		/**< Signature to verify. */
	u32 SigLen;		/**< Length of Sig in bytes. */
	const u8 *PreHash;	/**< Pre-hash digest of the message. */
	u32 PreHashLen;		/**< Length of PreHash in bytes. */
	const u8 *Ctx;		/**< Context string (NULL when CtxLen is 0). */
	u32 CtxLen;		/**< Length of the context string in bytes. */
	const u8 *PubKey;	/**< Packed public key. */
	s32 HashAlg;		/**< mldsa-native MLD_PREHASH_* constant. */
} XMldsa_VerifyPreHashReq;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XMldsa_GetLengths(u8 ParamSet, u32 *PubKeyLen, u32 *PrivKeyLen, u32 *SignLen);
static s32 XMldsa_HwDigest(XSha *ShaPtr, u32 ShaMode, XAsufw_Dma *DmaPtr,
	const XMldsa_MsgRef *Data, u8 *Hash, u32 HashLen);
static s32 XMldsa_HwComputeMu(XSha *Sha3Ptr, XAsufw_Dma *DmaPtr, const XMldsa_MuInput *MuIn,
	u8 *Mu, u32 Resume);
static s32 XMldsa_ComputePreHash(u8 ShaType, u8 ShaMode, XSha *Sha2Ptr, XSha *Sha3Ptr,
	XAsufw_Dma *DmaPtr, const XMldsa_MsgRef *Msg, XMldsa_PreHashOut *Out);
static s32 XMldsa_KeypairInternal(u8 ParamSet, u8 *PubKey, u8 *PrivKey, const u8 *Seed);
static s32 XMldsa_SignatureInternal(u8 ParamSet, const XMldsa_SignReq *Req);
static s32 XMldsa_SignaturePreHashInternal(u8 ParamSet, const XMldsa_SignPreHashReq *Req);
static s32 XMldsa_VerifyInternal(u8 ParamSet, const XMldsa_VerifyReq *Req);
static s32 XMldsa_VerifyPreHashInternal(u8 ParamSet, const XMldsa_VerifyPreHashReq *Req);
static u32 XMldsa_PrepareDsPrefix(u8 ParamSet, u8 *Prefix, const XMldsa_PreHashOut *PreHash,
	const u8 *Ctx, u32 CtxLen);
static s32 XMldsa_Pwct(u8 ParamSet, XSha *Sha3Ptr, XAsufw_Dma *DmaPtr, const u8 *PubKey,
	u32 ExpPubKeyLen, const u8 *PrivKey, u8 *Sig, u32 ExpSignLen);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function generates an ML-DSA key pair for the requested parameter set. The keys
 *		are left in the scratch region, at XMLDSA_KEYGEN_SK_OFFSET and
 *		XMLDSA_KEYGEN_PK_OFFSET, for the caller to consume and then clear.
 *
 * @param	DmaPtr		Pointer to the ASU DMA instance used to marshal client buffers.
 * @param	Sha3Ptr		SHA-3/SHAKE256 engine instance (XSHA_1), used by the pairwise
 *				consistency test to compute mu. Key generation needs no other engine.
 * @param	ParamsPtr	Pointer to the ML-DSA key generation parameters. If SeedAddr is
 *				non-zero, seed-based KeyGen (explicit 32-byte xi seed) is used;
 *				otherwise the seed is drawn from the TRNG.
 *
 * @return
 *	- XASUFW_SUCCESS on success.
 *	- XASUFW_MLDSA_INVALID_PARAM if inputs are invalid.
 *	- XASUFW_MLDSA_UNSUPPORTED_PARAM_SET if the parameter set is unsupported.
 *	- XASUFW_MLDSA_KEYGEN_FAILED if key generation fails.
 *	- XASUFW_MLDSA_RANDOM_GEN_FAILED, folded with the TRNG error, if the seed cannot be drawn.
 *	- XASUFW_MLDSA_PWCT_SIGN_GEN_FAIL or XASUFW_MLDSA_PWCT_SIGN_VER_FAIL if the pairwise
 *	  consistency test on the generated pair fails, in which case the pair must not be used.
 *	- XASUFW_DMA_COPY_FAIL if the seed transfer fails.
 *
 *************************************************************************************************/
s32 XMldsa_KeyGen(XAsufw_Dma *DmaPtr, XSha *Sha3Ptr, const XMldsa_KeyGenParams *ParamsPtr)
{
	/**
	 * Capture the start time of the ML-DSA key generation operation, if performance
	 * measurement is enabled. The reported time includes the pairwise consistency test.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_MLDSA_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 ExpPubKeyLen = 0U;
	u32 ExpPrivKeyLen = 0U;
	u32 ExpSignLen = 0U;
	u8 *Base = NULL;
	u8 *LocalPrivKey;
	u8 *LocalPubKey;
	u8 *LocalSig;
	u8 *LocalSeed;
	const u8 *SeedPtr;

	if ((DmaPtr == NULL) || (Sha3Ptr == NULL) || (ParamsPtr == NULL) ||
		(ParamsPtr->PubKeyAddr == 0U) || (ParamsPtr->PrivKeyAddr == 0U)) {
		Status = XASUFW_MLDSA_INVALID_PARAM;
		goto END;
	}

	Status = XMldsa_GetLengths(ParamsPtr->ParamSet, &ExpPubKeyLen, &ExpPrivKeyLen, &ExpSignLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLDSA_KEYGEN_FAILED);
		goto END;
	}

	if ((ParamsPtr->PubKeyLen < ExpPubKeyLen) || (ParamsPtr->PrivKeyLen < ExpPrivKeyLen)) {
		Status = XASUFW_MLDSA_INVALID_PARAM;
		goto END;
	}

	Base = (u8 *)(UINTPTR)XMLDSA_SCRATCH_ADDR;
	LocalPrivKey = Base + XMLDSA_KEYGEN_SK_OFFSET;
	LocalPubKey = Base + XMLDSA_KEYGEN_PK_OFFSET;
	LocalSig = Base + XMLDSA_KEYGEN_SIG_OFFSET;
	LocalSeed = Base + XMLDSA_KEYGEN_SEED_OFFSET;

	/** Resolve the xi seed: DMA in an explicit seed if provided, else draw one from the TRNG. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (ParamsPtr->SeedAddr != 0U) {
		if (ParamsPtr->SeedLen != MLDSA_SEEDBYTES) {
			Status = XASUFW_MLDSA_INVALID_PARAM;
			goto END;
		}
		Status = XAsufw_DmaXfr(DmaPtr, ParamsPtr->SeedAddr, (u64)(UINTPTR)LocalSeed,
			MLDSA_SEEDBYTES, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}
	} else {
		Status = XTrng_GetRandomNumbers(LocalSeed, MLDSA_SEEDBYTES);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_MLDSA_RANDOM_GEN_FAILED);
			goto END;
		}
	}
	SeedPtr = LocalSeed;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XMldsa_KeypairInternal(ParamsPtr->ParamSet, LocalPubKey, LocalPrivKey, SeedPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLDSA_KEYGEN_FAILED);
		goto END;
	}

	/**
	 * Run the pairwise consistency test before the caller is told the pair is ready, so an
	 * inconsistent pair is never stored or handed out.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XMldsa_Pwct(ParamsPtr->ParamSet, Sha3Ptr, DmaPtr, LocalPubKey, ExpPubKeyLen,
		LocalPrivKey, LocalSig, ExpSignLen);

	/**
	 * The pair stays where the backend wrote it. The caller reads it at XMLDSA_KEYGEN_SK_OFFSET
	 * and XMLDSA_KEYGEN_PK_OFFSET and clears it once the keys are stored, so it is never
	 * copied to a second location.
	 */

END:
	/**
	 * Measure and print the performance time for the ML-DSA key generation operation, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_MLDSA_ID);

	/**
	 * Do not leave the xi seed or the pairwise consistency test signature in the scratch
	 * region. The seed reconstructs the whole pair; the test signature is not secret, but it
	 * is a live signature under the caller's new key and the region is shared with ML-KEM.
	 */
	if (Base != NULL) {
		XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus,
			Base + XMLDSA_KEYGEN_SEED_OFFSET, MLDSA_SEEDBYTES);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
		XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus,
			Base + XMLDSA_KEYGEN_SIG_OFFSET, ExpSignLen);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates an ML-DSA signature over the provided message, supporting
 *		pure ML-DSA and HashML-DSA (pre-hash), hedged or deterministic randomness, and an
 *		optional context string. The secret key and context are DMA'd in, the message is
 *		streamed through the ASU hardware SHA engines, and the signature is DMA'd out.
 *
 * @param	DmaPtr		Pointer to the ASU DMA instance used to marshal client buffers.
 * @param	Sha2Ptr		SHA-2 engine instance (XSHA_0), used for SHA-2 HashML-DSA pre-hash.
 *				The SHA-3 engine is resolved internally; see the note in the body.
 * @param	ParamsPtr	Pointer to the ML-DSA signature generation parameters.
 * @param	RndPtr		Explicit signing randomness rnd, or NULL to let the Deterministic
 *				field of ParamsPtr decide between an all-zero rnd and TRNG bytes.
 *				Only an algorithm validation harness supplies it, so that the ACVP
 *				hedged signature vectors can be reproduced.
 * @param	RndLen		Length of RndPtr in bytes; must equal MLDSA_RNDBYTES when it is
 *				supplied, and is ignored when RndPtr is NULL.
 *
 * @return
 *	- XASUFW_SUCCESS on success.
 *	- XASUFW_CMD_IN_PROGRESS if a large message streams in DMA non-blocking mode.
 *	- XASUFW_MLDSA_INVALID_PARAM if inputs are invalid.
 *	- XASUFW_MLDSA_UNSUPPORTED_PARAM_SET if the parameter set is unsupported.
 *	- XASUFW_MLDSA_UNSUPPORTED_SIGN_MODE if the sign mode is unsupported.
 *	- XASUFW_MLDSA_UNSUPPORTED_PREHASH_ALG if the pre-hash algorithm is unsupported.
 *	- XASUFW_MLDSA_PREHASH_FAILED if the message pre-hash / mu computation fails.
 *	- XASUFW_MLDSA_SIGN_FAILED if signature generation fails.
 *	- XASUFW_MLDSA_RANDOM_GEN_FAILED, folded with the TRNG error, if hedged randomness cannot
 *	  be drawn.
 *	- XASUFW_DMA_COPY_FAIL if a secret key, context or signature transfer fails.
 *
 *************************************************************************************************/
s32 XMldsa_Sign(XAsufw_Dma *DmaPtr, XSha *Sha2Ptr, const XAsu_MldsaSignVerifyParams *ParamsPtr,
	const u8 *RndPtr, u32 RndLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 ExpPubKeyLen = 0U;
	u32 ExpPrivKeyLen = 0U;
	u32 ExpSignLen = 0U;
	u32 CtxLen;
	/*
	 * CmdStage and Rnd are function-local statics so they survive the non-blocking suspend/resume
	 * of the streaming SHA work, which is the HashML-DSA message pre-hash in pre-hash mode and the
	 * mu computation in pure mode. This is safe because the MLDSA and SHA resources are
	 * held for the whole command, so only one ML-DSA sign runs at a time. Rnd (32 bytes) is drawn
	 * once in the prelude and zeroized on completion.
	 */
	static u32 CmdStage = XMLDSA_STAGE_INIT;
	static u8 Rnd[MLDSA_RNDBYTES];
	u8 PreHash[XMLDSA_MAX_PREHASH_LEN];
	u8 Mu[MLDSA_CRHBYTES];
	u8 Prefix[MLD_DOMAIN_SEPARATION_MAX_BYTES];
	XMldsa_PreHashOut PreHashOut = {PreHash, 0U, (s32)MLD_PREHASH_NONE};
	XMldsa_MsgRef MsgRef;
	XMldsa_MuInput MuIn;
	XMldsa_SignReq SignReq;
	XMldsa_SignPreHashReq SignPreHashReq;
	u32 PreLen = 0U;
	u32 OutSigLen = 0U;
	u8 *Base = NULL;
	u8 *LocalPrivKey;
	u8 *LocalSig;
	u8 *LocalCtx;
	const u8 *CtxPtr = NULL;
	/*
	 * mu is always SHAKE256, which always lives on XSHA_1, so this engine is a property of the
	 * algorithm rather than of the request and is resolved here rather than threaded in.
	 */
	XSha *Sha3Ptr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);

	/*
	 * The key must be resolved to an address by now: a vault key ID is turned into KeyAddr by
	 * the handler before the request reaches here.
	 */
	if ((DmaPtr == NULL) || (Sha2Ptr == NULL) || (Sha3Ptr == NULL) || (ParamsPtr == NULL) ||
		(ParamsPtr->KeyAddr == 0U)) {
		Status = XASUFW_MLDSA_INVALID_PARAM;
		goto END;
	}

	/** Validate the request parameters the client validates against the same contract. */
	Status = XAsu_MldsaValidateParams(ParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLDSA_INVALID_PARAM;
		goto END;
	}

	CtxLen = ParamsPtr->ContextLen;

	Status = XMldsa_GetLengths(ParamsPtr->ParamSet, &ExpPubKeyLen, &ExpPrivKeyLen, &ExpSignLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLDSA_SIGN_FAILED);
		goto END;
	}

	if ((ParamsPtr->KeyLen < ExpPrivKeyLen) || (ParamsPtr->SignatureLen < ExpSignLen)) {
		Status = XASUFW_MLDSA_INVALID_PARAM;
		goto END;
	}

	Base = (u8 *)(UINTPTR)XMLDSA_SCRATCH_ADDR;
	LocalPrivKey = Base + XMLDSA_SLOT_KEY_OFFSET;
	LocalSig = Base + XMLDSA_SLOT_SIG_OFFSET;
	LocalCtx = Base + XMLDSA_SLOT_CTX_OFFSET;

	/** The context pointer is recomputed on every (re)entry; its buffer persists in scratch. */
	if (CtxLen != 0U) {
		CtxPtr = LocalCtx;
	}

	/**
	 * One-time prelude: marshal the secret key (and context) into scratch and draw the signing
	 * randomness. On the non-blocking resume pass (CmdStage != INIT) this is skipped, so nothing
	 * DMA/engine-touching runs between the suspended pre-hash update and its resumed finish; the
	 * scratch contents and the static Rnd drawn on the initial pass are reused.
	 */
	if (CmdStage == XMLDSA_STAGE_INIT) {
		/**
		 * Capture the start time of the ML-DSA signature generation operation, if performance
		 * measurement is enabled. It is taken here, inside the one-time prelude, so that a
		 * non-blocking resume pass does not restart the measurement; the elapsed time therefore
		 * spans the whole command, including the suspended streaming pre-hash.
		 */
		XASUFW_MEASURE_PERF_START(XASU_MODULE_MLDSA_ID);

		/** DMA the client secret key (and context, if any) into the local scratch region. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, ParamsPtr->KeyAddr, (u64)(UINTPTR)LocalPrivKey,
			ExpPrivKeyLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}

		if (CtxLen != 0U) {
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XAsufw_DmaXfr(DmaPtr, ParamsPtr->ContextAddr, (u64)(UINTPTR)LocalCtx,
				CtxLen, 0U);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_DMA_COPY_FAIL;
				goto END;
			}
		}

		/**
		 * An rnd supplied by the caller is used as it stands, which only an algorithm
		 * validation harness does; otherwise deterministic signing uses an all-zero rnd
		 * and hedged signing uses TRNG randomness.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		if (RndPtr != NULL) {
			if (RndLen != MLDSA_RNDBYTES) {
				Status = XASUFW_MLDSA_INVALID_PARAM;
				goto END;
			}
			Status = Xil_SMemCpy(Rnd, MLDSA_RNDBYTES, RndPtr, RndLen, RndLen);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLDSA_SIGN_FAILED);
				goto END;
			}
		} else if (ParamsPtr->Deterministic == XASU_MLDSA_DETERMINISTIC) {
			/*
			 * Rnd is static and may still hold the hedged randomness of an earlier
			 * signature, so the all-zero rnd is written with a verified zeroization.
			 */
			Status = Xil_SecureZeroize(Rnd, MLDSA_RNDBYTES);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLDSA_SIGN_FAILED);
				goto END;
			}
		} else {
			Status = XTrng_GetRandomNumbers(Rnd, MLDSA_RNDBYTES);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_MLDSA_RANDOM_GEN_FAILED);
				goto END;
			}
		}
	}

	MsgRef.Addr = ParamsPtr->MsgAddr;
	MsgRef.Len = ParamsPtr->MsgLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (ParamsPtr->SignMode == XASU_MLDSA_SIGN_MODE_PURE) {
		PreLen = XMldsa_PrepareDsPrefix(ParamsPtr->ParamSet, Prefix, NULL, CtxPtr, CtxLen);
		if (PreLen == 0U) {
			Status = XASUFW_MLDSA_SIGN_FAILED;
			goto END;
		}

		/**
		 * Compute the FIPS 204 message representative mu = SHAKE256(tr || pre || msg) on the
		 * ASU hardware, streaming the message. tr is embedded in the secret key at offset 64.
		 * A large message streams non-blocking: record the resume stage and return
		 * XASUFW_CMD_IN_PROGRESS so the handler can arm the DMA-done wait. RET is taken to
		 * preserve the secret key/randomness in scratch across the suspend; on re-dispatch the
		 * prelude is skipped and only the final XSha_Finish squeezes mu.
		 */
		MuIn.Tr = LocalPrivKey + XMLDSA_SK_TR_OFFSET;
		MuIn.Pre = Prefix;
		MuIn.PreLen = PreLen;
		MuIn.Msg = MsgRef;
		Status = XMldsa_HwComputeMu(Sha3Ptr, DmaPtr, &MuIn, Mu,
			(u32)(CmdStage == XMLDSA_STAGE_MU_RESUME));
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			CmdStage = XMLDSA_STAGE_MU_RESUME;
			goto RET;
		}
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MLDSA_PREHASH_FAILED;
			goto END;
		}

		/** mu is already the message representative, so the backend runs in external-mu mode. */
		SignReq.Sig = LocalSig;
		SignReq.SigLen = &OutSigLen;
		SignReq.Msg = Mu;
		SignReq.MsgLen = MLDSA_CRHBYTES;
		SignReq.Pre = NULL;
		SignReq.PreLen = 0U;
		SignReq.Rnd = Rnd;
		SignReq.PrivKey = LocalPrivKey;
		SignReq.ExternalMu = (u32)XASU_TRUE;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XMldsa_SignatureInternal(ParamsPtr->ParamSet, &SignReq);
	} else if (ParamsPtr->SignMode == XASU_MLDSA_SIGN_MODE_PREHASH) {
		Status = XMldsa_ComputePreHash(ParamsPtr->ShaType, ParamsPtr->ShaMode, Sha2Ptr,
			Sha3Ptr, DmaPtr, &MsgRef, &PreHashOut);
		/**
		 * A large message pre-hash streams non-blocking: record the resume stage and return
		 * XASUFW_CMD_IN_PROGRESS so the handler can arm the DMA-done wait. On re-dispatch the
		 * prelude is skipped and XSha_Digest finishes the digest. RET is taken to preserve the
		 * secret key/randomness in scratch across the suspend (they must not be zeroized yet).
		 */
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			CmdStage = XMLDSA_STAGE_DIGEST_RESUME;
			goto RET;
		}
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLDSA_SIGN_FAILED);
			goto END;
		}

		SignPreHashReq.Sig = LocalSig;
		SignPreHashReq.SigLen = &OutSigLen;
		SignPreHashReq.PreHash = PreHash;
		SignPreHashReq.PreHashLen = PreHashOut.DigestLen;
		SignPreHashReq.Ctx = CtxPtr;
		SignPreHashReq.CtxLen = CtxLen;
		SignPreHashReq.Rnd = Rnd;
		SignPreHashReq.PrivKey = LocalPrivKey;
		SignPreHashReq.HashAlg = PreHashOut.MldHashAlg;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XMldsa_SignaturePreHashInternal(ParamsPtr->ParamSet, &SignPreHashReq);
	} else {
		Status = XASUFW_MLDSA_UNSUPPORTED_SIGN_MODE;
		goto END;
	}

	if ((Status != XASUFW_SUCCESS) || (OutSigLen != ExpSignLen)) {
		Status = XASUFW_MLDSA_SIGN_FAILED;
		goto END;
	}

	/** DMA the generated signature back to the client output address. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)LocalSig, ParamsPtr->SignatureAddr,
		(u32)OutSigLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
	}

END:
	/**
	 * Reached on completion or error (not on the non-blocking suspend, which takes RET). Reset the
	 * resume stage for the next command and wipe the signing randomness, the message
	 * representative and the secret key from memory / shared scratch. mu is publicly recomputable
	 * from the public key, but it is derived from tr read out of the secret key, so it is cleared
	 * with the rest of the key-adjacent material.
	 */
	CmdStage = XMLDSA_STAGE_INIT;

	/**
	 * Measure and print the performance time for the ML-DSA signature generation operation, if
	 * performance measurement is enabled. This is reached on every terminating path, so the
	 * measurement is always closed even when the command fails.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_MLDSA_ID);

	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, Rnd, MLDSA_RNDBYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, Mu, MLDSA_CRHBYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	if (Base != NULL) {
		XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus,
			Base + XMLDSA_SLOT_KEY_OFFSET, ExpPrivKeyLen);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
		XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus,
			Base + XMLDSA_SLOT_SIG_OFFSET, ExpSignLen);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}

RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function verifies an ML-DSA signature over the provided message, supporting
 *		pure ML-DSA and HashML-DSA (pre-hash) and an optional context string. The public
 *		key, signature and context are DMA'd in and the message is streamed through the ASU
 *		hardware SHA engines.
 *
 * @param	DmaPtr		Pointer to the ASU DMA instance used to marshal client buffers.
 * @param	Sha2Ptr		SHA-2 engine instance (XSHA_0), used for SHA-2 HashML-DSA pre-hash.
 *				The SHA-3 engine is resolved internally; see the note in the body.
 * @param	ParamsPtr	Pointer to the ML-DSA signature verification parameters.
 *
 * @return
 *	- XASUFW_SUCCESS if the signature is valid.
 *	- XASUFW_CMD_IN_PROGRESS if a large message streams in DMA non-blocking mode.
 *	- XASUFW_MLDSA_INVALID_PARAM if inputs are invalid.
 *	- XASUFW_MLDSA_UNSUPPORTED_PARAM_SET if the parameter set is unsupported.
 *	- XASUFW_MLDSA_UNSUPPORTED_SIGN_MODE if the sign mode is unsupported.
 *	- XASUFW_MLDSA_UNSUPPORTED_PREHASH_ALG if the pre-hash algorithm is unsupported.
 *	- XASUFW_MLDSA_PREHASH_FAILED if the message pre-hash / mu computation fails.
 *	- XASUFW_MLDSA_VERIFY_FAILED if verification fails.
 *	- XASUFW_DMA_COPY_FAIL if a public key, signature or context transfer fails.
 *
 *************************************************************************************************/
s32 XMldsa_Verify(XAsufw_Dma *DmaPtr, XSha *Sha2Ptr, const XAsu_MldsaSignVerifyParams *ParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 ExpPubKeyLen = 0U;
	u32 ExpPrivKeyLen = 0U;
	u32 ExpSignLen = 0U;
	u32 CtxLen;
	/*
	 * Function-local static resume stage for the non-blocking HashML-DSA message pre-hash. Safe
	 * because the MLDSA and SHA resources are held for the whole command (only one verify at a
	 * time). Verify has no secret randomness, so only the stage needs to persist.
	 */
	static u32 CmdStage = XMLDSA_STAGE_INIT;
	u8 PreHash[XMLDSA_MAX_PREHASH_LEN];
	u8 Mu[MLDSA_CRHBYTES];
	u8 Hpk[MLDSA_CRHBYTES];
	u8 Prefix[MLD_DOMAIN_SEPARATION_MAX_BYTES];
	XMldsa_PreHashOut PreHashOut = {PreHash, 0U, (s32)MLD_PREHASH_NONE};
	XMldsa_MsgRef MsgRef;
	XMldsa_MsgRef PubKeyRef;
	XMldsa_MuInput MuIn;
	XMldsa_VerifyReq VerifyReq;
	XMldsa_VerifyPreHashReq VerifyPreHashReq;
	u32 PreLen = 0U;
	u8 *Base = NULL;
	u8 *LocalPubKey;
	u8 *LocalSig;
	u8 *LocalCtx;
	const u8 *CtxPtr = NULL;
	/*
	 * mu and H(pk) are always SHAKE256, which always lives on XSHA_1, so this engine is a
	 * property of the algorithm rather than of the request and is resolved here rather than
	 * threaded in. Sha2Ptr stays a parameter because whether SHA-2 is used at all, and in which
	 * mode, depends on the requested HashML-DSA pre-hash. The caller still reserves both
	 * engines; see the SHA2 and SHA3 masks on the verify command in xasufw_mldsahandler.c.
	 */
	XSha *Sha3Ptr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);

	/*
	 * The key must be resolved to an address by now: a vault key ID is turned into KeyAddr by
	 * the handler before the request reaches here.
	 */
	if ((DmaPtr == NULL) || (Sha3Ptr == NULL) || (Sha2Ptr == NULL) || (ParamsPtr == NULL) ||
		(ParamsPtr->KeyAddr == 0U)) {
		Status = XASUFW_MLDSA_INVALID_PARAM;
		goto END;
	}

	/** Validate the request parameters the client validates against the same contract. */
	Status = XAsu_MldsaValidateParams(ParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLDSA_INVALID_PARAM;
		goto END;
	}

	CtxLen = ParamsPtr->ContextLen;

	Status = XMldsa_GetLengths(ParamsPtr->ParamSet, &ExpPubKeyLen, &ExpPrivKeyLen, &ExpSignLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLDSA_VERIFY_FAILED);
		goto END;
	}

	if ((ParamsPtr->KeyLen < ExpPubKeyLen) || (ParamsPtr->SignatureLen != ExpSignLen)) {
		Status = XASUFW_MLDSA_INVALID_PARAM;
		goto END;
	}

	Base = (u8 *)(UINTPTR)XMLDSA_SCRATCH_ADDR;
	LocalPubKey = Base + XMLDSA_SLOT_KEY_OFFSET;
	LocalSig = Base + XMLDSA_SLOT_SIG_OFFSET;
	LocalCtx = Base + XMLDSA_SLOT_CTX_OFFSET;

	/** The context pointer is recomputed on every (re)entry; its buffer persists in scratch. */
	if (CtxLen != 0U) {
		CtxPtr = LocalCtx;
	}

	/**
	 * One-time prelude: marshal the public key, signature (and context) into scratch. On the
	 * non-blocking resume pass (CmdStage != INIT) this is skipped so nothing DMA/engine-touching
	 * runs between the suspended pre-hash update and its resumed finish; the scratch contents from
	 * the initial pass are reused.
	 */
	if (CmdStage == XMLDSA_STAGE_INIT) {
		/**
		 * Capture the start time of the ML-DSA signature verification operation, if performance
		 * measurement is enabled. It is taken here, inside the one-time prelude, so that a
		 * non-blocking resume pass does not restart the measurement; the elapsed time therefore
		 * spans the whole command, including the suspended streaming pre-hash.
		 */
		XASUFW_MEASURE_PERF_START(XASU_MODULE_MLDSA_ID);

		/** DMA the client public key and signature (and context, if any) into local scratch. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, ParamsPtr->KeyAddr, (u64)(UINTPTR)LocalPubKey,
			ExpPubKeyLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, ParamsPtr->SignatureAddr, (u64)(UINTPTR)LocalSig,
			ExpSignLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}

		if (CtxLen != 0U) {
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XAsufw_DmaXfr(DmaPtr, ParamsPtr->ContextAddr, (u64)(UINTPTR)LocalCtx,
				CtxLen, 0U);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_DMA_COPY_FAIL;
				goto END;
			}
		}
	}

	MsgRef.Addr = ParamsPtr->MsgAddr;
	MsgRef.Len = ParamsPtr->MsgLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (ParamsPtr->SignMode == XASU_MLDSA_SIGN_MODE_PURE) {
		PreLen = XMldsa_PrepareDsPrefix(ParamsPtr->ParamSet, Prefix, NULL, CtxPtr, CtxLen);
		if (PreLen == 0U) {
			Status = XASUFW_MLDSA_VERIFY_FAILED;
			goto END;
		}

		/**
		 * For verification, tr is derived fresh from the public key: hpk = SHAKE256(pk). Then
		 * mu = SHAKE256(hpk || pre || msg), streaming the message, both on the ASU hardware.
		 * hpk is a small, synchronous digest and must run only on the initial pass: on a
		 * non-blocking resume the same SHAKE256 engine (XSHA_1) still holds the suspended mu
		 * state, so recomputing hpk here would corrupt it.
		 */
		if (CmdStage == XMLDSA_STAGE_INIT) {
			PubKeyRef.Addr = (u64)(UINTPTR)LocalPubKey;
			PubKeyRef.Len = ExpPubKeyLen;
			Status = XMldsa_HwDigest(Sha3Ptr, XASU_SHA_MODE_SHAKE256, DmaPtr,
				&PubKeyRef, Hpk, MLDSA_CRHBYTES);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_MLDSA_PREHASH_FAILED;
				goto END;
			}
		}

		/**
		 * A large message streams non-blocking while computing mu: record the resume stage and
		 * return XASUFW_CMD_IN_PROGRESS so the handler can arm the DMA-done wait. On re-dispatch
		 * the prelude and hpk are skipped and only the final XSha_Finish squeezes mu.
		 */
		MuIn.Tr = Hpk;
		MuIn.Pre = Prefix;
		MuIn.PreLen = PreLen;
		MuIn.Msg = MsgRef;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XMldsa_HwComputeMu(Sha3Ptr, DmaPtr, &MuIn, Mu,
			(u32)(CmdStage == XMLDSA_STAGE_MU_RESUME));
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			CmdStage = XMLDSA_STAGE_MU_RESUME;
			goto RET;
		}
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MLDSA_PREHASH_FAILED;
			goto END;
		}

		/** mu is already the message representative, so the backend runs in external-mu mode. */
		VerifyReq.Sig = LocalSig;
		VerifyReq.SigLen = ExpSignLen;
		VerifyReq.Msg = Mu;
		VerifyReq.MsgLen = MLDSA_CRHBYTES;
		VerifyReq.Pre = NULL;
		VerifyReq.PreLen = 0U;
		VerifyReq.PubKey = LocalPubKey;
		VerifyReq.ExternalMu = (u32)XASU_TRUE;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XMldsa_VerifyInternal(ParamsPtr->ParamSet, &VerifyReq);
	} else if (ParamsPtr->SignMode == XASU_MLDSA_SIGN_MODE_PREHASH) {
		Status = XMldsa_ComputePreHash(ParamsPtr->ShaType, ParamsPtr->ShaMode, Sha2Ptr,
			Sha3Ptr, DmaPtr, &MsgRef, &PreHashOut);
		/**
		 * A large message pre-hash streams non-blocking: record the resume stage and return
		 * XASUFW_CMD_IN_PROGRESS so the handler can arm the DMA-done wait. On re-dispatch the
		 * prelude is skipped and XSha_Digest finishes the digest.
		 */
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			CmdStage = XMLDSA_STAGE_DIGEST_RESUME;
			goto RET;
		}
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLDSA_VERIFY_FAILED);
			goto END;
		}

		VerifyPreHashReq.Sig = LocalSig;
		VerifyPreHashReq.SigLen = ExpSignLen;
		VerifyPreHashReq.PreHash = PreHash;
		VerifyPreHashReq.PreHashLen = PreHashOut.DigestLen;
		VerifyPreHashReq.Ctx = CtxPtr;
		VerifyPreHashReq.CtxLen = CtxLen;
		VerifyPreHashReq.PubKey = LocalPubKey;
		VerifyPreHashReq.HashAlg = PreHashOut.MldHashAlg;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XMldsa_VerifyPreHashInternal(ParamsPtr->ParamSet, &VerifyPreHashReq);
	} else {
		Status = XASUFW_MLDSA_UNSUPPORTED_SIGN_MODE;
		goto END;
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLDSA_VERIFY_FAILED);
		goto END;
	}

	/**
	 * Redundantly record the verified result in the global ReturnStatus (anti-glitch).It is
	 * surfaced in the response AdditionalStatus field by the command response handler.
	 */
	ReturnStatus = XASUFW_MLDSA_SIGNATURE_VERIFIED;

END:
	/** Reached on completion or error (not on the non-blocking suspend, which takes RET). */
	CmdStage = XMLDSA_STAGE_INIT;

	/**
	 * Measure and print the performance time for the ML-DSA signature verification operation, if
	 * performance measurement is enabled. This is reached on every terminating path, so the
	 * measurement is always closed even when the command fails.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_MLDSA_ID);
	if (Base != NULL) {
		Status = XAsufw_UpdateBufStatus(Status, Xil_SecureZeroize(Base, XMLDSA_SIGN_SCRATCH_LEN));
	}

RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the public key, secret key and signature lengths for the
 *		requested ML-DSA parameter set.
 *
 * @param	ParamSet	ML-DSA parameter set (XAsu_MldsaParamSet).
 * @param	PubKeyLen	Pointer to store the public key length.
 * @param	PrivKeyLen	Pointer to store the secret key length.
 * @param	SignLen		Pointer to store the signature length.
 *
 * @return
 *	- XASUFW_SUCCESS on a supported parameter set.
 *	- XASUFW_MLDSA_UNSUPPORTED_PARAM_SET otherwise.
 *
 *************************************************************************************************/
static s32 XMldsa_GetLengths(u8 ParamSet, u32 *PubKeyLen, u32 *PrivKeyLen, u32 *SignLen)
{
	s32 Status = XASUFW_MLDSA_UNSUPPORTED_PARAM_SET;

	switch (ParamSet) {
		case XASU_MLDSA_44:
			*PubKeyLen = XASU_MLDSA_44_PUB_KEY_LEN;
			*PrivKeyLen = XASU_MLDSA_44_PRIV_KEY_LEN;
			*SignLen = XASU_MLDSA_44_SIGN_LEN;
			Status = XASUFW_SUCCESS;
			break;
		case XASU_MLDSA_65:
			*PubKeyLen = XASU_MLDSA_65_PUB_KEY_LEN;
			*PrivKeyLen = XASU_MLDSA_65_PRIV_KEY_LEN;
			*SignLen = XASU_MLDSA_65_SIGN_LEN;
			Status = XASUFW_SUCCESS;
			break;
		case XASU_MLDSA_87:
			*PubKeyLen = XASU_MLDSA_87_PUB_KEY_LEN;
			*PrivKeyLen = XASU_MLDSA_87_PRIV_KEY_LEN;
			*SignLen = XASU_MLDSA_87_SIGN_LEN;
			Status = XASUFW_SUCCESS;
			break;
		default:
			Status = XASUFW_MLDSA_UNSUPPORTED_PARAM_SET;
			break;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function computes a single-shot digest on an ASU hardware SHA engine, streaming
 *		the input from the given (possibly 64-bit client) address using the shared
 *		XSha_Digest API. It is used for the HashML-DSA message pre-hash (SHA-2 on XSHA_0,
 *		SHA-3/SHAKE256 on XSHA_1) and for the verification hpk = SHAKE256(pk) computation.
 *
 *		For an input larger than XASUFW_DMA_BLOCKING_SIZE, XSha_Digest runs the transfer
 *		non-blocking and returns XASUFW_CMD_IN_PROGRESS; that status is propagated verbatim so
 *		the caller can suspend the command and complete the digest when re-dispatched on DMA
 *		completion. Small inputs (e.g. hpk = SHAKE256(pk)) complete synchronously.
 *
 * @param	ShaPtr		SHA engine instance to use (XSHA_0 for SHA-2, XSHA_1 for
 *				SHA-3/SHAKE256), resolved and reserved by the caller.
 * @param	ShaMode		ASU SHA mode (XASU_SHA_MODE_256/_384/_512/_SHAKE256).
 * @param	DmaPtr		Pointer to the ASU DMA instance.
 * @param	Data		Input data location: a valid, non-zero address whose length may be
 *				0 for an empty message.
 * @param	Hash		Pointer to the output digest buffer.
 * @param	HashLen		Digest length in bytes.
 *
 * @return
 *	- XASUFW_SUCCESS on success.
 *	- XASUFW_CMD_IN_PROGRESS if the SHA transfer is running non-blocking.
 *	- XASUFW_MLDSA_PREHASH_FAILED, folded with the SHA driver error, on digest failure.
 *
 *************************************************************************************************/
static s32 XMldsa_HwDigest(XSha *ShaPtr, u32 ShaMode, XAsufw_Dma *DmaPtr,
	const XMldsa_MsgRef *Data, u8 *Hash, u32 HashLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XAsu_ShaOperationCmd ShaParams;

	/**
	 * Drive the hash through the shared XSha_Digest (Start -> Update -> Finish) API. A
	 * zero-length input with a valid address is handled by XSha_Update as a no-op and padded by
	 * XSha_Finish. The status (including XASUFW_CMD_IN_PROGRESS) is returned to the caller as is.
	 */
	Status = Xil_SMemSet(&ShaParams, sizeof(ShaParams), 0U, sizeof(ShaParams));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLDSA_PREHASH_FAILED);
		goto END;
	}

	ShaParams.DataAddr = Data->Addr;
	ShaParams.DataSize = Data->Len;
	ShaParams.HashAddr = (u64)(UINTPTR)Hash;
	ShaParams.HashBufSize = HashLen;
	ShaParams.ShaMode = (u8)ShaMode;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Digest(ShaPtr, DmaPtr, &ShaParams);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function computes the FIPS 204 message representative mu on the ASU hardware
 *		SHAKE256 engine (XSHA_1) as mu = SHAKE256(tr || pre || msg), streaming the message
 *		from its (possibly 64-bit client) address. Computing mu here keeps the message out
 *		of ASU memory and lets the backend be driven through its external-mu entry point.
 *
 *		For a message larger than XASUFW_DMA_BLOCKING_SIZE the final XSha_Update runs the DMA
 *		non-blocking and returns XASUFW_CMD_IN_PROGRESS, which is propagated so the caller can
 *		suspend the command. The SHAKE256 engine state persists across the suspend, so on the
 *		resume pass (Resume == TRUE) the tr/prefix/message have already been absorbed and only
 *		the final XSha_Finish (XOF squeeze) is performed.
 *
 * @param	Sha3Ptr	SHA-3/SHAKE256 engine instance (XSHA_1), resolved by the public entry point
 *			and reserved by the command handler.
 * @param	DmaPtr	Pointer to the ASU DMA instance.
 * @param	MuIn	Inputs absorbed to form mu: tr, the domain-separation prefix and its length,
 *			and the streamed message. Ignored on the resume pass.
 * @param	Mu	Pointer to the output mu buffer (MLDSA_CRHBYTES bytes).
 * @param	Resume	XASU_TRUE to resume a suspended computation (finish only); XASU_FALSE for the
 *			initial pass (absorb tr || pre || msg then finish, unless the message stream
 *			suspends).
 *
 * @return
 *	- XASUFW_SUCCESS on success.
 *	- XASUFW_CMD_IN_PROGRESS if the message stream is running non-blocking (large message).
 *	- XASUFW_MLDSA_PREHASH_FAILED, folded with the SHA driver error, on mu computation failure.
 *
 *************************************************************************************************/
static s32 XMldsa_HwComputeMu(XSha *Sha3Ptr, XAsufw_Dma *DmaPtr, const XMldsa_MuInput *MuIn,
	u8 *Mu, u32 Resume)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/**
	 * On the resume pass tr || pre || msg have already been absorbed (the SHAKE256 engine state
	 * persisted across the non-blocking suspend), so skip straight to the final XOF squeeze.
	 */
	if (Resume == (u32)XASU_FALSE) {
		Status = XSha_Start(Sha3Ptr, XASU_SHA_MODE_SHAKE256);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLDSA_PREHASH_FAILED);
			goto END;
		}

		/** Absorb tr (never the last block, tr is always followed by pre). */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Update(Sha3Ptr, DmaPtr, (u64)(UINTPTR)MuIn->Tr, XMLDSA_TR_LEN,
			(u32)XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLDSA_PREHASH_FAILED);
			goto END;
		}

		/**
		 * Absorb the prefix followed by the streamed message. When there is no message, the
		 * prefix is the final block so the SHAKE256 hardware performs the correct XOF padding
		 * and the digest completes synchronously.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		if (MuIn->Msg.Len == 0U) {
			Status = XSha_Update(Sha3Ptr, DmaPtr, (u64)(UINTPTR)MuIn->Pre, MuIn->PreLen,
				(u32)XASU_TRUE);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLDSA_PREHASH_FAILED);
				goto END;
			}
		} else {
			Status = XSha_Update(Sha3Ptr, DmaPtr, (u64)(UINTPTR)MuIn->Pre, MuIn->PreLen,
				(u32)XASU_FALSE);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLDSA_PREHASH_FAILED);
				goto END;
			}

			/**
			 * Stream the message as the final block. For a message larger than
			 * XASUFW_DMA_BLOCKING_SIZE, XSha_Update runs the DMA non-blocking and returns
			 * XASUFW_CMD_IN_PROGRESS; propagate it (skipping XSha_Finish) so the caller can
			 * suspend and resume at the finish once the DMA completes.
			 */
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XSha_Update(Sha3Ptr, DmaPtr, MuIn->Msg.Addr, MuIn->Msg.Len,
				(u32)XASU_TRUE);
			if (Status == XASUFW_CMD_IN_PROGRESS) {
				goto END;
			}
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLDSA_PREHASH_FAILED);
				goto END;
			}
		}
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Finish(Sha3Ptr, DmaPtr, (u64)(UINTPTR)Mu, MLDSA_CRHBYTES, (u8)XASU_FALSE);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function computes the FIPS 204 pre-hash digest of a message for HashML-DSA on
 *		the ASU hardware SHA engines (streaming the message) and maps the requested SHA type
 *		and mode to the mldsa-native hash constant. Only 7 of the 12 FIPS 204 pre-hashes
 *		are reachable; SHA2-224, SHA2-512/224, SHA2-512/256, SHA3-224 and SHAKE128 have no
 *		SHA mode encoding and cannot be requested.
 *
 * @param	ShaType		Pre-hash family type (XASU_SHA2_TYPE / XASU_SHA3_TYPE).
 * @param	ShaMode		Pre-hash SHA mode (XASU_SHA_MODE_256/_384/_512/_SHAKE256).
 * @param	Sha2Ptr		SHA-2 engine instance (XSHA_0), used for the SHA2 pre-hashes.
 * @param	Sha3Ptr		SHA-3/SHAKE256 engine instance (XSHA_1), used for the SHA3 and
 *				SHAKE256 pre-hashes.
 * @param	DmaPtr		Pointer to the ASU DMA instance.
 * @param	Msg		Message location (the address may be a 64-bit client address).
 * @param	Out		Pre-hash result: on entry Out->Digest points to the output buffer
 *				(>= XMLDSA_MAX_PREHASH_LEN); on success Out->DigestLen and
 *				Out->MldHashAlg are set.
 *
 * @return
 *	- XASUFW_SUCCESS on success.
 *	- XASUFW_CMD_IN_PROGRESS if the message stream is running non-blocking (large message).
 *	- XASUFW_MLDSA_UNSUPPORTED_PREHASH_ALG if the algorithm is unsupported.
 *	- XASUFW_MLDSA_PREHASH_FAILED if the hardware hash fails.
 *
 *************************************************************************************************/
static s32 XMldsa_ComputePreHash(u8 ShaType, u8 ShaMode, XSha *Sha2Ptr, XSha *Sha3Ptr,
	XAsufw_Dma *DmaPtr, const XMldsa_MsgRef *Msg, XMldsa_PreHashOut *Out)
{
	CREATE_VOLATILE(Status, XASUFW_MLDSA_UNSUPPORTED_PREHASH_ALG);
	XSha *ShaPtr = NULL;
	u32 DigestLen = 0U;
	s32 MldHashAlg = (s32)MLD_PREHASH_NONE;
	u32 IsSha3 = (ShaType == XASU_SHA3_TYPE) ? 1U : 0U;

	/**
	 * The (type, mode) pairs accepted here cover 7 of the 12 pre-hash functions FIPS 204
	 * approves: SHA2-256/384/512 and SHA3-256/384/512 plus SHAKE256. SHA2-224, SHA2-512/224,
	 * SHA2-512/256, SHA3-224 and SHAKE128 have no SHA mode encoding and so cannot be
	 * requested at all.
	 */
	if (XAsu_ShaValidateModeAndType(ShaType, ShaMode) != XST_SUCCESS) {
		Status = XASUFW_MLDSA_UNSUPPORTED_PREHASH_ALG;
		goto END;
	}

	ShaPtr = (IsSha3 != 0U) ? Sha3Ptr : Sha2Ptr;
	if (ShaPtr == NULL) {
		Status = XASUFW_MLDSA_UNSUPPORTED_PREHASH_ALG;
		goto END;
	}

	/*
	 * The mode selects the digest length and, together with the type, the hash function's OID,
	 * which FIPS 204 needs and the SHA driver has no notion of. The digest length is stated per
	 * case rather than derived, because SHAKE256 outputs 64 bytes where mode-256 outputs 32.
	 */
	switch (ShaMode) {
		case XASU_SHA_MODE_256:
			DigestLen = XASU_SHA_SHAKE_256_HASH_LEN;
			MldHashAlg = (IsSha3 != 0U) ? (s32)MLD_PREHASH_SHA3_256 :
				(s32)MLD_PREHASH_SHA2_256;
			break;
		case XASU_SHA_MODE_384:
			DigestLen = XASU_SHA_384_HASH_LEN;
			MldHashAlg = (IsSha3 != 0U) ? (s32)MLD_PREHASH_SHA3_384 :
				(s32)MLD_PREHASH_SHA2_384;
			break;
		case XASU_SHA_MODE_512:
			DigestLen = XASU_SHA_512_HASH_LEN;
			MldHashAlg = (IsSha3 != 0U) ? (s32)MLD_PREHASH_SHA3_512 :
				(s32)MLD_PREHASH_SHA2_512;
			break;
		default:
			/* Validation above leaves only SHAKE256, which is SHA-3 only. */
			DigestLen = XMLDSA_PREHASH_SHAKE256_LEN;
			MldHashAlg = (s32)MLD_PREHASH_SHAKE_256;
			break;
	}

	Out->DigestLen = DigestLen;
	Out->MldHashAlg = MldHashAlg;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XMldsa_HwDigest(ShaPtr, ShaMode, DmaPtr, Msg, Out->Digest, DigestLen);

	/**
	 * Map any hardware hash failure to the pre-hash failure status. XASUFW_CMD_IN_PROGRESS (the
	 * message stream running non-blocking) is passed through so the caller can suspend.
	 */
	if ((Status != XASUFW_SUCCESS) && (Status != XASUFW_CMD_IN_PROGRESS)) {
		Status = XASUFW_MLDSA_PREHASH_FAILED;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Dispatches ML-DSA KeyGen_internal (seed-based) to the correct parameter set.
 *
 * @param	ParamSet	ML-DSA parameter set (XASU_MLDSA_44/_65/_87).
 * @param	PubKey		Pointer to the output public key buffer.
 * @param	PrivKey		Pointer to the output secret key buffer.
 * @param	Seed		Pointer to the MLDSA_SEEDBYTES xi seed.
 *
 * @return
 *	- XASUFW_SUCCESS if the key pair is generated.
 *	- XASUFW_MLDSA_KEYGEN_FAILED if the parameter set is unsupported or the backend fails.
 *
 *************************************************************************************************/
static s32 XMldsa_KeypairInternal(u8 ParamSet, u8 *PubKey, u8 *PrivKey, const u8 *Seed)
{
	s32 Status = XASUFW_MLDSA_KEYGEN_FAILED;
	int OqsStatus = XMLDSA_BACKEND_FAILURE;

	switch (ParamSet) {
		case XASU_MLDSA_44:
			OqsStatus = PQCP_MLDSA_NATIVE_MLDSA44_C_keypair_internal(PubKey, PrivKey,
				Seed);
			break;
		case XASU_MLDSA_65:
			OqsStatus = PQCP_MLDSA_NATIVE_MLDSA65_C_keypair_internal(PubKey, PrivKey,
				Seed);
			break;
		case XASU_MLDSA_87:
			OqsStatus = PQCP_MLDSA_NATIVE_MLDSA87_C_keypair_internal(PubKey, PrivKey,
				Seed);
			break;
		default:
			OqsStatus = XMLDSA_BACKEND_FAILURE;
			break;
	}

	if (OqsStatus == XMLDSA_BACKEND_SUCCESS) {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Dispatches ML-DSA signature_internal (pure/external-mu) to the correct parameter set.
 *
 * @param	ParamSet	ML-DSA parameter set (XASU_MLDSA_44/_65/_87).
 * @param	Req		Pointer to the signature request descriptor.
 *
 * @return
 *	- XASUFW_SUCCESS if the signature is generated and Req->SigLen is updated.
 *	- XASUFW_MLDSA_SIGN_FAILED if the parameter set is unsupported or the backend fails.
 *
 *************************************************************************************************/
static s32 XMldsa_SignatureInternal(u8 ParamSet, const XMldsa_SignReq *Req)
{
	s32 Status = XASUFW_MLDSA_SIGN_FAILED;
	int OqsStatus = XMLDSA_BACKEND_FAILURE;
	size_t OutSigLen = 0U;
	size_t MsgLen = (size_t)Req->MsgLen;
	size_t PreLen = (size_t)Req->PreLen;
	int ExternalMu = (int)Req->ExternalMu;

	switch (ParamSet) {
		case XASU_MLDSA_44:
			OqsStatus = PQCP_MLDSA_NATIVE_MLDSA44_C_signature_internal(Req->Sig,
				&OutSigLen, Req->Msg, MsgLen, Req->Pre, PreLen, Req->Rnd,
				Req->PrivKey, ExternalMu);
			break;
		case XASU_MLDSA_65:
			OqsStatus = PQCP_MLDSA_NATIVE_MLDSA65_C_signature_internal(Req->Sig,
				&OutSigLen, Req->Msg, MsgLen, Req->Pre, PreLen, Req->Rnd,
				Req->PrivKey, ExternalMu);
			break;
		case XASU_MLDSA_87:
			OqsStatus = PQCP_MLDSA_NATIVE_MLDSA87_C_signature_internal(Req->Sig,
				&OutSigLen, Req->Msg, MsgLen, Req->Pre, PreLen, Req->Rnd,
				Req->PrivKey, ExternalMu);
			break;
		default:
			OqsStatus = XMLDSA_BACKEND_FAILURE;
			break;
	}

	if (OqsStatus == XMLDSA_BACKEND_SUCCESS) {
		*Req->SigLen = (u32)OutSigLen;
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Dispatches HashML-DSA signature_pre_hash_internal to the correct parameter set.
 *
 * @param	ParamSet	ML-DSA parameter set (XASU_MLDSA_44/_65/_87).
 * @param	Req		Pointer to the pre-hash signature request descriptor.
 *
 * @return
 *	- XASUFW_SUCCESS if the signature is generated and Req->SigLen is updated.
 *	- XASUFW_MLDSA_SIGN_FAILED if the parameter set is unsupported or the backend fails.
 *
 *************************************************************************************************/
static s32 XMldsa_SignaturePreHashInternal(u8 ParamSet, const XMldsa_SignPreHashReq *Req)
{
	s32 Status = XASUFW_MLDSA_SIGN_FAILED;
	int OqsStatus = XMLDSA_BACKEND_FAILURE;
	size_t OutSigLen = 0U;
	size_t PreHashLen = (size_t)Req->PreHashLen;
	size_t CtxLen = (size_t)Req->CtxLen;
	int HashAlg = (int)Req->HashAlg;

	switch (ParamSet) {
		case XASU_MLDSA_44:
			OqsStatus = PQCP_MLDSA_NATIVE_MLDSA44_C_signature_pre_hash_internal(Req->Sig,
				&OutSigLen, Req->PreHash, PreHashLen, Req->Ctx, CtxLen, Req->Rnd,
				Req->PrivKey, HashAlg);
			break;
		case XASU_MLDSA_65:
			OqsStatus = PQCP_MLDSA_NATIVE_MLDSA65_C_signature_pre_hash_internal(Req->Sig,
				&OutSigLen, Req->PreHash, PreHashLen, Req->Ctx, CtxLen, Req->Rnd,
				Req->PrivKey, HashAlg);
			break;
		case XASU_MLDSA_87:
			OqsStatus = PQCP_MLDSA_NATIVE_MLDSA87_C_signature_pre_hash_internal(Req->Sig,
				&OutSigLen, Req->PreHash, PreHashLen, Req->Ctx, CtxLen, Req->Rnd,
				Req->PrivKey, HashAlg);
			break;
		default:
			OqsStatus = XMLDSA_BACKEND_FAILURE;
			break;
	}

	if (OqsStatus == XMLDSA_BACKEND_SUCCESS) {
		*Req->SigLen = (u32)OutSigLen;
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Dispatches ML-DSA verify_internal (pure/external-mu) to the correct parameter set.
 *
 * @param	ParamSet	ML-DSA parameter set (XASU_MLDSA_44/_65/_87).
 * @param	Req		Pointer to the verification request descriptor.
 *
 * @return
 *	- XASUFW_SUCCESS if the signature is valid.
 *	- XASUFW_MLDSA_VERIFY_FAILED if the signature is invalid, the parameter set is
 *	  unsupported, or the backend fails.
 *
 *************************************************************************************************/
static s32 XMldsa_VerifyInternal(u8 ParamSet, const XMldsa_VerifyReq *Req)
{
	s32 Status = XASUFW_MLDSA_VERIFY_FAILED;
	int OqsStatus = XMLDSA_BACKEND_FAILURE;
	size_t SigLen = (size_t)Req->SigLen;
	size_t MsgLen = (size_t)Req->MsgLen;
	size_t PreLen = (size_t)Req->PreLen;
	int ExternalMu = (int)Req->ExternalMu;

	switch (ParamSet) {
		case XASU_MLDSA_44:
			OqsStatus = PQCP_MLDSA_NATIVE_MLDSA44_C_verify_internal(Req->Sig, SigLen,
				Req->Msg, MsgLen, Req->Pre, PreLen, Req->PubKey, ExternalMu);
			break;
		case XASU_MLDSA_65:
			OqsStatus = PQCP_MLDSA_NATIVE_MLDSA65_C_verify_internal(Req->Sig, SigLen,
				Req->Msg, MsgLen, Req->Pre, PreLen, Req->PubKey, ExternalMu);
			break;
		case XASU_MLDSA_87:
			OqsStatus = PQCP_MLDSA_NATIVE_MLDSA87_C_verify_internal(Req->Sig, SigLen,
				Req->Msg, MsgLen, Req->Pre, PreLen, Req->PubKey, ExternalMu);
			break;
		default:
			OqsStatus = XMLDSA_BACKEND_FAILURE;
			break;
	}

	if (OqsStatus == XMLDSA_BACKEND_SUCCESS) {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Dispatches HashML-DSA verify_pre_hash_internal to the correct parameter set.
 *
 * @param	ParamSet	ML-DSA parameter set (XASU_MLDSA_44/_65/_87).
 * @param	Req		Pointer to the pre-hash verification request descriptor.
 *
 * @return
 *	- XASUFW_SUCCESS if the signature is valid.
 *	- XASUFW_MLDSA_VERIFY_FAILED if the signature is invalid, the parameter set is
 *	  unsupported, or the backend fails.
 *
 *************************************************************************************************/
static s32 XMldsa_VerifyPreHashInternal(u8 ParamSet, const XMldsa_VerifyPreHashReq *Req)
{
	s32 Status = XASUFW_MLDSA_VERIFY_FAILED;
	int OqsStatus = XMLDSA_BACKEND_FAILURE;
	size_t SigLen = (size_t)Req->SigLen;
	size_t PreHashLen = (size_t)Req->PreHashLen;
	size_t CtxLen = (size_t)Req->CtxLen;
	int HashAlg = (int)Req->HashAlg;

	switch (ParamSet) {
		case XASU_MLDSA_44:
			OqsStatus = PQCP_MLDSA_NATIVE_MLDSA44_C_verify_pre_hash_internal(Req->Sig,
				SigLen, Req->PreHash, PreHashLen, Req->Ctx, CtxLen, Req->PubKey,
				HashAlg);
			break;
		case XASU_MLDSA_65:
			OqsStatus = PQCP_MLDSA_NATIVE_MLDSA65_C_verify_pre_hash_internal(Req->Sig,
				SigLen, Req->PreHash, PreHashLen, Req->Ctx, CtxLen, Req->PubKey,
				HashAlg);
			break;
		case XASU_MLDSA_87:
			OqsStatus = PQCP_MLDSA_NATIVE_MLDSA87_C_verify_pre_hash_internal(Req->Sig,
				SigLen, Req->PreHash, PreHashLen, Req->Ctx, CtxLen, Req->PubKey,
				HashAlg);
			break;
		default:
			OqsStatus = XMLDSA_BACKEND_FAILURE;
			break;
	}

	if (OqsStatus == XMLDSA_BACKEND_SUCCESS) {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Dispatches the FIPS 204 domain-separation prefix builder to the correct set.
 *
 * @param	ParamSet	ML-DSA parameter set (XASU_MLDSA_44/_65/_87).
 * @param	Prefix		Pointer to the output prefix buffer
 *				(MLD_DOMAIN_SEPARATION_MAX_BYTES).
 * @param	PreHash		HashML-DSA pre-hash digest and its algorithm, or NULL for pure
 *				(non-pre-hash) mode.
 * @param	Ctx		Pointer to the context string (NULL when CtxLen is 0).
 * @param	CtxLen		Length of the context string in bytes.
 *
 * @return
 *	- Length of the produced prefix in bytes, or 0 if the parameter set is unsupported.
 *
 *************************************************************************************************/
static u32 XMldsa_PrepareDsPrefix(u8 ParamSet, u8 *Prefix, const XMldsa_PreHashOut *PreHash,
	const u8 *Ctx, u32 CtxLen)
{
	size_t PreLen;
	/*
	 * The mldsa-native builder is a third-party API taking size_t/int, so the ASUFW types are
	 * converted here at the boundary and the result is converted back; the prefix never exceeds
	 * MLD_DOMAIN_SEPARATION_MAX_BYTES, so the u32 narrowing cannot truncate.
	 */
	const u8 *Digest = (PreHash != NULL) ? PreHash->Digest : NULL;
	size_t DigestLen = (PreHash != NULL) ? (size_t)PreHash->DigestLen : 0U;
	int HashAlg = (PreHash != NULL) ? (int)PreHash->MldHashAlg : MLD_PREHASH_NONE;

	switch (ParamSet) {
		case XASU_MLDSA_44:
			PreLen = PQCP_MLDSA_NATIVE_MLDSA44_C_prepare_domain_separation_prefix(Prefix,
				Digest, DigestLen, Ctx, (size_t)CtxLen, HashAlg);
			break;
		case XASU_MLDSA_65:
			PreLen = PQCP_MLDSA_NATIVE_MLDSA65_C_prepare_domain_separation_prefix(Prefix,
				Digest, DigestLen, Ctx, (size_t)CtxLen, HashAlg);
			break;
		case XASU_MLDSA_87:
			PreLen = PQCP_MLDSA_NATIVE_MLDSA87_C_prepare_domain_separation_prefix(Prefix,
				Digest, DigestLen, Ctx, (size_t)CtxLen, HashAlg);
			break;
		default:
			PreLen = 0U;
			break;
	}

	return (u32)PreLen;
}

/*************************************************************************************************/
/**
 * @brief	This function runs the FIPS 140-3 IG 10.3.A pairwise consistency test on a freshly
 *		generated ML-DSA key pair: it signs a fixed empty message with the new secret key
 *		and verifies that signature with the new public key. Unlike the KAT, which
 *		validates the algorithm against a fixed vector, this validates the specific pair
 *		about to be handed to the client.
 *
 *		The message representative mu is computed on the hardware SHAKE256 engine, the same
 *		path production signing uses, so the test also exercises that engine. Because the
 *		message is empty, mu is absorbed in a single pass and the computation cannot
 *		suspend, which matters here: key generation has no resume state machine.
 *
 *		Each side derives its own mu from its own half of the pair, exactly as production
 *		signing and verification do: the signing mu from the tr embedded in the secret key
 *		and the verifying mu from hpk = SHAKE256(pk). This is what lets the test fault tr
 *		itself, which one shared mu would hide.
 *
 * @param	ParamSet	ML-DSA parameter set of the generated pair.
 * @param	Sha3Ptr		Pointer to the SHA-3 instance used to compute mu and hpk.
 * @param	DmaPtr		Pointer to the ASU DMA instance.
 * @param	PubKey		Freshly generated public key.
 * @param	ExpPubKeyLen	Expected public key length for this parameter set, hashed to
 *				obtain hpk for the verifying mu.
 * @param	PrivKey		Freshly generated secret key.
 * @param	Sig		Scratch for the test signature (ExpSignLen bytes).
 * @param	ExpSignLen	Expected signature length for this parameter set.
 *
 * @return
 *	- XASUFW_SUCCESS if the signature verifies under the generated public key.
 *	- XASUFW_MLDSA_PWCT_SIGN_GEN_FAIL if mu or the test signature cannot be produced.
 *	- XASUFW_MLDSA_PWCT_SIGN_VER_FAIL if hpk or the verifying mu cannot be produced, or if
 *	  the test signature does not verify.
 *
 *************************************************************************************************/
static s32 XMldsa_Pwct(u8 ParamSet, XSha *Sha3Ptr, XAsufw_Dma *DmaPtr, const u8 *PubKey,
	u32 ExpPubKeyLen, const u8 *PrivKey, u8 *Sig, u32 ExpSignLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 Mu[MLDSA_CRHBYTES];
	u8 Hpk[MLDSA_CRHBYTES];
	u8 Prefix[MLD_DOMAIN_SEPARATION_MAX_BYTES];
	u8 Rnd[MLDSA_RNDBYTES];
	XMldsa_MuInput MuIn;
	XMldsa_MsgRef PubKeyRef;
	XMldsa_SignReq SignReq;
	XMldsa_VerifyReq VerifyReq;
	u32 PreLen;
	u32 OutSigLen = 0U;

	/*
	 * Sign deterministically with an all-zero rnd. The test needs no unpredictability, and
	 * this keeps key generation from drawing TRNG entropy it would otherwise have to reserve.
	 */
	Status = Xil_SecureZeroize(Rnd, MLDSA_RNDBYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLDSA_PWCT_SIGN_GEN_FAIL;
		goto END;
	}

	/** Pure ML-DSA with an empty context, so the prefix is the two-byte domain separator. */
	PreLen = XMldsa_PrepareDsPrefix(ParamSet, Prefix, NULL, NULL, 0U);
	if (PreLen == 0U) {
		Status = XASUFW_MLDSA_PWCT_SIGN_GEN_FAIL;
		goto END;
	}

	/** Signing mu = SHAKE256(tr || pre), with tr taken from the secret key just generated. */
	MuIn.Tr = PrivKey + XMLDSA_SK_TR_OFFSET;
	MuIn.Pre = Prefix;
	MuIn.PreLen = PreLen;
	MuIn.Msg.Addr = 0U;
	MuIn.Msg.Len = 0U;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XMldsa_HwComputeMu(Sha3Ptr, DmaPtr, &MuIn, Mu, (u32)XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLDSA_PWCT_SIGN_GEN_FAIL;
		goto END;
	}

	SignReq.Sig = Sig;
	SignReq.SigLen = &OutSigLen;
	SignReq.Msg = Mu;
	SignReq.MsgLen = MLDSA_CRHBYTES;
	SignReq.Pre = NULL;
	SignReq.PreLen = 0U;
	SignReq.Rnd = Rnd;
	SignReq.PrivKey = PrivKey;
	SignReq.ExternalMu = (u32)XASU_TRUE;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XMldsa_SignatureInternal(ParamSet, &SignReq);
	if ((Status != XASUFW_SUCCESS) || (OutSigLen != ExpSignLen)) {
		Status = XASUFW_MLDSA_PWCT_SIGN_GEN_FAIL;
		goto END;
	}

	/*
	 * Recompute mu the way production verification does, from hpk = SHAKE256(pk), instead of
	 * reusing the mu the signature was made over. Sharing one mu would leave tr the single
	 * field of the pair this test cannot fault: tr lives inside the secret key and is the only
	 * part of it that commits to the public key, so a corrupt tr would cancel out between the
	 * two sides here and still pass, while every signature the stored key later produced would
	 * be rejected by any verifier that derives tr from the public key.
	 *
	 * Mu is safe to overwrite: the signature above has already been produced from it.
	 */
	PubKeyRef.Addr = (u64)(UINTPTR)PubKey;
	PubKeyRef.Len = ExpPubKeyLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XMldsa_HwDigest(Sha3Ptr, XASU_SHA_MODE_SHAKE256, DmaPtr, &PubKeyRef, Hpk,
		MLDSA_CRHBYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLDSA_PWCT_SIGN_VER_FAIL;
		goto END;
	}

	/*
	 * A public key is far smaller than XASUFW_DMA_BLOCKING_SIZE, so neither the digest above
	 * nor the mu below can suspend. XASUFW_CMD_IN_PROGRESS is therefore treated as a failure:
	 * key generation has no resume state machine that could complete it.
	 */
	MuIn.Tr = Hpk;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XMldsa_HwComputeMu(Sha3Ptr, DmaPtr, &MuIn, Mu, (u32)XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLDSA_PWCT_SIGN_VER_FAIL;
		goto END;
	}

	/*
	 * Verifying against the public key is what makes this a pairwise test: the signature was
	 * made with the secret key, while mu, w1 and t1 here all come from the public key, so a
	 * public key that does not belong to that secret key fails.
	 */
	VerifyReq.Sig = Sig;
	VerifyReq.SigLen = OutSigLen;
	VerifyReq.Msg = Mu;
	VerifyReq.MsgLen = MLDSA_CRHBYTES;
	VerifyReq.Pre = NULL;
	VerifyReq.PreLen = 0U;
	VerifyReq.PubKey = PubKey;
	VerifyReq.ExternalMu = (u32)XASU_TRUE;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XMldsa_VerifyInternal(ParamSet, &VerifyReq);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLDSA_PWCT_SIGN_VER_FAIL;
	}

END:
	/*
	 * The signing mu is derived from tr inside the secret key, so it must not outlive the
	 * test. hpk and the verifying mu that replaces it come from the public key alone and need
	 * no clearing, but the buffer is shared with the signing mu, so it is cleared regardless
	 * of which of the two it currently holds.
	 */
	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, Mu, MLDSA_CRHBYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

#endif /* XASU_MLDSA_ENABLE */
/** @} */
