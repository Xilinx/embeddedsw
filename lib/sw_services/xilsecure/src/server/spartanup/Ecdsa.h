/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**<
* ECDSA compact code for RSA5X
* API definitions
*/
#ifndef ECDSA_H
#define ECDSA_H

//#define NO_COMPACT_ECDSA_SUPPORT // If defined, the compact ECDSA code is disabled


/**< For compilation in C++ */
#ifdef __cplusplus
#define externC extern "C"
#else
#define externC extern
#endif

/*
#ifndef u8
#define u8  unsigned char
#define u16 unsigned short
#define u32 unsigned int
#endif
*/

#include "xil_types.h"

typedef struct {
	u8* Qx;		/**< Public key curve point x */
	u8* Qy;		/**< Public key curve point y */
} EcdsaKey;

typedef struct {
	u8* r;		/**< The signature component r */
	u8* s;		/**< The signature component s */
} EcdsaSign;

typedef enum {
	ECDSA_INVALID_LOW = 0,	/**< Invalid value (min) */
	ECDSA_NIST_P192,		/**< NIST P-192 curve */
	ECDSA_NIST_P224,		/**< NIST P-224 curve */
	ECDSA_NIST_P256,		/**< NIST P-256 curve */
	ECDSA_NIST_P384,		/**< NIST P-384 curve */
	ECDSA_NIST_P521,		/**< NIST P-521 curve */
	ECDSA_NIST_K163,		/**< NIST K-163 curve */
	ECDSA_NIST_B163,		/**< NIST B-163 curve */
	ECDSA_NIST_K233,		/**< NIST K-233 curve */
	ECDSA_NIST_B233,		/**< NIST B-233 curve */
	ECDSA_NIST_K283,		/**< NIST K-283 curve */
	ECDSA_NIST_B283,		/**< NIST B-283 curve */
	ECDSA_NIST_K409,		/**< NIST K-409 curve */
	ECDSA_NIST_B409,		/**< NIST B-409 curve */
	ECDSA_NIST_K571,		/**< NIST K-571 curve */
	ECDSA_NIST_B571,		/**< NIST B-571 curve */
	ECDSA_SECT_193R1,		/**< SECT 193R1 curve */
	ECDSA_BRAINPOOL_P160 = 18,	/**< Brainpool P-160 curve */
	ECDSA_BRAINPOOL_P192,		/**< Brainpool P-192 curve */
	ECDSA_BRAINPOOL_P224,		/**< Brainpool P-224 curve */
	ECDSA_BRAINPOOL_P256,		/**< Brainpool P-256 curve */
	ECDSA_BRAINPOOL_P320,		/**< Brainpool P-320 curve */
	ECDSA_BRAINPOOL_P384,		/**< Brainpool P-384 curve */
	ECDSA_BRAINPOOL_P512,		/**< Brainpool P-512 curve */
	ECDSA_INVALID_HIGH			/**< Invalid value (max) */
} EcdsaCrvTyp;

typedef enum {
	ECDSA_PRIME = 0,	/**< Prime curve */
	ECDSA_BINARY = 1,	/**< Binary curve */
} EcdsaCrvClass;

//
// The structure below matches the more private structures of
//    binary_curve and prime_curve in the SDK. The The field names are
//    obfuscated, as this API is intended for an end-user that does
//    not necessarily have the rights to the
//
typedef struct {
	EcdsaCrvTyp CrvType;	/**< Curve type */
	u16 Bits;				/**< Curve bits */
	EcdsaCrvClass Class;	/**< Curve class - Prime or Binary */
	const u8* const d0;		/**< Obfuscated field 0 */
	const u8* const d1;		/**< Obfuscated field 1 */
	const u8* const d2;		/**< Obfuscated field 2 */
	const u8* const d3;		/**< Obfuscated field 3 */
	const u8* const d4;		/**< Obfuscated field 4 */
	const u8* const d5;		/**< Obfuscated field 5 */
	const u8* const d6;		/**< Obfuscated field 6 */
	const u8* const d7;		/**< Obfuscated field 7 */
	const u8* const d8;		/**< Obfuscated field 8 */
	const u8* const d9;		/**< Obfuscated field 9 */
	const u8* const d10;	/**< Obfuscated field 10 */
	const u8* const d11;	/**< Obfuscated field 11 */
	const u8* const d12;	/**< Obfuscated field 12 */
	const u8* const d13;	/**< Obfuscated field 13 */
} EcdsaCrvInfo;

#define ELLIPTIC_SUCCESS					(0x0)
						/**< Return value in case of success */

/**
 * @name Elliptic validate key error codes
 * @{
 */
/**< Validate Public Key error codes */
#define ELLIPTIC_KEY_ZERO					(0x1) /**< Key is zero */
#define ELLIPTIC_KEY_WRONG_ORDER				(0x2) /**< Key has wrong order */
#define ELLIPTIC_KEY_NOT_ON_CRV				(0x3) /**< Key is not on the curve */
/** @} */

/**
 * @name Elliptic verify signature error codes
 * @{
 */
/**< Verify Sign error codes */
#define ELLIPTIC_BAD_SIGN					(0x1) /**< Signature verification failed */
#define ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN		(0x2) /**< Incorrect hash length for sign verification */
#define ELLIPTIC_VER_SIGN_R_ZERO				(0x3) /**< Signature component r is zero */
#define ELLIPTIC_VER_SIGN_S_ZERO				(0x4) /**< Signature component s is zero */
#define ELLIPTIC_VER_SIGN_R_ORDER_ERROR			(0x5) /**< r is greater than curve order */
#define ELLIPTIC_VER_SIGN_S_ORDER_ERROR			(0x6) /**< s is greater than curve order */
/** @} */

/**
 * @name Elliptic generate signature error codes
 * @{
 */
/**< Generate sign error codes */
#define ELLIPTIC_GEN_SIGN_BAD_R				(0x1) /**< Generated r component is invalid */
#define ELLIPTIC_GEN_SIGN_BAD_S				(0x2) /**< Generated s component is invalid */
#define ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN		(0x3) /**< Hash length is incorrect */
/** @} */

/**
 * @name IPCores API declarations
 * @{
 */

/******************************************************************************/
/**
 * @brief	This function validates the ECDSA public key.
 *
 * @param	CrvInfo - Pointer to EcdsaCrvInfo structure containing curve parameters
 * @param	Key - Pointer to EcdsaKey structure containing public key (Qx, Qy)
 *
 * @return
 *		- ELLIPTIC_SUCCESS - On successful key validation
 *		- ELLIPTIC_KEY_ZERO - If key is zero
 *		- ELLIPTIC_KEY_WRONG_ORDER - If key has wrong order
 *		- ELLIPTIC_KEY_NOT_ON_CRV - If key is not on the curve
 *
 ******************************************************************************/
externC int Ecdsa_ValidateKey(EcdsaCrvInfo* CrvInfo, EcdsaKey* Key);

/******************************************************************************/
/**
 * @brief	This function verifies the ECDSA signature.
 *
 * @param	CrvInfo - Pointer to EcdsaCrvInfo structure containing curve parameters
 * @param	Hash - Pointer to hash of the message to verify
 * @param	HashLen - Length of the hash in bytes
 * @param	Key - Pointer to EcdsaKey structure containing public key (Qx, Qy)
 * @param	Sign - Pointer to EcdsaSign structure containing signature (r, s)
 *
 * @return
 *		- ELLIPTIC_SUCCESS - On successful signature verification
 *		- ELLIPTIC_BAD_SIGN - If signature verification fails
 *		- ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN - If hash length is incorrect
 *		- ELLIPTIC_VER_SIGN_R_ZERO - If signature component r is zero
 *		- ELLIPTIC_VER_SIGN_S_ZERO - If signature component s is zero
 *		- ELLIPTIC_VER_SIGN_R_ORDER_ERROR - If r is greater than curve order
 *		- ELLIPTIC_VER_SIGN_S_ORDER_ERROR - If s is greater than curve order
 *
 ******************************************************************************/
externC int Ecdsa_VerifySign(EcdsaCrvInfo* CrvInfo, u8* Hash, u32 HashLen,
	EcdsaKey* Key, EcdsaSign* Sign);

/******************************************************************************/
/**
 * @brief	This function generates ECDSA signature.
 *
 * @param	CrvInfo - Pointer to EcdsaCrvInfo structure containing curve parameters
 * @param	Hash - Pointer to hash of the message to sign
 * @param	HashLen - Length of the hash in bytes
 * @param	D - Pointer to private key
 * @param	K - Pointer to ephemeral key (random nonce)
 * @param	Sign - Pointer to EcdsaSign structure to store generated signature (r, s)
 *
 * @return
 *		- ELLIPTIC_SUCCESS - On successful signature generation
 *		- ELLIPTIC_GEN_SIGN_BAD_R - If generated r component is invalid
 *		- ELLIPTIC_GEN_SIGN_BAD_S - If generated s component is invalid
 *		- ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN - If hash length is incorrect
 *
 ******************************************************************************/
externC int Ecdsa_GenerateSign(EcdsaCrvInfo* CrvInfo, u8* Hash, u32 HashLen,
	const u8* D, const u8* K, EcdsaSign* Sign);

/******************************************************************************/
/**
 * @brief	This function generates ECDSA public key from private key.
 *
 * @param	CrvInfo - Pointer to EcdsaCrvInfo structure containing curve parameters
 * @param	D - Pointer to private key
 * @param	Key - Pointer to EcdsaKey structure to store generated public key (Qx, Qy)
 *
 * @return
 *		- ELLIPTIC_SUCCESS - On successful public key generation
 *
 ******************************************************************************/
externC int Ecdsa_GeneratePublicKey(EcdsaCrvInfo* CrvInfo, const u8* D, EcdsaKey* Key);
/** @} */
#endif
