//
// ECDSA compact code for RSA5X
// API definitions
//
#ifndef ECDSA_H
#define ECSDA_H

//#define NO_COMPACT_ECDSA_SUPPORT // If defined, the compact ECDSA code is disabled

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
//    binary_curve and prime_curve in the SDK. The field naems are
//    obfuscated, as this API is intended for an end-user that does
//    not necessarily have the rights to the
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
} EcdsaCrvInfo;

#define ELLIPTIC_SUCCESS					(0x0)
// Validate Public Key error codes
#define ELLIPTIC_KEY_ZERO					(0x1)
#define ELLIPTIC_KEY_WRONG_ORDER				(0x2)
#define ELLIPTIC_KEY_NOT_ON_CRV				(0x3)

// Verify Sign error codes
#define ELLIPTIC_BAD_SIGN					(0x1)
#define ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN		(0x2)
#define ELLIPTIC_VER_SIGN_R_ZERO				(0x3)
#define ELLIPTIC_VER_SIGN_S_ZERO				(0x4)
#define ELLIPTIC_VER_SIGN_R_ORDER_ERROR			(0x5)
#define ELLIPTIC_VER_SIGN_S_ORDER_ERROR			(0x6)

// Generate sign error codes
#define ELLIPTIC_GEN_SIGN_BAD_R				(0x1)
#define ELLIPTIC_GEN_SIGN_BAD_S				(0x2)
#define ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN		(0x3)
 


externC int Ecdsa_ValidateKey(EcdsaCrvInfo* CrvInfo, EcdsaKey* Key);

externC int Ecdsa_VerifySign(EcdsaCrvInfo* CrvInfo, u8* Hash, u32 HashLen,
	EcdsaKey* Key, EcdsaSign* Sign);

externC int Ecdsa_GenerateSign(EcdsaCrvInfo* CrvInfo, u8* Hash, u32 HashLen,
	const u8* D, const u8* K, EcdsaSign* Sign);

externC int Ecdsa_GeneratePublicKey(EcdsaCrvInfo* CrvInfo, const u8* D, EcdsaKey* Key);
#endif
