/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file oqs_mldsa_internal.h
 *
 * This header is the ASU firmware CONTRACT for the lower-level (internal) ML-DSA entry points
 * exposed by the liboqs "mldsa-native" backend. liboqs's generic OQS_SIG API only exposes
 * pure + context signing; to support the FULL FIPS 204 surface (HashML-DSA pre-hash, hedged
 * AND deterministic signing, and seed-based KeyGen) the firmware calls these internal symbols
 * directly.
 *
 * These prototypes MUST match the symbols exported by liboqs.a exactly. The exact link-level
 * names were verified via `nm liboqs.a` and follow the pattern:
 *     PQCP_MLDSA_NATIVE_MLDSA{44|65|87}_C_<function>
 *
 * The build of liboqs used here does NOT define MLD_CONFIG_CONTEXT_PARAMETER, therefore the
 * functions have NO trailing application-context argument (context-less form). Defining it
 * would add that argument without changing a single symbol name, so the mismatch would link
 * silently. build_liboqs.sh checks for exactly that before it builds: it compiles these
 * declarations alongside mldsa-native's own, where a disagreement is a compile error. Keep
 * the two in step, and see README.liboqs.md section 2 if that check fires.
 *
 * NOTE: The liboqs drop agent must ensure these symbols are exported (not stripped) from
 *       liboqs.a. This header is the single source of truth; the drop must conform to it.
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
#ifndef OQS_MLDSA_INTERNAL_H_
#define OQS_MLDSA_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include <stddef.h>
#include <stdint.h>

/************************************ Constant Definitions ***************************************/
/* Byte lengths shared across all ML-DSA parameter sets (FIPS 204). */
#define MLDSA_SEEDBYTES		(32U) /**< ML-DSA KeyGen xi seed length in bytes */
#define MLDSA_RNDBYTES		(32U) /**< ML-DSA signing randomness length in bytes */
#define MLDSA_CRHBYTES		(64U) /**< ML-DSA message representative (mu) length in bytes */

/*
 * Pre-hash (HashML-DSA) hash algorithm constants, matching mldsa-native's MLD_PREHASH_*.
 * Passed as the 'hashalg' argument to the *_pre_hash_internal entry points.
 */
#define MLD_PREHASH_NONE	(0)  /**< Pure ML-DSA (no pre-hash) */
#define MLD_PREHASH_SHA2_224	(1)  /**< SHA2-224 pre-hash */
#define MLD_PREHASH_SHA2_256	(2)  /**< SHA2-256 pre-hash */
#define MLD_PREHASH_SHA2_384	(3)  /**< SHA2-384 pre-hash */
#define MLD_PREHASH_SHA2_512	(4)  /**< SHA2-512 pre-hash */
#define MLD_PREHASH_SHA2_512_224 (5) /**< SHA2-512/224 pre-hash */
#define MLD_PREHASH_SHA2_512_256 (6) /**< SHA2-512/256 pre-hash */
#define MLD_PREHASH_SHA3_224	(7)  /**< SHA3-224 pre-hash */
#define MLD_PREHASH_SHA3_256	(8)  /**< SHA3-256 pre-hash */
#define MLD_PREHASH_SHA3_384	(9)  /**< SHA3-384 pre-hash */
#define MLD_PREHASH_SHA3_512	(10) /**< SHA3-512 pre-hash */
#define MLD_PREHASH_SHAKE_128	(11) /**< SHAKE128 pre-hash */
#define MLD_PREHASH_SHAKE_256	(12) /**< SHAKE256 pre-hash */

/**
 * Maximum formatted domain-separation prefix length (mldsa-native):
 *   - Pure     : 0x00 || ctxlen(1) || ctx(<=255)
 *   - HashML-DSA: 0x01 || ctxlen(1) || ctx(<=255) || oid(11) || ph(<=64)
 */
#define MLD_DOMAIN_SEPARATION_MAX_BYTES	(2U + 255U + 11U + 64U)

/************************************ Function Prototypes ****************************************/
/*
 * The internal entry points, one set per parameter set. Buffer lengths that are not stated with
 * the argument are the parameter-set lengths from FIPS 204.
 */

/* ------------------------------- ML-DSA-44 ------------------------------- */
/*************************************************************************************************/
/**
 * @brief	This function generates an ML-DSA-44 key pair from the given xi seed.
 *
 * @param	PubKey		Pointer to the buffer which holds the generated public key.
 * @param	SecKey		Pointer to the buffer which holds the generated secret key.
 * @param	Seed		Pointer to the MLDSA_SEEDBYTES xi seed.
 *
 * @return
 *	- 0 on success.
 *	- Non-zero mldsa-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLDSA_NATIVE_MLDSA44_C_keypair_internal(uint8_t *PubKey, uint8_t *SecKey,
	const uint8_t *Seed);

/*************************************************************************************************/
/**
 * @brief	This function generates an ML-DSA-44 signature, taking the domain-separation prefix
 *		and the signing randomness from the caller.
 *
 * @param	Sig		Pointer to the buffer which holds the generated signature.
 * @param	SigLen		Pointer to the variable which holds the generated signature length.
 * @param	Msg		Pointer to the raw message, or to the precomputed mu when
 *				ExternalMu is non-zero.
 * @param	MsgLen		Length of the message in bytes, MLDSA_CRHBYTES when ExternalMu is
 *				non-zero.
 * @param	Pre		Pointer to the domain-separation prefix produced by
 *				PQCP_MLDSA_NATIVE_MLDSA44_C_prepare_domain_separation_prefix().
 * @param	PreLen		Length of the domain-separation prefix in bytes.
 * @param	Rnd		Pointer to the MLDSA_RNDBYTES signing randomness. All-zero selects
 *				deterministic signing, random bytes select hedged signing.
 * @param	SecKey		Pointer to the secret key.
 * @param	ExternalMu	Zero when Msg is the raw message, non-zero when Msg is a
 *				precomputed mu.
 *
 * @return
 *	- 0 on success.
 *	- Non-zero mldsa-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLDSA_NATIVE_MLDSA44_C_signature_internal(uint8_t *Sig, size_t *SigLen,
	const uint8_t *Msg, size_t MsgLen, const uint8_t *Pre, size_t PreLen,
	const uint8_t *Rnd, const uint8_t *SecKey, int ExternalMu);

/*************************************************************************************************/
/**
 * @brief	This function generates a HashML-DSA-44 signature over a pre-hash digest, forming
 *		the domain-separation prefix internally from the context and the hash algorithm.
 *
 * @param	Sig		Pointer to the buffer which holds the generated signature.
 * @param	SigLen		Pointer to the variable which holds the generated signature length.
 * @param	PreHash		Pointer to the pre-hash digest of the message.
 * @param	PreHashLen	Length of the pre-hash digest in bytes.
 * @param	Ctx		Pointer to the context string, may be NULL when CtxLen is zero.
 * @param	CtxLen		Length of the context string in bytes, 0 to 255.
 * @param	Rnd		Pointer to the MLDSA_RNDBYTES signing randomness. All-zero selects
 *				deterministic signing, random bytes select hedged signing.
 * @param	SecKey		Pointer to the secret key.
 * @param	HashAlg		Pre-hash algorithm, one of MLD_PREHASH_*.
 *
 * @return
 *	- 0 on success.
 *	- Non-zero mldsa-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLDSA_NATIVE_MLDSA44_C_signature_pre_hash_internal(uint8_t *Sig, size_t *SigLen,
	const uint8_t *PreHash, size_t PreHashLen, const uint8_t *Ctx, size_t CtxLen,
	const uint8_t *Rnd, const uint8_t *SecKey, int HashAlg);

/*************************************************************************************************/
/**
 * @brief	This function verifies an ML-DSA-44 signature, taking the domain-separation prefix
 *		from the caller.
 *
 * @param	Sig		Pointer to the signature to be verified.
 * @param	SigLen		Length of the signature in bytes.
 * @param	Msg		Pointer to the raw message, or to the precomputed mu when
 *				ExternalMu is non-zero.
 * @param	MsgLen		Length of the message in bytes, MLDSA_CRHBYTES when ExternalMu is
 *				non-zero.
 * @param	Pre		Pointer to the domain-separation prefix produced by
 *				PQCP_MLDSA_NATIVE_MLDSA44_C_prepare_domain_separation_prefix().
 * @param	PreLen		Length of the domain-separation prefix in bytes.
 * @param	PubKey		Pointer to the public key.
 * @param	ExternalMu	Zero when Msg is the raw message, non-zero when Msg is a
 *				precomputed mu.
 *
 * @return
 *	- 0 if the signature is valid.
 *	- Non-zero mldsa-native error code if the signature is invalid or on failure.
 *
 *************************************************************************************************/
int PQCP_MLDSA_NATIVE_MLDSA44_C_verify_internal(const uint8_t *Sig, size_t SigLen,
	const uint8_t *Msg, size_t MsgLen, const uint8_t *Pre, size_t PreLen,
	const uint8_t *PubKey, int ExternalMu);

/*************************************************************************************************/
/**
 * @brief	This function verifies a HashML-DSA-44 signature over a pre-hash digest, forming
 *		the domain-separation prefix internally from the context and the hash algorithm.
 *
 * @param	Sig		Pointer to the signature to be verified.
 * @param	SigLen		Length of the signature in bytes.
 * @param	PreHash		Pointer to the pre-hash digest of the message.
 * @param	PreHashLen	Length of the pre-hash digest in bytes.
 * @param	Ctx		Pointer to the context string, may be NULL when CtxLen is zero.
 * @param	CtxLen		Length of the context string in bytes, 0 to 255.
 * @param	PubKey		Pointer to the public key.
 * @param	HashAlg		Pre-hash algorithm, one of MLD_PREHASH_*.
 *
 * @return
 *	- 0 if the signature is valid.
 *	- Non-zero mldsa-native error code if the signature is invalid or on failure.
 *
 *************************************************************************************************/
int PQCP_MLDSA_NATIVE_MLDSA44_C_verify_pre_hash_internal(const uint8_t *Sig, size_t SigLen,
	const uint8_t *PreHash, size_t PreHashLen, const uint8_t *Ctx, size_t CtxLen,
	const uint8_t *PubKey, int HashAlg);

/*************************************************************************************************/
/**
 * @brief	This function formats the ML-DSA-44 domain-separation prefix for the given context
 *		and pre-hash digest, for use with the signature and verify entry points.
 *
 * @param	Prefix		Pointer to the buffer which holds the formatted prefix. It must be
 *				at least MLD_DOMAIN_SEPARATION_MAX_BYTES long.
 * @param	PreHash		Pointer to the pre-hash digest, NULL for pure ML-DSA.
 * @param	PreHashLen	Length of the pre-hash digest in bytes, 0 for pure ML-DSA.
 * @param	Ctx		Pointer to the context string, may be NULL when CtxLen is zero.
 * @param	CtxLen		Length of the context string in bytes, 0 to 255.
 * @param	HashAlg		Pre-hash algorithm, one of MLD_PREHASH_*, MLD_PREHASH_NONE for
 *				pure ML-DSA.
 *
 * @return
 *	- Length of the formatted prefix in bytes.
 *
 *************************************************************************************************/
size_t PQCP_MLDSA_NATIVE_MLDSA44_C_prepare_domain_separation_prefix(uint8_t *Prefix,
	const uint8_t *PreHash, size_t PreHashLen, const uint8_t *Ctx, size_t CtxLen, int HashAlg);

/* ------------------------------- ML-DSA-65 ------------------------------- */
/*************************************************************************************************/
/**
 * @brief	This function generates an ML-DSA-65 key pair from the given xi seed.
 *
 * @param	PubKey		Pointer to the buffer which holds the generated public key.
 * @param	SecKey		Pointer to the buffer which holds the generated secret key.
 * @param	Seed		Pointer to the MLDSA_SEEDBYTES xi seed.
 *
 * @return
 *	- 0 on success.
 *	- Non-zero mldsa-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLDSA_NATIVE_MLDSA65_C_keypair_internal(uint8_t *PubKey, uint8_t *SecKey,
	const uint8_t *Seed);

/*************************************************************************************************/
/**
 * @brief	This function generates an ML-DSA-65 signature, taking the domain-separation prefix
 *		and the signing randomness from the caller.
 *
 * @param	Sig		Pointer to the buffer which holds the generated signature.
 * @param	SigLen		Pointer to the variable which holds the generated signature length.
 * @param	Msg		Pointer to the raw message, or to the precomputed mu when
 *				ExternalMu is non-zero.
 * @param	MsgLen		Length of the message in bytes, MLDSA_CRHBYTES when ExternalMu is
 *				non-zero.
 * @param	Pre		Pointer to the domain-separation prefix produced by
 *				PQCP_MLDSA_NATIVE_MLDSA65_C_prepare_domain_separation_prefix().
 * @param	PreLen		Length of the domain-separation prefix in bytes.
 * @param	Rnd		Pointer to the MLDSA_RNDBYTES signing randomness. All-zero selects
 *				deterministic signing, random bytes select hedged signing.
 * @param	SecKey		Pointer to the secret key.
 * @param	ExternalMu	Zero when Msg is the raw message, non-zero when Msg is a
 *				precomputed mu.
 *
 * @return
 *	- 0 on success.
 *	- Non-zero mldsa-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLDSA_NATIVE_MLDSA65_C_signature_internal(uint8_t *Sig, size_t *SigLen,
	const uint8_t *Msg, size_t MsgLen, const uint8_t *Pre, size_t PreLen,
	const uint8_t *Rnd, const uint8_t *SecKey, int ExternalMu);

/*************************************************************************************************/
/**
 * @brief	This function generates a HashML-DSA-65 signature over a pre-hash digest, forming
 *		the domain-separation prefix internally from the context and the hash algorithm.
 *
 * @param	Sig		Pointer to the buffer which holds the generated signature.
 * @param	SigLen		Pointer to the variable which holds the generated signature length.
 * @param	PreHash		Pointer to the pre-hash digest of the message.
 * @param	PreHashLen	Length of the pre-hash digest in bytes.
 * @param	Ctx		Pointer to the context string, may be NULL when CtxLen is zero.
 * @param	CtxLen		Length of the context string in bytes, 0 to 255.
 * @param	Rnd		Pointer to the MLDSA_RNDBYTES signing randomness. All-zero selects
 *				deterministic signing, random bytes select hedged signing.
 * @param	SecKey		Pointer to the secret key.
 * @param	HashAlg		Pre-hash algorithm, one of MLD_PREHASH_*.
 *
 * @return
 *	- 0 on success.
 *	- Non-zero mldsa-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLDSA_NATIVE_MLDSA65_C_signature_pre_hash_internal(uint8_t *Sig, size_t *SigLen,
	const uint8_t *PreHash, size_t PreHashLen, const uint8_t *Ctx, size_t CtxLen,
	const uint8_t *Rnd, const uint8_t *SecKey, int HashAlg);

/*************************************************************************************************/
/**
 * @brief	This function verifies an ML-DSA-65 signature, taking the domain-separation prefix
 *		from the caller.
 *
 * @param	Sig		Pointer to the signature to be verified.
 * @param	SigLen		Length of the signature in bytes.
 * @param	Msg		Pointer to the raw message, or to the precomputed mu when
 *				ExternalMu is non-zero.
 * @param	MsgLen		Length of the message in bytes, MLDSA_CRHBYTES when ExternalMu is
 *				non-zero.
 * @param	Pre		Pointer to the domain-separation prefix produced by
 *				PQCP_MLDSA_NATIVE_MLDSA65_C_prepare_domain_separation_prefix().
 * @param	PreLen		Length of the domain-separation prefix in bytes.
 * @param	PubKey		Pointer to the public key.
 * @param	ExternalMu	Zero when Msg is the raw message, non-zero when Msg is a
 *				precomputed mu.
 *
 * @return
 *	- 0 if the signature is valid.
 *	- Non-zero mldsa-native error code if the signature is invalid or on failure.
 *
 *************************************************************************************************/
int PQCP_MLDSA_NATIVE_MLDSA65_C_verify_internal(const uint8_t *Sig, size_t SigLen,
	const uint8_t *Msg, size_t MsgLen, const uint8_t *Pre, size_t PreLen,
	const uint8_t *PubKey, int ExternalMu);

/*************************************************************************************************/
/**
 * @brief	This function verifies a HashML-DSA-65 signature over a pre-hash digest, forming
 *		the domain-separation prefix internally from the context and the hash algorithm.
 *
 * @param	Sig		Pointer to the signature to be verified.
 * @param	SigLen		Length of the signature in bytes.
 * @param	PreHash		Pointer to the pre-hash digest of the message.
 * @param	PreHashLen	Length of the pre-hash digest in bytes.
 * @param	Ctx		Pointer to the context string, may be NULL when CtxLen is zero.
 * @param	CtxLen		Length of the context string in bytes, 0 to 255.
 * @param	PubKey		Pointer to the public key.
 * @param	HashAlg		Pre-hash algorithm, one of MLD_PREHASH_*.
 *
 * @return
 *	- 0 if the signature is valid.
 *	- Non-zero mldsa-native error code if the signature is invalid or on failure.
 *
 *************************************************************************************************/
int PQCP_MLDSA_NATIVE_MLDSA65_C_verify_pre_hash_internal(const uint8_t *Sig, size_t SigLen,
	const uint8_t *PreHash, size_t PreHashLen, const uint8_t *Ctx, size_t CtxLen,
	const uint8_t *PubKey, int HashAlg);

/*************************************************************************************************/
/**
 * @brief	This function formats the ML-DSA-65 domain-separation prefix for the given context
 *		and pre-hash digest, for use with the signature and verify entry points.
 *
 * @param	Prefix		Pointer to the buffer which holds the formatted prefix. It must be
 *				at least MLD_DOMAIN_SEPARATION_MAX_BYTES long.
 * @param	PreHash		Pointer to the pre-hash digest, NULL for pure ML-DSA.
 * @param	PreHashLen	Length of the pre-hash digest in bytes, 0 for pure ML-DSA.
 * @param	Ctx		Pointer to the context string, may be NULL when CtxLen is zero.
 * @param	CtxLen		Length of the context string in bytes, 0 to 255.
 * @param	HashAlg		Pre-hash algorithm, one of MLD_PREHASH_*, MLD_PREHASH_NONE for
 *				pure ML-DSA.
 *
 * @return
 *	- Length of the formatted prefix in bytes.
 *
 *************************************************************************************************/
size_t PQCP_MLDSA_NATIVE_MLDSA65_C_prepare_domain_separation_prefix(uint8_t *Prefix,
	const uint8_t *PreHash, size_t PreHashLen, const uint8_t *Ctx, size_t CtxLen, int HashAlg);

/* ------------------------------- ML-DSA-87 ------------------------------- */
/*************************************************************************************************/
/**
 * @brief	This function generates an ML-DSA-87 key pair from the given xi seed.
 *
 * @param	PubKey		Pointer to the buffer which holds the generated public key.
 * @param	SecKey		Pointer to the buffer which holds the generated secret key.
 * @param	Seed		Pointer to the MLDSA_SEEDBYTES xi seed.
 *
 * @return
 *	- 0 on success.
 *	- Non-zero mldsa-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLDSA_NATIVE_MLDSA87_C_keypair_internal(uint8_t *PubKey, uint8_t *SecKey,
	const uint8_t *Seed);

/*************************************************************************************************/
/**
 * @brief	This function generates an ML-DSA-87 signature, taking the domain-separation prefix
 *		and the signing randomness from the caller.
 *
 * @param	Sig		Pointer to the buffer which holds the generated signature.
 * @param	SigLen		Pointer to the variable which holds the generated signature length.
 * @param	Msg		Pointer to the raw message, or to the precomputed mu when
 *				ExternalMu is non-zero.
 * @param	MsgLen		Length of the message in bytes, MLDSA_CRHBYTES when ExternalMu is
 *				non-zero.
 * @param	Pre		Pointer to the domain-separation prefix produced by
 *				PQCP_MLDSA_NATIVE_MLDSA87_C_prepare_domain_separation_prefix().
 * @param	PreLen		Length of the domain-separation prefix in bytes.
 * @param	Rnd		Pointer to the MLDSA_RNDBYTES signing randomness. All-zero selects
 *				deterministic signing, random bytes select hedged signing.
 * @param	SecKey		Pointer to the secret key.
 * @param	ExternalMu	Zero when Msg is the raw message, non-zero when Msg is a
 *				precomputed mu.
 *
 * @return
 *	- 0 on success.
 *	- Non-zero mldsa-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLDSA_NATIVE_MLDSA87_C_signature_internal(uint8_t *Sig, size_t *SigLen,
	const uint8_t *Msg, size_t MsgLen, const uint8_t *Pre, size_t PreLen,
	const uint8_t *Rnd, const uint8_t *SecKey, int ExternalMu);

/*************************************************************************************************/
/**
 * @brief	This function generates a HashML-DSA-87 signature over a pre-hash digest, forming
 *		the domain-separation prefix internally from the context and the hash algorithm.
 *
 * @param	Sig		Pointer to the buffer which holds the generated signature.
 * @param	SigLen		Pointer to the variable which holds the generated signature length.
 * @param	PreHash		Pointer to the pre-hash digest of the message.
 * @param	PreHashLen	Length of the pre-hash digest in bytes.
 * @param	Ctx		Pointer to the context string, may be NULL when CtxLen is zero.
 * @param	CtxLen		Length of the context string in bytes, 0 to 255.
 * @param	Rnd		Pointer to the MLDSA_RNDBYTES signing randomness. All-zero selects
 *				deterministic signing, random bytes select hedged signing.
 * @param	SecKey		Pointer to the secret key.
 * @param	HashAlg		Pre-hash algorithm, one of MLD_PREHASH_*.
 *
 * @return
 *	- 0 on success.
 *	- Non-zero mldsa-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLDSA_NATIVE_MLDSA87_C_signature_pre_hash_internal(uint8_t *Sig, size_t *SigLen,
	const uint8_t *PreHash, size_t PreHashLen, const uint8_t *Ctx, size_t CtxLen,
	const uint8_t *Rnd, const uint8_t *SecKey, int HashAlg);

/*************************************************************************************************/
/**
 * @brief	This function verifies an ML-DSA-87 signature, taking the domain-separation prefix
 *		from the caller.
 *
 * @param	Sig		Pointer to the signature to be verified.
 * @param	SigLen		Length of the signature in bytes.
 * @param	Msg		Pointer to the raw message, or to the precomputed mu when
 *				ExternalMu is non-zero.
 * @param	MsgLen		Length of the message in bytes, MLDSA_CRHBYTES when ExternalMu is
 *				non-zero.
 * @param	Pre		Pointer to the domain-separation prefix produced by
 *				PQCP_MLDSA_NATIVE_MLDSA87_C_prepare_domain_separation_prefix().
 * @param	PreLen		Length of the domain-separation prefix in bytes.
 * @param	PubKey		Pointer to the public key.
 * @param	ExternalMu	Zero when Msg is the raw message, non-zero when Msg is a
 *				precomputed mu.
 *
 * @return
 *	- 0 if the signature is valid.
 *	- Non-zero mldsa-native error code if the signature is invalid or on failure.
 *
 *************************************************************************************************/
int PQCP_MLDSA_NATIVE_MLDSA87_C_verify_internal(const uint8_t *Sig, size_t SigLen,
	const uint8_t *Msg, size_t MsgLen, const uint8_t *Pre, size_t PreLen,
	const uint8_t *PubKey, int ExternalMu);

/*************************************************************************************************/
/**
 * @brief	This function verifies a HashML-DSA-87 signature over a pre-hash digest, forming
 *		the domain-separation prefix internally from the context and the hash algorithm.
 *
 * @param	Sig		Pointer to the signature to be verified.
 * @param	SigLen		Length of the signature in bytes.
 * @param	PreHash		Pointer to the pre-hash digest of the message.
 * @param	PreHashLen	Length of the pre-hash digest in bytes.
 * @param	Ctx		Pointer to the context string, may be NULL when CtxLen is zero.
 * @param	CtxLen		Length of the context string in bytes, 0 to 255.
 * @param	PubKey		Pointer to the public key.
 * @param	HashAlg		Pre-hash algorithm, one of MLD_PREHASH_*.
 *
 * @return
 *	- 0 if the signature is valid.
 *	- Non-zero mldsa-native error code if the signature is invalid or on failure.
 *
 *************************************************************************************************/
int PQCP_MLDSA_NATIVE_MLDSA87_C_verify_pre_hash_internal(const uint8_t *Sig, size_t SigLen,
	const uint8_t *PreHash, size_t PreHashLen, const uint8_t *Ctx, size_t CtxLen,
	const uint8_t *PubKey, int HashAlg);

/*************************************************************************************************/
/**
 * @brief	This function formats the ML-DSA-87 domain-separation prefix for the given context
 *		and pre-hash digest, for use with the signature and verify entry points.
 *
 * @param	Prefix		Pointer to the buffer which holds the formatted prefix. It must be
 *				at least MLD_DOMAIN_SEPARATION_MAX_BYTES long.
 * @param	PreHash		Pointer to the pre-hash digest, NULL for pure ML-DSA.
 * @param	PreHashLen	Length of the pre-hash digest in bytes, 0 for pure ML-DSA.
 * @param	Ctx		Pointer to the context string, may be NULL when CtxLen is zero.
 * @param	CtxLen		Length of the context string in bytes, 0 to 255.
 * @param	HashAlg		Pre-hash algorithm, one of MLD_PREHASH_*, MLD_PREHASH_NONE for
 *				pure ML-DSA.
 *
 * @return
 *	- Length of the formatted prefix in bytes.
 *
 *************************************************************************************************/
size_t PQCP_MLDSA_NATIVE_MLDSA87_C_prepare_domain_separation_prefix(uint8_t *Prefix,
	const uint8_t *PreHash, size_t PreHashLen, const uint8_t *Ctx, size_t CtxLen, int HashAlg);

#ifdef __cplusplus
}
#endif

#endif /* OQS_MLDSA_INTERNAL_H_ */
