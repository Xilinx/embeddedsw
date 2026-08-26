/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file oqs_mlkem_internal.h
 *
 * This header is the ASU firmware CONTRACT for the lower-level (native) ML-KEM entry points
 * exposed by the liboqs "mlkem-native" backend. liboqs's generic OQS_KEM API reaches these same
 * functions indirectly, through a descriptor object that OQS_KEM_new() looks up by algorithm name
 * and allocates on the heap. The firmware calls the native symbols directly instead, so that the
 * ML-KEM path needs no allocator, and so that the name-keyed registry, which references every
 * algorithm liboqs was built with, does not defeat --gc-sections and anchor unused backends.
 *
 * These prototypes MUST match the symbols exported by liboqs.a exactly. The exact link-level
 * names were verified via `nm liboqs.a` and follow the pattern:
 *     PQCP_MLKEM_NATIVE_MLKEM{512|768|1024}_C_<function>
 *
 * The build of liboqs used here does NOT define MLK_CONFIG_CONTEXT_PARAMETER, therefore the
 * functions have NO trailing application-context argument. Defining it would add that argument
 * without changing a single symbol name, so the mismatch would link silently. build_liboqs.sh
 * checks for exactly that before it builds: it compiles these declarations alongside
 * mlkem-native's own, where a disagreement is a compile error. Keep the two in step, and see
 * README.liboqs.md section 2 if that check fires.
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
#ifndef OQS_MLKEM_INTERNAL_H_
#define OQS_MLKEM_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include <stdint.h>

/************************************ Constant Definitions ***************************************/
/* Byte lengths shared across all ML-KEM parameter sets (FIPS 203). */
#define MLKEM_KEYPAIR_SEEDBYTES	(64U) /**< Key generation seed length (d || z) in bytes */
#define MLKEM_ENCAPS_SEEDBYTES	(32U) /**< Encapsulation message (m) length in bytes */
#define MLKEM_SSBYTES		(32U) /**< Shared secret length in bytes */

/************************************ Function Prototypes ****************************************/
/*
 * The native entry points, one set per parameter set. Buffer lengths that are not stated with the
 * argument are the parameter-set lengths from FIPS 203.
 *
 * Only the derandomized entry points are declared. The backend's non-derandomized keypair() and
 * enc() draw their randomness through OQS_randombytes, which this bare-metal drop leaves as a
 * do-nothing stub, so every caller must draw its own seed from the ASU TRNG and pass it in.
 */

/* ------------------------------- ML-KEM-512 ------------------------------ */
/*************************************************************************************************/
/**
 * @brief	This function generates an ML-KEM-512 key pair from the given seed.
 *
 * @param	PubKey		Pointer to the buffer which holds the generated public
 *				(encapsulation) key.
 * @param	SecKey		Pointer to the buffer which holds the generated secret
 *				(decapsulation) key.
 * @param	Seed		Pointer to the MLKEM_KEYPAIR_SEEDBYTES seed (d || z).
 *
 * @return
 *	- 0 on success.
 *	- Non-zero mlkem-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLKEM_NATIVE_MLKEM512_C_keypair_derand(uint8_t *PubKey, uint8_t *SecKey,
	const uint8_t *Seed);

/*************************************************************************************************/
/**
 * @brief	This function performs ML-KEM-512 encapsulation with the given message, producing a
 *		ciphertext and the shared secret.
 *
 * @param	CipherText	Pointer to the buffer which holds the generated ciphertext.
 * @param	SharedSecret	Pointer to the buffer which holds the generated shared secret. It
 *				must be at least MLKEM_SSBYTES long.
 * @param	PubKey		Pointer to the public (encapsulation) key.
 * @param	Seed		Pointer to the MLKEM_ENCAPS_SEEDBYTES message (m).
 *
 * @return
 *	- 0 on success.
 *	- Non-zero mlkem-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLKEM_NATIVE_MLKEM512_C_enc_derand(uint8_t *CipherText, uint8_t *SharedSecret,
	const uint8_t *PubKey, const uint8_t *Seed);

/*************************************************************************************************/
/**
 * @brief	This function performs ML-KEM-512 decapsulation, recovering the shared secret from
 *		the given ciphertext.
 *
 * @param	SharedSecret	Pointer to the buffer which holds the recovered shared secret. It
 *				must be at least MLKEM_SSBYTES long.
 * @param	CipherText	Pointer to the ciphertext.
 * @param	SecKey		Pointer to the secret (decapsulation) key.
 *
 * @return
 *	- 0 on success. A ciphertext that does not match the key yields the implicit rejection
 *	  shared secret rather than an error, as specified by FIPS 203.
 *	- Non-zero mlkem-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLKEM_NATIVE_MLKEM512_C_dec(uint8_t *SharedSecret, const uint8_t *CipherText,
	const uint8_t *SecKey);

/* ------------------------------- ML-KEM-768 ------------------------------ */
/*************************************************************************************************/
/**
 * @brief	This function generates an ML-KEM-768 key pair from the given seed.
 *
 * @param	PubKey		Pointer to the buffer which holds the generated public
 *				(encapsulation) key.
 * @param	SecKey		Pointer to the buffer which holds the generated secret
 *				(decapsulation) key.
 * @param	Seed		Pointer to the MLKEM_KEYPAIR_SEEDBYTES seed (d || z).
 *
 * @return
 *	- 0 on success.
 *	- Non-zero mlkem-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLKEM_NATIVE_MLKEM768_C_keypair_derand(uint8_t *PubKey, uint8_t *SecKey,
	const uint8_t *Seed);

/*************************************************************************************************/
/**
 * @brief	This function performs ML-KEM-768 encapsulation with the given message, producing a
 *		ciphertext and the shared secret.
 *
 * @param	CipherText	Pointer to the buffer which holds the generated ciphertext.
 * @param	SharedSecret	Pointer to the buffer which holds the generated shared secret. It
 *				must be at least MLKEM_SSBYTES long.
 * @param	PubKey		Pointer to the public (encapsulation) key.
 * @param	Seed		Pointer to the MLKEM_ENCAPS_SEEDBYTES message (m).
 *
 * @return
 *	- 0 on success.
 *	- Non-zero mlkem-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLKEM_NATIVE_MLKEM768_C_enc_derand(uint8_t *CipherText, uint8_t *SharedSecret,
	const uint8_t *PubKey, const uint8_t *Seed);

/*************************************************************************************************/
/**
 * @brief	This function performs ML-KEM-768 decapsulation, recovering the shared secret from
 *		the given ciphertext.
 *
 * @param	SharedSecret	Pointer to the buffer which holds the recovered shared secret. It
 *				must be at least MLKEM_SSBYTES long.
 * @param	CipherText	Pointer to the ciphertext.
 * @param	SecKey		Pointer to the secret (decapsulation) key.
 *
 * @return
 *	- 0 on success. A ciphertext that does not match the key yields the implicit rejection
 *	  shared secret rather than an error, as specified by FIPS 203.
 *	- Non-zero mlkem-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLKEM_NATIVE_MLKEM768_C_dec(uint8_t *SharedSecret, const uint8_t *CipherText,
	const uint8_t *SecKey);

/* ------------------------------ ML-KEM-1024 ------------------------------ */
/*************************************************************************************************/
/**
 * @brief	This function generates an ML-KEM-1024 key pair from the given seed.
 *
 * @param	PubKey		Pointer to the buffer which holds the generated public
 *				(encapsulation) key.
 * @param	SecKey		Pointer to the buffer which holds the generated secret
 *				(decapsulation) key.
 * @param	Seed		Pointer to the MLKEM_KEYPAIR_SEEDBYTES seed (d || z).
 *
 * @return
 *	- 0 on success.
 *	- Non-zero mlkem-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLKEM_NATIVE_MLKEM1024_C_keypair_derand(uint8_t *PubKey, uint8_t *SecKey,
	const uint8_t *Seed);

/*************************************************************************************************/
/**
 * @brief	This function performs ML-KEM-1024 encapsulation with the given message, producing
 *		a ciphertext and the shared secret.
 *
 * @param	CipherText	Pointer to the buffer which holds the generated ciphertext.
 * @param	SharedSecret	Pointer to the buffer which holds the generated shared secret. It
 *				must be at least MLKEM_SSBYTES long.
 * @param	PubKey		Pointer to the public (encapsulation) key.
 * @param	Seed		Pointer to the MLKEM_ENCAPS_SEEDBYTES message (m).
 *
 * @return
 *	- 0 on success.
 *	- Non-zero mlkem-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLKEM_NATIVE_MLKEM1024_C_enc_derand(uint8_t *CipherText, uint8_t *SharedSecret,
	const uint8_t *PubKey, const uint8_t *Seed);

/*************************************************************************************************/
/**
 * @brief	This function performs ML-KEM-1024 decapsulation, recovering the shared secret from
 *		the given ciphertext.
 *
 * @param	SharedSecret	Pointer to the buffer which holds the recovered shared secret. It
 *				must be at least MLKEM_SSBYTES long.
 * @param	CipherText	Pointer to the ciphertext.
 * @param	SecKey		Pointer to the secret (decapsulation) key.
 *
 * @return
 *	- 0 on success. A ciphertext that does not match the key yields the implicit rejection
 *	  shared secret rather than an error, as specified by FIPS 203.
 *	- Non-zero mlkem-native error code on failure.
 *
 *************************************************************************************************/
int PQCP_MLKEM_NATIVE_MLKEM1024_C_dec(uint8_t *SharedSecret, const uint8_t *CipherText,
	const uint8_t *SecKey);

#ifdef __cplusplus
}
#endif

#endif /* OQS_MLKEM_INTERNAL_H_ */
