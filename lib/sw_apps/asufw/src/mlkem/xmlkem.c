/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xmlkem.c
 *
 * This file contains the implementation of the ML-KEM (FIPS 203) core wrapper for ASUFW over the
 * liboqs "mlkem-native" backend. It supports key generation, encapsulation and decapsulation for
 * all NIST parameter sets (ML-KEM-512/768/1024).
 *
 * The native entry points are called directly rather than through the generic OQS_KEM API,
 * because the latter reaches them via a descriptor object that OQS_KEM_new() looks up by
 * algorithm name and allocates on the heap. Calling the backend directly keeps the ML-KEM path
 * free of the allocator, and drops the name-keyed registry that would otherwise anchor every
 * algorithm liboqs was built with against --gc-sections.
 *
 * The wrapper accepts the client request parameter structure directly and a DMA instance. Since
 * the ASU is a 32-bit core and clients pass 64-bit DDR addresses, the fixed-size buffers are
 * marshalled through DMA into the ASU-addressable PQC scratch region (see xasufw_memory.h) before
 * the crypto runs. Encapsulation and decapsulation DMA their results back out; key generation
 * leaves the pair in the scratch region for the caller to read at the XMLKEM_KEYGEN_* offsets.
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
 * @addtogroup xmlkem_server_apis ML-KEM Server APIs
 * @{
 */
/*************************************** Include Files *******************************************/
#include "xmlkem.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_dma.h"
#include "xasufw_memory.h"
#include "xtrng.h"
#include "xil_util.h"
#include "xfih.h"
#include "xasu_mlkem_common.h"
#include "xasufw_perf.h"

#ifdef XASU_MLKEM_ENABLE
/* Ships with the liboqs drop, which the BSP only provides when ML-KEM is enabled. */
#include "oqs_mlkem_internal.h"

/************************************ Constant Definitions ***************************************/
#define XMLKEM_BACKEND_SUCCESS		(0)  /**< mlkem-native success return value */
#define XMLKEM_BACKEND_FAILURE		(-1) /**< Local failure value for an unsupported set */

/*
 * Encapsulation and decapsulation carve the scratch region at runtime from the active parameter
 * set's lengths, so bind each declared worst case here. Adding a buffer to a carve then fails the
 * build rather than silently running off the end of the region. Bounding a carve by
 * XMLKEM_SCRATCH_LEN would catch nothing, as that is the largest of the footprints and so grows
 * with whichever carve is being checked. The key generation carve is in xmlkem.h, since the caller
 * reads the pair out of it.
 *
 * XMLKEM_SCRATCH_LEN spans only decapsulation and key generation, so encapsulation is bound to the
 * decapsulation footprint rather than to the pool. FIPS 203 builds the decapsulation key around the
 * encapsulation key and both carves end in the same ciphertext and shared secret, so encapsulation
 * is the smaller of the two at every parameter set, and this check keeps it that way. Bounding it
 * by the pool instead would let it grow past XMLKEM_SCRATCH_END into the buffer the known-answer
 * test takes from there.
 */
_Static_assert(XMLKEM_KEYGEN_SCRATCH_LEN <= XASUFW_CRYPTO_SHARED_POOL_SIZE,
	       "ML-KEM key generation carve overruns the shared crypto pool reserved in lscript.ld");
_Static_assert(XMLKEM_DECAPS_SCRATCH_LEN <= XASUFW_CRYPTO_SHARED_POOL_SIZE,
	       "ML-KEM decapsulation carve overruns the shared crypto pool reserved in lscript.ld");
_Static_assert(XMLKEM_ENCAPS_SCRATCH_LEN <= XMLKEM_DECAPS_SCRATCH_LEN,
	       "ML-KEM encapsulation carve overruns the decapsulation footprint sizing the region");
_Static_assert((XMLKEM_SCRATCH_END - XASUFW_CRYPTO_ALGO_DATA_BLOCK) <=
	       XASUFW_CRYPTO_SHARED_POOL_SIZE,
	       "ML-KEM scratch region overruns the shared crypto pool reserved in lscript.ld");

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XMlkem_GetLengths(u8 ParamSet, u32 *PubKeyLen, u32 *PrivKeyLen, u32 *CtLen);
static s32 XMlkem_KeypairDerandInternal(u8 ParamSet, u8 *PubKey, u8 *PrivKey, const u8 *Seed);
static s32 XMlkem_EncapsDerandInternal(u8 ParamSet, u8 *Ct, u8 *SharedSecret, const u8 *PubKey,
	const u8 *Seed);
static s32 XMlkem_DecapsInternal(u8 ParamSet, u8 *SharedSecret, const u8 *Ct, const u8 *PrivKey);
static s32 XMlkem_DrawSeed(u8 *SeedBuf, u32 BufLen, const u8 *Seed, u32 SeedLen, u32 ReqLen,
	const u8 **SeedPtr);
static s32 XMlkem_Pwct(u8 ParamSet, const u8 *PubKey, const u8 *PrivKey, u8 *Ct,
	u8 *SsEncaps, u8 *SsDecaps);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function generates an ML-KEM key pair (encapsulation/decapsulation keys) for
 *		the requested parameter set from an explicit or TRNG-drawn d || z seed. The keys are
 *		left in the scratch region, at XMLKEM_KEYGEN_PK_OFFSET and XMLKEM_KEYGEN_SK_OFFSET,
 *		for the caller to consume and then clear.
 *
 * @param	DmaPtr		Pointer to the ASU DMA instance.
 * @param	ParamsPtr	Pointer to the ML-KEM key generation parameters, carrying the
 *				parameter set and the expected public/secret key addresses and
 *				lengths. If SeedAddr is non-zero, seed-based KeyGen (explicit
 *				64-byte d || z seed) is used; otherwise the seed is drawn from the
 *				TRNG.
 *
 * @return
 *	- XASUFW_SUCCESS on success.
 *	- XASUFW_MLKEM_INVALID_PARAM if inputs are invalid.
 *	- XASUFW_MLKEM_UNSUPPORTED_PARAM_SET if the parameter set is unsupported.
 *	- XASUFW_MLKEM_KEYGEN_FAILED if key generation fails.
 *	- XASUFW_MLKEM_RANDOM_GEN_FAILED, folded with the TRNG error, if the seed cannot be drawn,
 *	  or if the pairwise consistency test cannot draw its test encapsulation message.
 *	- XASUFW_DMA_COPY_FAIL if the seed transfer fails.
 *	- XASUFW_MLKEM_PWCT_ENCAPS_FAIL, XASUFW_MLKEM_PWCT_DECAPS_FAIL or
 *	  XASUFW_MLKEM_PWCT_COMPARISON_FAIL if the pairwise consistency test on the generated pair
 *	  fails, in which case the pair must not be used.
 *
 *************************************************************************************************/
s32 XMlkem_KeyGen(XAsufw_Dma *DmaPtr, const XMlkem_KeyGenParams *ParamsPtr)
{
	/**
	 * Capture the start time of the ML-KEM key generation operation, if performance
	 * measurement is enabled. The reported time includes the pairwise consistency test.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_MLKEM_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 PubKeyLen = 0U;
	u32 PrivKeyLen = 0U;
	u32 CtLen = 0U;
	u8 *Base = NULL;
	u8 *LocalPubKey;
	u8 *LocalPrivKey;
	u8 *LocalCt;
	u8 *LocalSsEncaps;
	u8 *LocalSsDecaps;
	u8 LocalSeed[XASU_MLKEM_KEYGEN_SEED_LEN];
	const u8 *SeedPtr = NULL;

	if ((DmaPtr == NULL) || (ParamsPtr == NULL) || (ParamsPtr->PubKeyAddr == 0U) ||
		(ParamsPtr->PrivKeyAddr == 0U)) {
		Status = XASUFW_MLKEM_INVALID_PARAM;
		goto END;
	}

	Status = XMlkem_GetLengths(ParamsPtr->ParamSet, &PubKeyLen, &PrivKeyLen, &CtLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if ((ParamsPtr->PubKeyLen < PubKeyLen) || (ParamsPtr->PrivKeyLen < PrivKeyLen)) {
		Status = XASUFW_MLKEM_INVALID_PARAM;
		goto END;
	}

	/**
	 * Resolve the d || z seed before any key material exists: DMA in an explicit seed when one
	 * is supplied, else draw one from the TRNG. A DMA or TRNG failure is therefore reported as
	 * an ordinary error with nothing generated.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (ParamsPtr->SeedAddr != 0U) {
		if (ParamsPtr->SeedLen != XASU_MLKEM_KEYGEN_SEED_LEN) {
			Status = XASUFW_MLKEM_INVALID_PARAM;
			goto END;
		}
		Status = XAsufw_DmaXfr(DmaPtr, ParamsPtr->SeedAddr, (u64)(UINTPTR)LocalSeed,
			XASU_MLKEM_KEYGEN_SEED_LEN, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}
		SeedPtr = LocalSeed;
	} else {
		Status = XMlkem_DrawSeed(LocalSeed, XASU_MLKEM_KEYGEN_SEED_LEN, NULL, 0U,
			XASU_MLKEM_KEYGEN_SEED_LEN, &SeedPtr);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	/**
	 * Carve the scratch region at the offsets the caller reads the pair from:
	 * [public key][secret key][PCT ciphertext][PCT encaps secret][PCT decaps secret].
	 */
	Base = (u8 *)(UINTPTR)XMLKEM_SCRATCH_ADDR;
	LocalPubKey = Base + XMLKEM_KEYGEN_PK_OFFSET;
	LocalPrivKey = Base + XMLKEM_KEYGEN_SK_OFFSET;
	LocalCt = Base + XMLKEM_KEYGEN_CT_OFFSET;
	LocalSsEncaps = Base + XMLKEM_KEYGEN_SS_OFFSET;
	LocalSsDecaps = LocalSsEncaps + XASU_MLKEM_SHARED_SECRET_LEN;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XMlkem_KeypairDerandInternal(ParamsPtr->ParamSet, LocalPubKey, LocalPrivKey,
		SeedPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * Run the pairwise consistency test before the caller is told the pair is ready, so an
	 * inconsistent pair is never stored or handed out. The pair then stays where the backend
	 * wrote it: the caller reads it at XMLKEM_KEYGEN_PK_OFFSET and XMLKEM_KEYGEN_SK_OFFSET and
	 * clears it once the keys are stored, so it is never copied to a second location.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XMlkem_Pwct(ParamsPtr->ParamSet, LocalPubKey, LocalPrivKey, LocalCt, LocalSsEncaps,
		LocalSsDecaps);

END:
	/**
	 * Measure and print the performance time for the ML-KEM key generation operation, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_MLKEM_ID);

	/** d || z reconstructs the whole key pair, so it must not survive on the stack. */
	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, LocalSeed,
		XASU_MLKEM_KEYGEN_SEED_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/**
	 * Do not leave the pairwise consistency test ciphertext or its two shared secrets in the
	 * scratch region: the secrets are real key material for the pair just generated. The three
	 * are adjacent, so one call covers them.
	 */
	if (Base != NULL) {
		XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus,
			Base + XMLKEM_KEYGEN_CT_OFFSET,
			XASU_MLKEM_MAX_CT_LEN + (2U * XASU_MLKEM_SHARED_SECRET_LEN));
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs ML-KEM encapsulation: given a public key, it produces a
 *		ciphertext and the corresponding shared secret. The client public key is DMA'd in,
 *		and the produced ciphertext and shared secret are DMA'd back out. The message m is
 *		either supplied by the caller or drawn from the TRNG when none is given: the
 *		encapsulation command supplies none, while the ML-KEM KAT supplies the fixed ACVP
 *		vector so that the known answer drives this same production path.
 *
 * @param	DmaPtr		Pointer to the ASU DMA instance used to marshal client buffers.
 * @param	ParamsPtr	Pointer to the ML-KEM encapsulation parameters, carrying the
 *				parameter set, the input public (encapsulation) key and the output
 *				ciphertext and shared secret addresses and lengths.
 * @param	Seed		Explicit message m, or NULL to draw one from the TRNG.
 * @param	SeedLen		Length of Seed in bytes; must equal XASU_MLKEM_ENCAPS_SEED_LEN when
 *				Seed is supplied, and is ignored when Seed is NULL.
 *
 * @return
 *	- XASUFW_SUCCESS on success.
 *	- XASUFW_MLKEM_INVALID_PARAM if inputs are invalid.
 *	- XASUFW_MLKEM_UNSUPPORTED_PARAM_SET if the parameter set is unsupported.
 *	- XASUFW_MLKEM_ENCAPS_FAILED if encapsulation fails.
 *	- XASUFW_MLKEM_RANDOM_GEN_FAILED, folded with the TRNG error, if the seed cannot be drawn.
 *	- XASUFW_DMA_COPY_FAIL if a public key, ciphertext or shared secret transfer fails.
 *
 *************************************************************************************************/
s32 XMlkem_Encaps(XAsufw_Dma *DmaPtr, const XAsu_MlkemEncapsDecapsParams *ParamsPtr,
	const u8 *Seed, u32 SeedLen)
{
	/**
	 * Capture the start time of the ML-KEM encapsulation operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_MLKEM_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 PubKeyLen = 0U;
	u32 PrivKeyLen = 0U;
	u32 CtLen = 0U;
	u8 *LocalPubKey = NULL;
	u8 *LocalCt;
	u8 *LocalSs = NULL;
	u8 LocalSeed[XASU_MLKEM_ENCAPS_SEED_LEN];
	const u8 *SeedPtr = NULL;

	/*
	 * The key must be resolved to an address by now: a vault key ID is turned into KeyAddr by
	 * the handler before the request reaches here.
	 */
	if ((DmaPtr == NULL) || (ParamsPtr == NULL) || (ParamsPtr->KeyAddr == 0U)) {
		Status = XASUFW_MLKEM_INVALID_PARAM;
		goto END;
	}

	/** Validate the request parameters the client validates against the same contract. */
	Status = XAsu_MlkemValidateParams(ParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLKEM_INVALID_PARAM;
		goto END;
	}

	Status = XMlkem_GetLengths(ParamsPtr->ParamSet, &PubKeyLen, &PrivKeyLen, &CtLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if ((ParamsPtr->KeyLen < PubKeyLen) || (ParamsPtr->CtLen < CtLen)) {
		Status = XASUFW_MLKEM_INVALID_PARAM;
		goto END;
	}

	/**
	 * Resolve m before the ciphertext or shared secret exist, so a TRNG failure is reported
	 * as an ordinary error with nothing exported.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XMlkem_DrawSeed(LocalSeed, XASU_MLKEM_ENCAPS_SEED_LEN, Seed, SeedLen,
		XASU_MLKEM_ENCAPS_SEED_LEN, &SeedPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Carve the local scratch region: [public key][ciphertext][shared secret]. */
	LocalPubKey = (u8 *)(UINTPTR)XMLKEM_SCRATCH_ADDR;
	LocalCt = LocalPubKey + PubKeyLen;
	LocalSs = LocalCt + CtLen;

	/** DMA the client public (encapsulation) key into the local scratch region. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, ParamsPtr->KeyAddr, (u64)(UINTPTR)LocalPubKey,
		PubKeyLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XMlkem_EncapsDerandInternal(ParamsPtr->ParamSet, LocalCt, LocalSs, LocalPubKey,
		SeedPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** DMA the ciphertext and shared secret back to the client output addresses. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)LocalCt, ParamsPtr->CtAddr,
		CtLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)LocalSs, ParamsPtr->SharedSecretAddr,
		XASU_MLKEM_SHARED_SECRET_LEN, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
	}

END:
	/**
	 * Measure and print the performance time for the ML-KEM encapsulation operation, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_MLKEM_ID);

	/** m derives the shared secret, so it must not survive on the stack. */
	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, LocalSeed,
		XASU_MLKEM_ENCAPS_SEED_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/** Do not leave the shared secret in the shared scratch region. */
	if (LocalSs != NULL) {
		XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, LocalSs,
			XASU_MLKEM_SHARED_SECRET_LEN);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}

	/**
	 * The public key and the ciphertext are not secret, but clear them as well so that the
	 * next ML-KEM or ML-DSA command to take this shared region does not inherit them.
	 */
	if (LocalPubKey != NULL) {
		XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, LocalPubKey,
			PubKeyLen + CtLen);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs ML-KEM decapsulation: given a secret key and a ciphertext,
 *		it recovers the shared secret. The client secret key and ciphertext are DMA'd in,
 *		and the recovered shared secret is DMA'd back out.
 *
 * @param	DmaPtr		Pointer to the ASU DMA instance used to marshal client buffers.
 * @param	ParamsPtr	Pointer to the ML-KEM decapsulation parameters, carrying the
 *				parameter set, the input secret (decapsulation) key, the input
 *				ciphertext and the output shared secret addresses and lengths.
 *
 * @return
 *	- XASUFW_SUCCESS on success.
 *	- XASUFW_MLKEM_INVALID_PARAM if inputs are invalid.
 *	- XASUFW_MLKEM_UNSUPPORTED_PARAM_SET if the parameter set is unsupported.
 *	- XASUFW_MLKEM_DECAPS_FAILED if decapsulation fails.
 *	- XASUFW_DMA_COPY_FAIL if a secret key, ciphertext or shared secret transfer fails.
 *
 *************************************************************************************************/
s32 XMlkem_Decaps(XAsufw_Dma *DmaPtr, const XAsu_MlkemEncapsDecapsParams *ParamsPtr)
{
	/**
	 * Capture the start time of the ML-KEM decapsulation operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_MLKEM_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 PubKeyLen = 0U;
	u32 PrivKeyLen = 0U;
	u32 CtLen = 0U;
	u8 *LocalPrivKey = NULL;
	u8 *LocalCt;
	u8 *LocalSs = NULL;

	/*
	 * The key must be resolved to an address by now: a vault key ID is turned into KeyAddr by
	 * the handler before the request reaches here.
	 */
	if ((DmaPtr == NULL) || (ParamsPtr == NULL) || (ParamsPtr->KeyAddr == 0U)) {
		Status = XASUFW_MLKEM_INVALID_PARAM;
		goto END;
	}

	/** Validate the request parameters the client validates against the same contract. */
	Status = XAsu_MlkemValidateParams(ParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLKEM_INVALID_PARAM;
		goto END;
	}

	Status = XMlkem_GetLengths(ParamsPtr->ParamSet, &PubKeyLen, &PrivKeyLen, &CtLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if ((ParamsPtr->KeyLen < PrivKeyLen) || (ParamsPtr->CtLen < CtLen)) {
		Status = XASUFW_MLKEM_INVALID_PARAM;
		goto END;
	}

	/** Carve the local scratch region: [secret key][ciphertext][shared secret]. */
	LocalPrivKey = (u8 *)(UINTPTR)XMLKEM_SCRATCH_ADDR;
	LocalCt = LocalPrivKey + PrivKeyLen;
	LocalSs = LocalCt + CtLen;

	/** DMA the client secret (decapsulation) key and ciphertext into the local scratch region. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, ParamsPtr->KeyAddr, (u64)(UINTPTR)LocalPrivKey,
		PrivKeyLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, ParamsPtr->CtAddr, (u64)(UINTPTR)LocalCt,
		CtLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XMlkem_DecapsInternal(ParamsPtr->ParamSet, LocalSs, LocalCt, LocalPrivKey);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** DMA the recovered shared secret back to the client output address. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)LocalSs, ParamsPtr->SharedSecretAddr,
		XASU_MLKEM_SHARED_SECRET_LEN, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
	}

END:
	/**
	 * Measure and print the performance time for the ML-KEM decapsulation operation, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_MLKEM_ID);

	/**
	 * Do not leave the secret key or shared secret in the shared scratch region. The ciphertext
	 * sitting between them is not secret, but is cleared in the same span so that the next
	 * ML-KEM or ML-DSA command to take this region does not inherit it.
	 */
	if (LocalPrivKey != NULL) {
		XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, LocalPrivKey,
			PrivKeyLen + CtLen);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}
	if (LocalSs != NULL) {
		XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, LocalSs,
			XASU_MLKEM_SHARED_SECRET_LEN);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the public key, secret key and ciphertext lengths for the
 *		requested ML-KEM parameter set. The shared secret and both seed lengths are the
 *		same for every parameter set, so they are used directly from xasu_mlkeminfo.h.
 *
 * @param	ParamSet	ML-KEM parameter set (XAsu_MlkemParamSet).
 * @param	PubKeyLen	Pointer to store the public (encapsulation) key length.
 * @param	PrivKeyLen	Pointer to store the secret (decapsulation) key length.
 * @param	CtLen		Pointer to store the ciphertext length.
 *
 * @return
 *	- XASUFW_SUCCESS on a supported parameter set.
 *	- XASUFW_MLKEM_UNSUPPORTED_PARAM_SET otherwise.
 *
 *************************************************************************************************/
static s32 XMlkem_GetLengths(u8 ParamSet, u32 *PubKeyLen, u32 *PrivKeyLen, u32 *CtLen)
{
	s32 Status = XASUFW_MLKEM_UNSUPPORTED_PARAM_SET;

	switch (ParamSet) {
		case XASU_MLKEM_512:
			*PubKeyLen = XASU_MLKEM_512_PUB_KEY_LEN;
			*PrivKeyLen = XASU_MLKEM_512_PRIV_KEY_LEN;
			*CtLen = XASU_MLKEM_512_CT_LEN;
			Status = XASUFW_SUCCESS;
			break;
		case XASU_MLKEM_768:
			*PubKeyLen = XASU_MLKEM_768_PUB_KEY_LEN;
			*PrivKeyLen = XASU_MLKEM_768_PRIV_KEY_LEN;
			*CtLen = XASU_MLKEM_768_CT_LEN;
			Status = XASUFW_SUCCESS;
			break;
		case XASU_MLKEM_1024:
			*PubKeyLen = XASU_MLKEM_1024_PUB_KEY_LEN;
			*PrivKeyLen = XASU_MLKEM_1024_PRIV_KEY_LEN;
			*CtLen = XASU_MLKEM_1024_CT_LEN;
			Status = XASUFW_SUCCESS;
			break;
		default:
			Status = XASUFW_MLKEM_UNSUPPORTED_PARAM_SET;
			break;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Dispatches ML-KEM KeyGen_internal (seed-based) to the correct parameter set.
 *
 * @param	ParamSet	ML-KEM parameter set (XASU_MLKEM_512/_768/_1024).
 * @param	PubKey		Pointer to the output public (encapsulation) key buffer.
 * @param	PrivKey		Pointer to the output secret (decapsulation) key buffer.
 * @param	Seed		Pointer to the XASU_MLKEM_KEYGEN_SEED_LEN d || z seed.
 *
 * @return
 *	- XASUFW_SUCCESS if the key pair is generated.
 *	- XASUFW_MLKEM_KEYGEN_FAILED if the parameter set is unsupported or the backend fails.
 *
 *************************************************************************************************/
static s32 XMlkem_KeypairDerandInternal(u8 ParamSet, u8 *PubKey, u8 *PrivKey, const u8 *Seed)
{
	s32 Status = XASUFW_MLKEM_KEYGEN_FAILED;
	int OqsStatus = XMLKEM_BACKEND_FAILURE;

	switch (ParamSet) {
		case XASU_MLKEM_512:
			OqsStatus = PQCP_MLKEM_NATIVE_MLKEM512_C_keypair_derand(PubKey, PrivKey,
				Seed);
			break;
		case XASU_MLKEM_768:
			OqsStatus = PQCP_MLKEM_NATIVE_MLKEM768_C_keypair_derand(PubKey, PrivKey,
				Seed);
			break;
		case XASU_MLKEM_1024:
			OqsStatus = PQCP_MLKEM_NATIVE_MLKEM1024_C_keypair_derand(PubKey, PrivKey,
				Seed);
			break;
		default:
			OqsStatus = XMLKEM_BACKEND_FAILURE;
			break;
	}

	if (OqsStatus == XMLKEM_BACKEND_SUCCESS) {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Dispatches ML-KEM Encaps_internal (message-based) to the correct parameter set.
 *
 * @param	ParamSet	ML-KEM parameter set (XASU_MLKEM_512/_768/_1024).
 * @param	Ct		Pointer to the output ciphertext buffer.
 * @param	SharedSecret	Pointer to the output shared secret buffer.
 * @param	PubKey		Pointer to the input public (encapsulation) key.
 * @param	Seed		Pointer to the XASU_MLKEM_ENCAPS_SEED_LEN message m.
 *
 * @return
 *	- XASUFW_SUCCESS if encapsulation succeeds.
 *	- XASUFW_MLKEM_ENCAPS_FAILED if the parameter set is unsupported or the backend fails.
 *
 *************************************************************************************************/
static s32 XMlkem_EncapsDerandInternal(u8 ParamSet, u8 *Ct, u8 *SharedSecret, const u8 *PubKey,
	const u8 *Seed)
{
	s32 Status = XASUFW_MLKEM_ENCAPS_FAILED;
	int OqsStatus = XMLKEM_BACKEND_FAILURE;

	switch (ParamSet) {
		case XASU_MLKEM_512:
			OqsStatus = PQCP_MLKEM_NATIVE_MLKEM512_C_enc_derand(Ct, SharedSecret,
				PubKey, Seed);
			break;
		case XASU_MLKEM_768:
			OqsStatus = PQCP_MLKEM_NATIVE_MLKEM768_C_enc_derand(Ct, SharedSecret,
				PubKey, Seed);
			break;
		case XASU_MLKEM_1024:
			OqsStatus = PQCP_MLKEM_NATIVE_MLKEM1024_C_enc_derand(Ct, SharedSecret,
				PubKey, Seed);
			break;
		default:
			OqsStatus = XMLKEM_BACKEND_FAILURE;
			break;
	}

	if (OqsStatus == XMLKEM_BACKEND_SUCCESS) {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Dispatches ML-KEM Decaps to the correct parameter set.
 *
 * @param	ParamSet	ML-KEM parameter set (XASU_MLKEM_512/_768/_1024).
 * @param	SharedSecret	Pointer to the output shared secret buffer.
 * @param	Ct		Pointer to the input ciphertext.
 * @param	PrivKey		Pointer to the input secret (decapsulation) key.
 *
 * @return
 *	- XASUFW_SUCCESS if decapsulation succeeds.
 *	- XASUFW_MLKEM_DECAPS_FAILED if the parameter set is unsupported or the backend fails.
 *
 *************************************************************************************************/
static s32 XMlkem_DecapsInternal(u8 ParamSet, u8 *SharedSecret, const u8 *Ct, const u8 *PrivKey)
{
	s32 Status = XASUFW_MLKEM_DECAPS_FAILED;
	int OqsStatus = XMLKEM_BACKEND_FAILURE;

	switch (ParamSet) {
		case XASU_MLKEM_512:
			OqsStatus = PQCP_MLKEM_NATIVE_MLKEM512_C_dec(SharedSecret, Ct, PrivKey);
			break;
		case XASU_MLKEM_768:
			OqsStatus = PQCP_MLKEM_NATIVE_MLKEM768_C_dec(SharedSecret, Ct, PrivKey);
			break;
		case XASU_MLKEM_1024:
			OqsStatus = PQCP_MLKEM_NATIVE_MLKEM1024_C_dec(SharedSecret, Ct, PrivKey);
			break;
		default:
			OqsStatus = XMLKEM_BACKEND_FAILURE;
			break;
	}

	if (OqsStatus == XMLKEM_BACKEND_SUCCESS) {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function resolves the derandomization seed for a key generation or an
 *		encapsulation. When the caller supplies a seed (the KAT, driving a fixed ACVP
 *		vector) it is used as is; otherwise the seed is drawn from the ASU TRNG into the
 *		caller's buffer. Drawing the seed here, rather than letting the liboqs backend pull
 *		it through a randomness callback, is what allows a TRNG failure to be reported as an
 *		ordinary status before any key material is produced or exported.
 *
 * @param	SeedBuf		Buffer that receives a TRNG-drawn seed. Must be at least ReqLen
 *				bytes. Untouched when Seed is supplied.
 * @param	BufLen		Size of SeedBuf in bytes.
 * @param	Seed		Caller-supplied seed, or NULL to draw one from the TRNG.
 * @param	SeedLen		Length of the caller-supplied seed in bytes; ignored when Seed is
 *				NULL.
 * @param	ReqLen		Seed length the backend requires for the operation: 64 bytes (d || z)
 *				for key generation and 32 bytes (m) for encapsulation, the same for
 *				every parameter set.
 * @param	SeedPtr		On success, set to the seed to hand to the backend.
 *
 * @return
 *	- XASUFW_SUCCESS on success.
 *	- XASUFW_MLKEM_INVALID_PARAM if the required length does not fit the buffer or the supplied
 *	  seed length does not match what the backend requires.
 *	- XASUFW_MLKEM_RANDOM_GEN_FAILED, folded with the TRNG error, if the seed cannot be drawn.
 *
 *************************************************************************************************/
static s32 XMlkem_DrawSeed(u8 *SeedBuf, u32 BufLen, const u8 *Seed, u32 SeedLen, u32 ReqLen,
	const u8 **SeedPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	if (ReqLen > BufLen) {
		Status = XASUFW_MLKEM_INVALID_PARAM;
		goto END;
	}

	if (Seed != NULL) {
		if (SeedLen != ReqLen) {
			Status = XASUFW_MLKEM_INVALID_PARAM;
			goto END;
		}
		*SeedPtr = Seed;
		ASSIGN_VOLATILE(Status, XASUFW_SUCCESS);
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XTrng_GetRandomNumbers(SeedBuf, ReqLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MLKEM_RANDOM_GEN_FAILED);
		goto END;
	}
	*SeedPtr = SeedBuf;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function runs the FIPS 140-3 IG 10.3.A pairwise consistency test on a freshly
 *		generated ML-KEM key pair: it encapsulates to the new encapsulation key and
 *		decapsulates the resulting ciphertext with the new decapsulation key, then requires
 *		the two shared secrets to agree. Unlike the KAT, which validates the algorithm
 *		against a fixed vector, this validates the specific pair about to be handed to the
 *		client, so a glitched NTT or a partial write is caught here rather than at the peer.
 *
 * @param	ParamSet	ML-KEM parameter set (XASU_MLKEM_512/_768/_1024).
 * @param	PubKey		Freshly generated encapsulation key.
 * @param	PrivKey		Freshly generated decapsulation key.
 * @param	Ct		Scratch for the test ciphertext (parameter-set ciphertext length).
 * @param	SsEncaps	Scratch for the encapsulated shared secret.
 * @param	SsDecaps	Scratch for the decapsulated shared secret.
 *
 * @return
 *	- XASUFW_SUCCESS if both shared secrets agree.
 *	- XASUFW_MLKEM_RANDOM_GEN_FAILED, folded with the TRNG error, if the test encapsulation
 *	  message cannot be drawn.
 *	- XASUFW_MLKEM_PWCT_ENCAPS_FAIL if encapsulation to the new key fails.
 *	- XASUFW_MLKEM_PWCT_DECAPS_FAIL if decapsulation with the new key fails.
 *	- XASUFW_MLKEM_PWCT_COMPARISON_FAIL if the two shared secrets differ.
 *
 *************************************************************************************************/
static s32 XMlkem_Pwct(u8 ParamSet, const u8 *PubKey, const u8 *PrivKey, u8 *Ct,
	u8 *SsEncaps, u8 *SsDecaps)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihBufferClear = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 LocalSeed[XASU_MLKEM_ENCAPS_SEED_LEN];
	const u8 *SeedPtr = NULL;

	/*
	 * Draw the test encapsulation message from the TRNG and encapsulate through the
	 * derandomized entry point, as the command path does. All ML-KEM randomness on the ASU is
	 * drawn here and handed to the backend explicitly, so the backend never sources its own.
	 */
	Status = XMlkem_DrawSeed(LocalSeed, XASU_MLKEM_ENCAPS_SEED_LEN, NULL, 0U,
		XASU_MLKEM_ENCAPS_SEED_LEN, &SeedPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XMlkem_EncapsDerandInternal(ParamSet, Ct, SsEncaps, PubKey, SeedPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLKEM_PWCT_ENCAPS_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XMlkem_DecapsInternal(ParamSet, SsDecaps, Ct, PrivKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLKEM_PWCT_DECAPS_FAIL;
		goto END;
	}

	/*
	 * ML-KEM decapsulation is designed never to fail: an invalid ciphertext yields an implicit
	 * rejection secret rather than an error, so an inconsistent key pair shows up here as a
	 * mismatch rather than as a decapsulation failure above.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCmp_CT(SsEncaps, XASU_MLKEM_SHARED_SECRET_LEN, SsDecaps,
		XASU_MLKEM_SHARED_SECRET_LEN, XASU_MLKEM_SHARED_SECRET_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLKEM_PWCT_COMPARISON_FAIL;
	}

END:
	/** m derives the test shared secret, so it must not survive on the stack. */
	XFIH_CALL(Xil_SecureZeroize, XFihBufferClear, ClearStatus, LocalSeed,
		XASU_MLKEM_ENCAPS_SEED_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

#endif /* XASU_MLKEM_ENABLE */
/** @} */
