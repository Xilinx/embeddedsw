/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xasu_slhdsa_common.c
*
* This file contains the SLH-DSA function definitions which are common across the client
* and server. Provides parameter-set size lookups and validation of IPI parameter structures.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   kk   07/10/26 Initial release
*
* </pre>
*
*************************************************************************************************/
/*************************************** Include Files *******************************************/
/**
* @addtogroup xasu_slhdsa_common_apis SLH-DSA Common APIs
* @{
*/
#include "xasu_slhdsa_common.h"
#include "xasu_def.h"
#include "xstatus.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsu_SlhdsaValidateCtxLen(u32 CtxLen);
static u8 XAsu_SlhdsaGetPreHashHashId(u8 ParamSet);
static s32 XAsu_SlhdsaValidateParamSet(u8 ParamSet);
static s32 XAsu_SlhdsaValidateCtxAddr(u64 CtxAddr, u32 CtxLen);
static s32 XAsu_SlhdsaValidatePreHashFields(u8 ParamSet, u8 PreHashMode, u8 HashId,
					    u32 MsgLen);
static s32 XAsu_SlhdsaValidateSignMode(u8 SignMode);

/************************************ Variable Definitions ***************************************/
/*************************************************************************************************/
/**
* @brief	This function returns the public key size in bytes for the given SLH-DSA
*		parameter set.
*
* @param	ParamSet	SLH-DSA parameter set selector (XASU_SLHDSA_PARAM_*).
*
* @return
*		Public key size in bytes (32 for 128s sets, 64 for 256s sets), or 0 if ParamSet
*		is unsupported.
*
*************************************************************************************************/
u32 XAsu_SlhdsaGetPkSize(u8 ParamSet)
{
	u32 Size = 0U;

	/**
	 * Public key size is 2n bytes: 64 for 256s sets and 32 for 128s sets.
	 * Unsupported ParamSet values return 0.
	 */
	switch (ParamSet) {
		case XASU_SLHDSA_PARAM_SHAKE_256S:
		case XASU_SLHDSA_PARAM_SHA2_256S: {
			Size = XASU_SLHDSA_256S_PK_SIZE;
			break;
		}
		case XASU_SLHDSA_PARAM_SHAKE_128S:
		case XASU_SLHDSA_PARAM_SHA2_128S: {
			Size = XASU_SLHDSA_128S_PK_SIZE;
			break;
		}
		default: {
			Size = 0U;
			break;
		}
	}

	return Size;
}
/*************************************************************************************************/
/**
* @brief	This function returns the secret key size in bytes for the given SLH-DSA
*		parameter set.
*
* @param	ParamSet	SLH-DSA parameter set selector (XASU_SLHDSA_PARAM_*).
*
* @return
*		Secret key size in bytes (64 for 128s sets, 128 for 256s sets), or 0 if ParamSet
*		is unsupported.
*
*************************************************************************************************/
u32 XAsu_SlhdsaGetSkSize(u8 ParamSet)
{
	u32 Size = 0U;

	/**
	 * Secret key size is 4n bytes: 128 for 256s sets and 64 for 128s sets.
	 * Unsupported ParamSet values return 0.
	 */
	switch (ParamSet) {
		case XASU_SLHDSA_PARAM_SHAKE_256S:
		case XASU_SLHDSA_PARAM_SHA2_256S: {
			Size = XASU_SLHDSA_256S_SK_SIZE;
			break;
		}
		case XASU_SLHDSA_PARAM_SHAKE_128S:
		case XASU_SLHDSA_PARAM_SHA2_128S: {
			Size = XASU_SLHDSA_128S_SK_SIZE;
			break;
		}
		default: {
			Size = 0U;
			break;
		}
	}

	return Size;
}
/*************************************************************************************************/
/**
* @brief	This function returns the signature size in bytes for the given SLH-DSA
*		parameter set.
*
* @param	ParamSet	SLH-DSA parameter set selector (XASU_SLHDSA_PARAM_*).
*
* @return
*		Signature size in bytes (7856 for 128s sets, 29792 for 256s sets), or 0 if
*		ParamSet is unsupported.
*
*************************************************************************************************/
u32 XAsu_SlhdsaGetSigSize(u8 ParamSet)
{
	u32 Size = 0U;

	/**
	 * Signature length is fixed per parameter set (FIPS 205 Table 2). SHAKE and
	 * SHA2 share the same sizes for a given security level; unsupported
	 * selectors return 0.
	 */
	switch (ParamSet) {
		case XASU_SLHDSA_PARAM_SHAKE_256S: {
			Size = XASU_SLHDSA_SHAKE_256S_SIG_SIZE;
			break;
		}
		case XASU_SLHDSA_PARAM_SHA2_256S: {
			Size = XASU_SLHDSA_SHA2_256S_SIG_SIZE;
			break;
		}
		case XASU_SLHDSA_PARAM_SHAKE_128S: {
			Size = XASU_SLHDSA_SHAKE_128S_SIG_SIZE;
			break;
		}
		case XASU_SLHDSA_PARAM_SHA2_128S: {
			Size = XASU_SLHDSA_SHA2_128S_SIG_SIZE;
			break;
		}
		default: {
			Size = 0U;
			break;
		}
	}

	return Size;
}
/*************************************************************************************************/
/**
* @brief	This function returns the PH(M) digest length for a pre-hash HashId.
*
* @param	HashId	Pre-hash function identifier (XASU_SLHDSA_HASHID_*).
*
* @return
*		Digest length in bytes (32 for SHA-256, 64 for SHA-512), or 0 if unsupported.
*
*************************************************************************************************/
u32 XAsu_SlhdsaGetPreHashDigestLen(u8 HashId)
{
	u32 Len = 0U;

	/**
	 * Map HashId to PH(M) length: 32 bytes for SHA-256, 64 bytes for SHA-512.
	 * An unsupported HashId returns 0.
	 */
	switch (HashId) {
		case XASU_SLHDSA_HASHID_SHA256: {
			Len = XASU_SLHDSA_PREHASH_SHA256_LEN;
			break;
		}
		case XASU_SLHDSA_HASHID_SHA512: {
			Len = XASU_SLHDSA_PREHASH_SHA512_LEN;
			break;
		}
		default: {
			Len = 0U;
			break;
		}
	}

	return Len;
}
/*************************************************************************************************/
/**
* @brief	This function returns the required pre-hash HashId for a parameter set.
*
* FIPS 205 footnote 18: PH(M) must be at least 2n bytes. 128s sets (n = 16) use
* SHA-256; 256s sets (n = 32) use SHA-512.
*
* @param	ParamSet	SLH-DSA parameter set selector (XASU_SLHDSA_PARAM_*).
*
* @return
*		XASU_SLHDSA_HASHID_SHA256 or XASU_SLHDSA_HASHID_SHA512, or
*		XASU_SLHDSA_HASHID_INVALID if ParamSet is unsupported.
*
*************************************************************************************************/
static u8 XAsu_SlhdsaGetPreHashHashId(u8 ParamSet)
{
	u8 HashId = XASU_SLHDSA_HASHID_INVALID;

	/**
	 * 256s sets require SHA-512 (n = 32); 128s sets require SHA-256 (n = 16).
	 * An unsupported ParamSet returns XASU_SLHDSA_HASHID_INVALID.
	 */
	switch (ParamSet) {
		case XASU_SLHDSA_PARAM_SHAKE_256S:
		case XASU_SLHDSA_PARAM_SHA2_256S: {
			HashId = XASU_SLHDSA_HASHID_SHA512;
			break;
		}
		case XASU_SLHDSA_PARAM_SHAKE_128S:
		case XASU_SLHDSA_PARAM_SHA2_128S: {
			HashId = XASU_SLHDSA_HASHID_SHA256;
			break;
		}
		default: {
			HashId = XASU_SLHDSA_HASHID_INVALID;
			break;
		}
	}

	return HashId;
}
/*************************************************************************************************/
/**
* @brief	This function validates the SLH-DSA parameter set selector.
*
* @param	ParamSet	SLH-DSA parameter set selector (XASU_SLHDSA_PARAM_*).
*
* @return
*		- XST_SUCCESS, if ParamSet is a supported value (0..3).
*		- XST_INVALID_PARAM, if ParamSet is out of range.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaValidateParamSet(u8 ParamSet)
{
	s32 Status = (s32)XST_FAILURE;

	/**
	 * The supported parameter sets are numbered contiguously with XASU_SLHDSA_PARAM_SHA2_128S
	 * as the highest value, so a single upper-bound check rejects every unsupported selector.
	 */
	if (ParamSet > XASU_SLHDSA_PARAM_SHA2_128S) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	Status = (s32)XST_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	This function validates SLH-DSA sign parameters.
*
* @param	ParamsPtr	Pointer to XAsu_SlhdsaSignParams.
*
* @return
*		- XST_SUCCESS, if validation succeeds.
*		- XST_INVALID_PARAM, if any field is invalid (NULL pointer, zero addresses,
*		  invalid ParamSet, CtxLen/CtxAddr mismatch, MsgLen exceeds DMA limit,
*		  SignatureLen mismatch, or SkLen not equal to the ParamSet secret-key size).
*
*************************************************************************************************/
s32 XAsu_SlhdsaValidateSignParams(const XAsu_SlhdsaSignParams *ParamsPtr)
{
	s32 Status = (s32)XST_FAILURE;
	u32 SigSize;
	u32 SkSize;

	/** Validate input pointer. */
	if (ParamsPtr == NULL) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * Validate the parameter set first, since the expected signature size and the required
	 * pre-hash function are both derived from it.
	 */
	if (XAsu_SlhdsaValidateParamSet(ParamsPtr->ParamSet) != XST_SUCCESS) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/** Validate the context length against the FIPS 205 limit of 255 bytes. */
	if (XAsu_SlhdsaValidateCtxLen(ParamsPtr->CtxLen) != XST_SUCCESS) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/** Validate that a context address is supplied whenever a non-empty context is used. */
	if (XAsu_SlhdsaValidateCtxAddr(ParamsPtr->CtxAddr, ParamsPtr->CtxLen) != XST_SUCCESS) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * Bound the message length by what the ASU DMA can transfer in a single operation, as
	 * the message is fed to the hash engine without being split.
	 */
	if (ParamsPtr->MsgLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * The secret key, message and signature addresses are all mandatory, so a zero address
	 * in any of them is rejected before ASUFW dereferences it.
	 */
	if ((ParamsPtr->SkObj.SkAddr == 0U) || (ParamsPtr->MsgAddr == 0U) ||
	    (ParamsPtr->SignatureAddr == 0U)) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * The secret key is a fixed length per parameter set, so SkLen must match exactly. A
	 * caller declaring any other length has not supplied the whole key.
	 */
	SkSize = XAsu_SlhdsaGetSkSize(ParamsPtr->ParamSet);
	if (ParamsPtr->SkObj.SkLen != SkSize) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * SLH-DSA signatures are a fixed length per parameter set, so SignatureLen must match
	 * exactly. This is what guarantees the output buffer is large enough to be written.
	 */
	SigSize = XAsu_SlhdsaGetSigSize(ParamsPtr->ParamSet);
	if (ParamsPtr->SignatureLen != SigSize) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * Validate the pure versus pre-hash message mode, and in pre-hash mode that HashId is
	 * the one required by the parameter set and MsgLen is the matching digest length.
	 */
	Status = XAsu_SlhdsaValidatePreHashFields(ParamsPtr->ParamSet, ParamsPtr->PreHashMode,
						  ParamsPtr->HashId, ParamsPtr->MsgLen);
	if (Status != XST_SUCCESS) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/** Validate the sign mode, which selects hedged or deterministic signing. */
	if (XAsu_SlhdsaValidateSignMode(ParamsPtr->SignMode) != XST_SUCCESS) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	Status = (s32)XST_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	This function validates SLH-DSA signature verification parameters.
*
* @param	ParamsPtr	Pointer to XAsu_SlhdsaSignVerifyParams.
*
* @return
*		- XST_SUCCESS, if validation succeeds.
*		- XST_INVALID_PARAM, if any field is invalid (NULL pointer, zero addresses,
*		  invalid ParamSet, CtxLen/CtxAddr mismatch, MsgLen exceeds DMA limit,
*		  SignatureLen mismatch, or PubKeyLen not equal to the ParamSet public key size).
*
*************************************************************************************************/
s32 XAsu_SlhdsaValidateSignVerifyParams(const XAsu_SlhdsaSignVerifyParams *ParamsPtr)
{
	s32 Status = (s32)XST_FAILURE;
	u32 SigSize;

	/** Validate input pointer. */
	if (ParamsPtr == NULL) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/** Validate the parameter set, which fixes the expected signature and public key sizes. */
	if (XAsu_SlhdsaValidateParamSet(ParamsPtr->ParamSet) != XST_SUCCESS) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/** Validate the context length against the FIPS 205 limit of 255 bytes. */
	if (XAsu_SlhdsaValidateCtxLen(ParamsPtr->CtxLen) != XST_SUCCESS) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * Validate that a context address is supplied whenever a non-empty context is used.
	 * Both CtxLen and CtxAddr must reproduce the values used at signing time, because
	 * the context is bound into the message digest.
	 */
	if (XAsu_SlhdsaValidateCtxAddr(ParamsPtr->CtxAddr, ParamsPtr->CtxLen) != XST_SUCCESS) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/** Bound the message length by the maximum single ASU DMA transfer length. */
	if (ParamsPtr->MsgLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/** The public key, message and signature addresses are all mandatory for verification. */
	if ((ParamsPtr->PkObj.PubKeyAddr == 0U) || (ParamsPtr->MsgAddr == 0U) ||
	    (ParamsPtr->SignatureAddr == 0U)) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * Reject a signature whose length does not match the parameter set, so that a truncated
	 * or oversized signature is never parsed as if it were well formed.
	 */
	SigSize = XAsu_SlhdsaGetSigSize(ParamsPtr->ParamSet);
	if (ParamsPtr->SignatureLen != SigSize) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * The public key is a fixed length per parameter set, so PubKeyLen must match exactly.
	 * Verification always reads the whole key, so a shorter declared length would leave part
	 * of it unvalidated.
	 */
	if (ParamsPtr->PkObj.PubKeyLen != XAsu_SlhdsaGetPkSize(ParamsPtr->ParamSet)) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/** Validate the message mode and, for pre-hash mode, the HashId and digest length. */
	Status = XAsu_SlhdsaValidatePreHashFields(ParamsPtr->ParamSet, ParamsPtr->PreHashMode,
						  ParamsPtr->HashId, ParamsPtr->MsgLen);
	if (Status != XST_SUCCESS) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	Status = (s32)XST_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	This function validates the FIPS 205 context length field.
*
* @param	CtxLen	Context length: 0..XASU_SLHDSA_MAX_CTX_LEN (FIPS context).
*
* @return
*		- XST_SUCCESS, if CtxLen is valid.
*		- XST_INVALID_PARAM, if CtxLen exceeds XASU_SLHDSA_MAX_CTX_LEN.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaValidateCtxLen(u32 CtxLen)
{
	s32 Status = (s32)XST_FAILURE;

	/**
	 * FIPS 205 encodes the context length in a single byte, so any value above
	 * XASU_SLHDSA_MAX_CTX_LEN cannot be represented in the signed message prefix. CtxLen
	 * zero is valid and selects the empty context.
	 */
	if (CtxLen > XASU_SLHDSA_MAX_CTX_LEN) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	Status = (s32)XST_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	This function validates that CtxAddr is provided when a non-empty context
*		is specified.
*
* @param	CtxAddr	DMA address of context bytes (required when CtxLen is 1..255).
* @param	CtxLen	Context length field from sign or verify parameters.
*
* @return
*		- XST_SUCCESS, if CtxAddr is valid for the given CtxLen.
*		- XST_INVALID_PARAM, if CtxLen is 1..255 and CtxAddr is zero.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaValidateCtxAddr(u64 CtxAddr, u32 CtxLen)
{
	s32 Status = (s32)XST_FAILURE;

	/**
	 * A non-zero context length promises context bytes to hash, so the address must be
	 * present. The reverse case is allowed: a non-zero address with CtxLen zero is simply
	 * ignored and treated as the empty context.
	 */
	if ((CtxLen > 0U) && (CtxAddr == 0U)) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	Status = (s32)XST_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	This function validates the SLH-DSA sign mode field.
*
* @param	SignMode	XASU_SLHDSA_SIGN_MODE_HEDGED or XASU_SLHDSA_SIGN_MODE_DETERMINISTIC.
*
* @return
*		- XST_SUCCESS, if SignMode is valid.
*		- XST_INVALID_PARAM, otherwise.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaValidateSignMode(u8 SignMode)
{
	s32 Status = (s32)XST_FAILURE;

	/**
	 * Accept only the two defined modes. Any other value is rejected rather than being
	 * silently treated as hedged, so a corrupted field cannot quietly change how the
	 * signature randomizer is derived.
	 */
	if ((SignMode != XASU_SLHDSA_SIGN_MODE_HEDGED) &&
	    (SignMode != XASU_SLHDSA_SIGN_MODE_DETERMINISTIC)) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	Status = (s32)XST_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	This function validates HashSLH-DSA pre-hash fields on sign/verify parameters.
*
* @param	ParamSet	Parameter set selector (determines required HashId).
* @param	PreHashMode	XASU_SLHDSA_MSG_PURE or XASU_SLHDSA_MSG_PREHASH.
* @param	HashId		Pre-hash function identifier.
* @param	MsgLen		Message or PH(M) digest length in bytes.
*
* @return
*		- XST_SUCCESS, if pre-hash fields are valid.
*		- XST_INVALID_PARAM, otherwise.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaValidatePreHashFields(u8 ParamSet, u8 PreHashMode, u8 HashId,
					    u32 MsgLen)
{
	s32 Status = (s32)XST_FAILURE;
	u32 DigestLen;

	/** Accept only the two defined message modes. */
	if ((PreHashMode != XASU_SLHDSA_MSG_PURE) &&
	    (PreHashMode != XASU_SLHDSA_MSG_PREHASH)) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * Pure SLH-DSA hashes the message itself, so HashId is unused and MsgLen is already
	 * bounded by the caller. Nothing further to check.
	 */
	if (PreHashMode == XASU_SLHDSA_MSG_PURE) {
		Status = (s32)XST_SUCCESS;
		goto END;
	}

	/**
	 * HashSLH-DSA fixes the pre-hash function per parameter set, so that PH(M) is at least
	 * 2n bytes (FIPS 205 footnote 18). A caller supplied HashId that disagrees is rejected
	 * instead of being overridden.
	 */
	if (HashId != XAsu_SlhdsaGetPreHashHashId(ParamSet)) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/** A zero digest length means HashId is not a supported pre-hash function. */
	DigestLen = XAsu_SlhdsaGetPreHashDigestLen(HashId);
	if (DigestLen == 0U) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * In pre-hash mode the caller passes PH(M) rather than the message, so MsgLen must be
	 * exactly the digest length of the selected pre-hash function.
	 */
	if (MsgLen != DigestLen) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	Status = (s32)XST_SUCCESS;

END:
	return Status;
}
/** @} */
