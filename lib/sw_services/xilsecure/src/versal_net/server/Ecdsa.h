//
// ECDSA compact code for RSA5X, "quiet" version
// API definitions
// (C) IP Cores, Inc. 2021-2022
//
// Version	1.8  Initial release (version number synchronized with regular Ecdsa)
//			1.81 MISRA C changes applied, Xilinx-only changes
//			1.82 Xilinx updates applied
//			1.84 No changes, revision bumped to match the .c file
//			1.86 Synchronized with Xilinx changes (except extern changed to externC
//				 for Ecdsa_P384)
//
#ifndef RSA5X_H
#define RSA5X_H

#include <stdint.h>
#include <string.h>
#include "xsecure_trng.h"

#define BYTE_SIZE_IN_BITS	8U
#define TRNG_BURST_SIZE		128U
#define RSA5X_GET_RANDOM(bytes, buf, bufsize) XTrng_Generate(bytes, buf, bufsize)

#ifdef __cplusplus
#define externC extern "C"
#else
#define externC extern
#endif

typedef struct {
	u8* Qx;
	u8* Qy;
} EcdsaKey;

typedef struct {
	u8* r;
	u8* s;
} EcdsaSign;

typedef enum {
	ECDSA_INVALID_LOW = 0,
	ECDSA_NIST_P192,
	ECDSA_NIST_P224,
	ECDSA_NIST_P256,
	ECDSA_NIST_P384,
	ECDSA_NIST_P521,
	ECDSA_NIST_K163,
	ECDSA_NIST_B163,
	ECDSA_NIST_K233,
	ECDSA_NIST_B233,
	ECDSA_NIST_K283,
	ECDSA_NIST_B283,
	ECDSA_NIST_K409,
	ECDSA_NIST_B409,
	ECDSA_NIST_K571,
	ECDSA_NIST_B571,
	ECDSA_SECT_193R1,
	ECDSA_SM2_FP256,
	ECDSA_BRAINPOOL_P160,
	ECDSA_BRAINPOOL_P192,
	ECDSA_BRAINPOOL_P224,
	ECDSA_BRAINPOOL_P256,
	ECDSA_BRAINPOOL_P320,
	ECDSA_BRAINPOOL_P384,
	ECDSA_BRAINPOOL_P512,
	ECDSA_INVALID_HIGH
} EcdsaCrvTyp;

typedef enum {
	ECDSA_PRIME = 0,
	ECDSA_BINARY = 1,
} EcdsaCrvClass;

//
// The structure below matches the more private structures of
//    binary_curve and prime_curve in the SDK. The field names are
//    obfuscated, as this API is intended for an end-user that does
//    not necessarily have the rights to the source code of the SDK
//
typedef struct {
	EcdsaCrvTyp CrvType;
	u16 Bits; EcdsaCrvClass Class;
	const u8* const d0;
	const u8* const d1;
	const u8* const d2;
	const u8* const d3;
	const u8* const d4;
	const u8* const d5;
	const u8* const d6;
	const u8* const d7;
	const u8* const d8;
	const u8* const d9;
	const u8* const d10;
	const u8* const d11;
	const u8* const d12;
	const u8* const d13;
	const u8* const d14;
	const u8* const d15;
} EcdsaCrvInfo;

externC const EcdsaCrvInfo Ecdsa_P384;

//
//	The following restrictions define the memory used for the ECC variables
//  The design sometimes requires two extra digits
//
#define MAX_CURVE_BITS  571
#define ECC_DIGIT_BITS	(RSA5X_DSIZE*8U)
#define ECC_MAX_BITS	(((MAX_CURVE_BITS+ECC_DIGIT_BITS-1)/ECC_DIGIT_BITS+2)*ECC_DIGIT_BITS)
#define ECC_P384_SIZE_IN_BYTES		48U

#define ELLIPTIC_SUCCESS					(0x0)
						/**< Return value in case of success */

/**
 * @name Elliptic validate key error codes
 * @{
 */
/**< Validate Public Key error codes */
#define ELLIPTIC_KEY_ZERO					(0x1)
#define ELLIPTIC_KEY_WRONG_ORDER				(0x2)
#define ELLIPTIC_KEY_NOT_ON_CRV				(0x3)
/** @} */

/**
 * @name Elliptic verify signature error codes
 * @{
 */
/**< Verify Sign error codes */
#define ELLIPTIC_BAD_SIGN					(0x1)
#define ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN		(0x2)
#define ELLIPTIC_VER_SIGN_R_ZERO				(0x3)
#define ELLIPTIC_VER_SIGN_S_ZERO				(0x4)
#define ELLIPTIC_VER_SIGN_R_ORDER_ERROR			(0x5)
#define ELLIPTIC_VER_SIGN_S_ORDER_ERROR			(0x6)
/** @} */

/**
 * @name Elliptic generate signature error codes
 * @{
 */
/**< Generate sign error codes */
#define ELLIPTIC_GEN_SIGN_BAD_R				(0x1)
#define ELLIPTIC_GEN_SIGN_BAD_S				(0x2)
#define ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN		(0x3)
/** @} */


//
// Validate public key
//
// Returns: 0 if validated successfully, error otherwise
// Input:	CrvInfo		- pointer to the curve data structure
//			Key			- public key
//
externC s32 Ecdsa_ValidateKey(const EcdsaCrvInfo* CrvInfo, const EcdsaKey* Key);

//
// Verify the signature
//
// Returns: 0 if validated successfully, error otherwise
// Input:	CrvInfo		- pointer to the curve data structure
//			z		- pointer to hash of the signed data
//			zlen		- length of the hash (bits)
//			Key			- public key
//			Sign		- signature
//
externC s32 Ecdsa_VerifySign(const EcdsaCrvInfo* CrvInfo, u8* z, u32 zlen,
	const EcdsaKey* Key, const EcdsaSign* Sign);

//
// Generate the signature
//
// Returns: 0 if OK, error otherwise
// Input:	CrvInfo		- pointer to the curve data structure
//			z		- pointer to hash of the data to be signed
//			zlen		- length of the hash (bits)
//			D			- secret key
//			K			- nonce
// Output:  Sign		- signature
//
externC s32 Ecdsa_GenerateSign(const EcdsaCrvInfo* CrvInfo, const u8* z, u32 zlen,
	const u8* D, const u8* K, const EcdsaSign* Sign);

//
// Generate the public key
//
// Returns: 0 if OK, error otherwise
// Input:	CrvInfo		- pointer to the curve data structure
//			D			- secret key
// Output:  Key			- public key
//
externC s32 Ecdsa_GeneratePublicKey(const EcdsaCrvInfo* CrvInfo, const u8* D, const EcdsaKey* Key);

//
// SDK assert function
//
externC void sdk_assert(int cond);

//
// Performs mod operation result = a MOD ecc order of the curve
//
// Returns: 0 if OK, error otherwise
// Input:	CrvInfo		- pointer to the curve data structure
//		a		- input number
//		alen		- length of input number
//		out		- memory pointer where mod result is to be stored
//		outlen		- size of out buffer
// Output:  Key			- public key
//
externC s32 Ecdsa_ModEccOrder(const EcdsaCrvInfo* CrvInfo, const u8* In, u8* Out);
//
// Clearing the ECC RAMs
//
void Ecdsa_ClearEccRam(void);

externC s32 XTrng_Generate(u32 RandBytes, u8 *RandBuf, u32 RandBufSize);

#endif