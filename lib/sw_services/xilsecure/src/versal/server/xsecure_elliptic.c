/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_elliptic.c
*
* This file contains the implementation of the interface functions for ECC
* engine.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   rpo 03/31/20  Initial release
* 4.2   rpo 03/31/20  Updated file version to sync with library version
* 4.3   har 08/24/20  Added APIs to generate and verify ECDSA public key and
*                     signature
*                     Added support for ECDSA P521 curve
*       am  09/25/20  Resolved MISRA C violations
*       har 10/12/20  Addressed security review comments
*       har 10/14/20  Replaced ecdsa with elliptic in names of function and
*                     macros
* 4.5   har 01/18/20  Added support for ECDSA P521 KAT
*       kpt 02/14/21  Added redundancy for ECDSA hash length check
*       har 03/22/21  Added volatile keyword to status variables used in
*                     XSECURE_TEMPORAL_CHECK
* 4.6   har 07/14/21  Fixed doxygen warnings
*       gm  07/16/21  Added support for 64-bit address
*       rb  08/11/21  Fix compilation warnings
*       har 09/13/21  Fixed signature verification issue for P521 curve
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_error.h"
#include "xsecure_elliptic.h"
#include "xsecure_ecdsa_rsa_hw.h"
#include "xsecure_utils.h"
#include "xil_util.h"
#include "xsecure_cryptochk.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @name  Value of Public Strength (bits 7:4) in Authentication Certificate
 * @{
 */
/**< Public Strength value for NIST P-384 curve and NIST P-521 curve */
#define XSECURE_ECDSA_KAT_NIST_P384	0U
#define XSECURE_ECDSA_KAT_NIST_P521	2U
/** @} */


#define XSECURE_ECDSA_P521_ALIGN_BYTES	2U
				/**< Size of NIST P-521 curve is 66 bytes. This macro is used
				to make the address word aligned */

/************************** Function Prototypes ******************************/
static EcdsaCrvInfo* XSecure_EllipticGetCrvData(XSecure_EllipticCrvTyp CrvTyp);
static void XSecure_PutData(const u32 Size, u8 *Dst, const u64 SrcAddr);
static void XSecure_GetData(const u32 Size, const u8 *Src, const u64 DstAddr);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * @brief	This function generates Public Key for a given curve type using
 *		private key where both keys located at 64 bit address
 *
 * @param	CrvType - Is a type of elliptic curve
 * @param	DAddr   - Address of static private key
 * @param	KeyAddr - Pointer to public key address
 *
 * @return
 *		- XST_SUCCESS - On success
 * 		- XSECURE_ELLIPTIC_NON_SUPPORTED_CRV - When elliptic Curve is not supported
 * 		- XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 * 		- XSECURE_ELLIPTIC_GEN_KEY_ERR 	 Error in generating Public key
 *
 *****************************************************************************/
int XSecure_EllipticGenerateKey_64Bit(XSecure_EllipticCrvTyp CrvType,
	const u64 DAddr, XSecure_EllipticKeyAddr *KeyAddr)
{
	int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	EcdsaCrvInfo *Crv = NULL;
	u8 PubKey[XSECURE_ECC_P521_SIZE_IN_BYTES +
	XSECURE_ECDSA_P521_ALIGN_BYTES +
	XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	u8 D[XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	EcdsaKey Key;
	u32 Size = 0U;
	u32 OffSet = 0U;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((CrvType != XSECURE_ECC_NIST_P384) &&
			(CrvType != XSECURE_ECC_NIST_P521)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	if (CrvType == XSECURE_ECC_NIST_P384) {
		Size = XSECURE_ECC_P384_SIZE_IN_BYTES;
		OffSet = Size;
	}
	else {
		Size = XSECURE_ECC_P521_SIZE_IN_BYTES;
		OffSet = Size + XSECURE_ECDSA_P521_ALIGN_BYTES;
	}

	/* Store Priv key to local buffer */
	XSecure_PutData(Size, (u8*)D, DAddr);

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + OffSet);

	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);

	Crv = XSecure_EllipticGetCrvData(CrvType);
	if(Crv != NULL) {
		Status = Ecdsa_GeneratePublicKey(Crv, D, (EcdsaKey *)&Key);
		if (Status != XST_SUCCESS) {
			Status = (int)XSECURE_ELLIPTIC_GEN_KEY_ERR;
		} else {
			/* Store key to destination address */
			XSecure_GetData(Size, (u8 *)PubKey, KeyAddr->Qx);
			XSecure_GetData(Size,
					(u8 *)(PubKey + OffSet),
					KeyAddr->Qy);
		}
	}

END:
	XSecure_SetReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function generates Public Key for a given curve type
 *
 * @param	CrvType - Is a type of elliptic curve
 * @param	D       - Pointer to static private key
 * @param	Key     - Pointer to public key
 *
 * @return
 *		- XST_SUCCESS - On success
 * 		- XSECURE_ELLIPTIC_NON_SUPPORTED_CRV - When elliptic Curve is not supported
 * 		- XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 * 		- XSECURE_ELLIPTIC_GEN_KEY_ERR 	 Error in generating Public key
 *
 *****************************************************************************/
int XSecure_EllipticGenerateKey(XSecure_EllipticCrvTyp CrvType, const u8* D,
	XSecure_EllipticKey *Key)
{
	XSecure_EllipticKeyAddr KeyAddr = { (u64)(UINTPTR)Key->Qx,
		(u64)(UINTPTR)Key->Qy};

	return XSecure_EllipticGenerateKey_64Bit(CrvType, (u64)(UINTPTR)D,
			(XSecure_EllipticKeyAddr *) &KeyAddr);
}

/*****************************************************************************/
/**
 * @brief	This function generates signature for a given hash and curve
 *		type where data is located at 64-bit address.
 *
 * @param	CrvType  -Type of elliptic curve
 * @param	HashInfo - Pointer to Hash Data i.e. Hash Address and length
 * @param	DAddr    - Address of the static private key
 * @param	KAddr    - Ephemeral private key
 * @param	SignAddr - Pointer to signature address
 *
 * @return
 *	- XST_SUCCESS - On success
 * 	- XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 * 	- XSECURE_ELLIPTIC_GEN_SIGN_BAD_RAND_NUM - When Bad random number used
 *						for sign generation
 * 	- XSECURE_ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN - Incorrect hash length for sign
 *							generation
 *	- XST_FAILURE - On failure
 *
 * @note
 * K, the ephemeral private key, shall be an unpredictable (cryptographically
 * secure) random number unique for each signature
 * Note that reuse or external predictability of this number generally breaks
 * the security of ECDSA
 *
 *****************************************************************************/
int XSecure_EllipticGenerateSignature_64Bit(XSecure_EllipticCrvTyp CrvType,
	XSecure_EllipticHashData *HashInfo, const u64 DAddr,
	const u64 KAddr, XSecure_EllipticSignAddr *SignAddr)
{
	int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	int StatusTemp = XST_FAILURE;
	volatile int GenStatus = XST_FAILURE;
	EcdsaCrvInfo *Crv = NULL;
	u8 PaddedHash[XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	u8 D[XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	u8 K[XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	u8 Signature[XSECURE_ECC_P521_SIZE_IN_BYTES +
	XSECURE_ECDSA_P521_ALIGN_BYTES +
	XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};

	EcdsaSign Sign = {0};
	volatile u32 HashLenTmp = 0xFFFFFFFFU;
	u32 OffSet = 0U;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((CrvType != XSECURE_ECC_NIST_P384) &&
			(CrvType != XSECURE_ECC_NIST_P521)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	/* Store Hash,D,K to local buffers */
	XSecure_PutData(HashInfo->Len, (u8 *)PaddedHash, HashInfo->Addr);
	XSecure_PutData(HashInfo->Len, (u8 *)D, DAddr);
	XSecure_PutData(HashInfo->Len, (u8 *)K, KAddr);

	if (CrvType == XSECURE_ECC_NIST_P384) {
		OffSet = HashInfo->Len;
	}
	else {
		OffSet = HashInfo->Len + XSECURE_ECDSA_P521_ALIGN_BYTES;
	}

	Sign.r = (u8 *)Signature;
	Sign.s = (u8 *)(Signature + OffSet);

	HashLenTmp = HashInfo->Len;
	if ((HashInfo->Len > XSECURE_ECC_P521_SIZE_IN_BYTES) ||
		(HashLenTmp > XSECURE_ECC_P521_SIZE_IN_BYTES)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);

	Crv = XSecure_EllipticGetCrvData(CrvType);
	if(Crv != NULL) {
		XSECURE_TEMPORAL_CHECK(SIG_ERR, GenStatus, Ecdsa_GenerateSign,
			Crv, PaddedHash, Crv->Bits, D, K, (EcdsaSign *)&Sign);

SIG_ERR:
		if ((GenStatus == ELLIPTIC_GEN_SIGN_BAD_R) ||
				(GenStatus == ELLIPTIC_GEN_SIGN_BAD_S)) {
			Status = (int)XSECURE_ELLIPTIC_GEN_SIGN_BAD_RAND_NUM;
		}
		else if (GenStatus == ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN) {
			Status = (int)XSECURE_ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN;
		}
		else if (GenStatus != ELLIPTIC_SUCCESS) {
			Status = XST_FAILURE;
		}
		else {
			/* Store Sign to destination address */
			XSecure_GetData(HashInfo->Len, (u8 *)Signature,
					SignAddr->SignR);
			XSecure_GetData(HashInfo->Len,
					(u8 *)(Signature + OffSet),
					SignAddr->SignS);
			Status = XST_SUCCESS;
		}
	}

END:
	/* Zeroize local key copy */
	StatusTemp = Xil_SecureZeroize((u8*)D, XSECURE_ECC_P521_SIZE_IN_BYTES);
	if (Status == XST_SUCCESS) {
		Status |= StatusTemp;
	}
	StatusTemp = XST_FAILURE;
	StatusTemp = Xil_SecureZeroize((u8*)K, XSECURE_ECC_P521_SIZE_IN_BYTES);
	if (Status == XST_SUCCESS) {
		Status |= StatusTemp;
	}
	XSecure_SetReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function generates signature for a given hash and curve type
 *
 * @param	CrvType  -Type of elliptic curve
 * @param	Hash    - Pointer to the hash for which sign has to be generated
 * @param	HashLen - Length of the hash in bytes
 * @param	D       - Pointer to the static private key
 * @param	K       - Ephemeral private key
 * @param	Sign    - Pointer to the signature
 *
 * @return
 *	- XST_SUCCESS - On success
 * 	- XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 * 	- XSECURE_ELLIPTIC_GEN_SIGN_BAD_RAND_NUM - When Bad random number used
 *						for sign generation
 * 	- XSECURE_ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN - Incorrect hash length for sign
 *							generation
 *	- XST_FAILURE - On failure
 *
 * @note
 * K, the ephemeral private key, shall be an unpredictable (cryptographically
 * secure) random number unique for each signature
 * Note that reuse or external predictability of this number generally breaks
 * the security of ECDSA
 *
 *****************************************************************************/
int XSecure_EllipticGenerateSignature(XSecure_EllipticCrvTyp CrvType,
	const u8* Hash, const u32 HashLen, const u8* D,
	const u8* K, XSecure_EllipticSign *Sign)
{
	XSecure_EllipticSignAddr SignAddr =  {(u64)(UINTPTR)Sign->SignR,
		(u64)(UINTPTR)Sign->SignS};

	XSecure_EllipticHashData HashInfo = {(u64)(UINTPTR)Hash, HashLen};

	return XSecure_EllipticGenerateSignature_64Bit(CrvType,
			(XSecure_EllipticHashData *) &HashInfo,
			(u64)(UINTPTR)D,
			(u64)(UINTPTR)K,
			(XSecure_EllipticSignAddr *) &SignAddr);
}

/*****************************************************************************/
/**
 * @brief	This function validates the public key for a given curve type
 *		where key is located at 64-bit address.
 *
 * @param	CrvType - Type of elliptic curve
 * @param	KeyAddr - Pointer to public key address
 *
 * @return
 *		- XST_SUCCESS - On success
 * 		- XSECURE_ELLIPTIC_INVALID_PARAM   - On invalid argument
 * 		- XSECURE_ELLIPTIC_KEY_ZERO        - When Public key is zero
 *		- XSECURE_ELLIPTIC_KEY_WRONG_ORDER - Wrong order of Public key
 * 		- XSECURE_ELLIPTIC_KEY_NOT_ON_CRV  - When Key is not found on the curve
 *		- XST_FAILURE                   - On failure
 *
 *****************************************************************************/
int XSecure_EllipticValidateKey_64Bit(XSecure_EllipticCrvTyp CrvType,
		XSecure_EllipticKeyAddr *KeyAddr)
{
	int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	volatile int ValidateStatus = XST_FAILURE;
	EcdsaCrvInfo *Crv = NULL;
	EcdsaKey Key;
	u8 PubKey[XSECURE_ECC_P521_SIZE_IN_BYTES +
	XSECURE_ECDSA_P521_ALIGN_BYTES +
	XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	u32 Size = 0U;
	u32 OffSet = 0U;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((CrvType != XSECURE_ECC_NIST_P384) &&
		(CrvType != XSECURE_ECC_NIST_P521)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	if (CrvType == XSECURE_ECC_NIST_P384) {
		Size = XSECURE_ECC_P384_SIZE_IN_BYTES;
		OffSet = Size;
	}
	else {
		Size = XSECURE_ECC_P521_SIZE_IN_BYTES;
		OffSet = Size + XSECURE_ECDSA_P521_ALIGN_BYTES;
	}

	/* Store Pub key(Qx,Qy) to local buffer */
	XSecure_PutData(Size, (u8 *)PubKey, KeyAddr->Qx);
	XSecure_PutData(Size, (u8 *)(PubKey + OffSet), KeyAddr->Qy);

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + OffSet);

	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);

	Crv = XSecure_EllipticGetCrvData(CrvType);
	if(Crv != NULL) {
		XSECURE_TEMPORAL_CHECK(KEY_ERR, ValidateStatus, Ecdsa_ValidateKey,
			Crv, (EcdsaKey *)&Key);

KEY_ERR:
		if (ValidateStatus == ELLIPTIC_KEY_ZERO) {
			Status = (int)XSECURE_ELLIPTIC_KEY_ZERO;
		}
		else if (ValidateStatus == ELLIPTIC_KEY_WRONG_ORDER) {
			Status = (int)XSECURE_ELLIPTIC_KEY_WRONG_ORDER;
		}
		else if (ValidateStatus == ELLIPTIC_KEY_NOT_ON_CRV) {
			Status = (int)XSECURE_ELLIPTIC_KEY_NOT_ON_CRV;
		}
		else if (ValidateStatus != ELLIPTIC_SUCCESS) {
			Status = XST_FAILURE;
		}
		else {
			Status = XST_SUCCESS;
		}
	}

END:
	XSecure_SetReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function validates the public key for a given curve type
 *
 * @param	CrvType - Type of elliptic curve
 * @param	Key     - Pointer to the public key
 *
 * @return
 *		- XST_SUCCESS - On success
 * 		- XSECURE_ELLIPTIC_INVALID_PARAM   - On invalid argument
 * 		- XSECURE_ELLIPTIC_KEY_ZERO        - When Public key is zero
 *		- XSECURE_ELLIPTIC_KEY_WRONG_ORDER - Wrong order of Public key
 * 		- XSECURE_ELLIPTIC_KEY_NOT_ON_CRV  - When Key is not found on the curve
 *		- XST_FAILURE                   - On failure
 *
 *****************************************************************************/
int XSecure_EllipticValidateKey(XSecure_EllipticCrvTyp CrvType,
	XSecure_EllipticKey *Key)
{
	XSecure_EllipticKeyAddr KeyAddr = {(u64)(UINTPTR)Key->Qx,
		(u64)(UINTPTR)Key->Qy};

	return XSecure_EllipticValidateKey_64Bit(CrvType,
			(XSecure_EllipticKeyAddr *) &KeyAddr);
}

/*****************************************************************************/
/**
 * @brief	This function verifies the signature for a given hash, key and
 *		curve type where data is located at 64-bit address
 *
 * @param	CrvType - Type of elliptic curve
 * @param	HashInfo - Pointer to Hash Data i.e. Hash Address and length
 * @param	KeyAddr  - Pointer to public key address
 * @param	SignAddr - Pointer to signature address
 *
 * @return
 *	- XST_SUCCESS - On success
 *	- XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *	- XSECURE_ELLIPTIC_BAD_SIGN - When signature provided for verification is bad
 *	- XSECURE_ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN - Incorrect hash length
 *						for sign verification
 *	- XSECURE_ELLIPTIC_VER_SIGN_R_ZERO - R set to zero
 *	- XSECURE_ELLIPTIC_VER_SIGN_S_ZERO - S set to zero
 *	- XSECURE_ELLIPTIC_VER_SIGN_R_ORDER_ERROR - R is not within ECC order
 *	- XSECURE_ELLIPTIC_VER_SIGN_S_ORDER_ERROR - S is not within ECC order
 *	- XST_FAILURE - On failure
 *
 *****************************************************************************/
int XSecure_EllipticVerifySign_64Bit(XSecure_EllipticCrvTyp CrvType,
	XSecure_EllipticHashData *HashInfo, XSecure_EllipticKeyAddr *KeyAddr,
	XSecure_EllipticSignAddr *SignAddr)
{
	int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	volatile int VerifyStatus = XST_FAILURE;
	EcdsaCrvInfo *Crv = NULL;
	u8 PaddedHash[XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	volatile u32 HashLenTmp = 0xFFFFFFFFU;
	u8 PubKey[XSECURE_ECC_P521_SIZE_IN_BYTES +
	XSECURE_ECDSA_P521_ALIGN_BYTES +
	XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	u8 Signature[XSECURE_ECC_P521_SIZE_IN_BYTES +
	XSECURE_ECDSA_P521_ALIGN_BYTES +
	XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	EcdsaKey Key;
	EcdsaSign Sign;
	u32 OffSet = 0U;
	u32 Size = 0U;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((CrvType != XSECURE_ECC_NIST_P384) && (CrvType != XSECURE_ECC_NIST_P521)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	HashLenTmp = HashInfo->Len;
	if ((HashInfo->Len > XSECURE_ECC_P521_SIZE_IN_BYTES) ||
		(HashLenTmp > XSECURE_ECC_P521_SIZE_IN_BYTES)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	/* Store Pub key(Qx,Qy) and Sign(SignR, SignS) to local buffers */
	if (CrvType == XSECURE_ECC_NIST_P521) {
		Size = XSECURE_ECC_P521_SIZE_IN_BYTES;
		OffSet = Size + XSECURE_ECDSA_P521_ALIGN_BYTES;
	} else {
		Size = XSECURE_ECC_P384_SIZE_IN_BYTES;
		OffSet = Size;
	}
	XSecure_PutData(Size, (u8 *)PubKey, KeyAddr->Qx);
	XSecure_PutData(Size, (u8 *)(PubKey + OffSet), KeyAddr->Qy);

	XSecure_PutData(Size, (u8 *)Signature, SignAddr->SignR);
	XSecure_PutData(Size, (u8 *)(Signature + OffSet),
			SignAddr->SignS);


	/* Store Hash to local buffer */
	XSecure_PutData(HashInfo->Len, (u8 *)PaddedHash, HashInfo->Addr);

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + OffSet);

	Sign.r = (u8 *)(UINTPTR)Signature;
	Sign.s = (u8 *)(UINTPTR)(Signature + OffSet);

	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);

	Crv = XSecure_EllipticGetCrvData(CrvType);
	if(Crv != NULL) {
		XSECURE_TEMPORAL_CHECK(SIG_ERR, VerifyStatus, Ecdsa_VerifySign,
			Crv, PaddedHash, Crv->Bits, (EcdsaKey *)&Key, (EcdsaSign *)&Sign);

SIG_ERR:
		if ((int)ELLIPTIC_BAD_SIGN == VerifyStatus) {
			Status = (int)XSECURE_ELLIPTIC_BAD_SIGN;
		}
		else if ((int)ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN == VerifyStatus) {
			Status = (int)XSECURE_ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN;
		}
		else if (ELLIPTIC_VER_SIGN_R_ZERO == VerifyStatus) {
			Status = (int)XSECURE_ELLIPTIC_VER_SIGN_R_ZERO;
		}
		else if (ELLIPTIC_VER_SIGN_S_ZERO == VerifyStatus) {
			Status = (int)XSECURE_ELLIPTIC_VER_SIGN_S_ZERO;
		}
		else if (ELLIPTIC_VER_SIGN_R_ORDER_ERROR == VerifyStatus) {
			Status = (int)XSECURE_ELLIPTIC_VER_SIGN_R_ORDER_ERROR;
		}
		else if (ELLIPTIC_VER_SIGN_S_ORDER_ERROR == VerifyStatus) {
			Status = (int)XSECURE_ELLIPTIC_VER_SIGN_S_ORDER_ERROR;
		}
		else if (ELLIPTIC_SUCCESS != VerifyStatus) {
			Status = XST_FAILURE;
		}
		else {
			Status = XST_SUCCESS;
		}
	}

END:
	XSecure_SetReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function verifies the signature for a given hash, key and
 *		curve type
 *
 * @param	CrvType - Type of elliptic curve
 * @param	Hash    - Pointer to the hash for which sign has to be generated
 * @param	HashLen - Length of hash in bytes
 * @param	Key     - Pointer to the public key
 * @param	Sign    - Pointer to the signature
 *
 * @return
 *	- XST_SUCCESS - On success
 *	- XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *	- XSECURE_ELLIPTIC_BAD_SIGN - When signature provided for verification is bad
 *	- XSECURE_ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN - Incorrect hash length
 *						for sign verification
 *	- XSECURE_ELLIPTIC_VER_SIGN_R_ZERO - R set to zero
 *	- XSECURE_ELLIPTIC_VER_SIGN_S_ZERO - S set to zero
 *	- XSECURE_ELLIPTIC_VER_SIGN_R_ORDER_ERROR - R is not within ECC order
 *	- XSECURE_ELLIPTIC_VER_SIGN_S_ORDER_ERROR - S is not within ECC order
 *	- XST_FAILURE - On failure
 *
 *****************************************************************************/
int XSecure_EllipticVerifySign(XSecure_EllipticCrvTyp CrvType, const u8 *Hash,
	const u32 HashLen, XSecure_EllipticKey *Key, XSecure_EllipticSign *Sign)
{
	XSecure_EllipticSignAddr SignAddr = {(u64)(UINTPTR)Sign->SignR,
		(u64)(UINTPTR)Sign->SignS};

	XSecure_EllipticKeyAddr KeyAddr = {(u64)(UINTPTR)Key->Qx,
		(u64)(UINTPTR)Key->Qy};

	XSecure_EllipticHashData HashInfo = {(u64)(UINTPTR)Hash, HashLen};

	return XSecure_EllipticVerifySign_64Bit(CrvType,
			(XSecure_EllipticHashData *) &HashInfo,
			(XSecure_EllipticKeyAddr *) &KeyAddr,
			(XSecure_EllipticSignAddr *) &SignAddr);
}

/*****************************************************************************/
/**
 * @brief	This function performs known answer test(KAT) on ECC core
 *
 * @param	AuthCurve - Type of ECC curve used for authentication
 *
 * @return
 *		- XST_SUCCESS - On success
 * 		- XSECURE_ELLIPTIC_KAT_KEY_NOTVALID_ERROR - When elliptic key is not valid
 * 		- XSECURE_ELLIPTIC_KAT_FAILED_ERROR       - When elliptic KAT fails
 *
 *****************************************************************************/
int XSecure_EllipticKat(u32 AuthCurve)
{
	volatile int Status = (int)XSECURE_ELLIPTIC_KAT_FAILED_ERROR;

	static const u32 QxCord_P384[XSECURE_ECC_P384_DATA_SIZE_WORDS] = {
		0x88371BE6U, 0xFD2D8761U, 0x30DA0A10U, 0xEA9DBD2EU,
		0x30FB204AU, 0x1361EFBAU, 0xF9FDF2CEU, 0x48405353U,
		0xDE06D343U, 0x335DFF33U, 0xCBF43FDFU, 0x6C037A0U
	};

	static const u32 QyCord_P384[XSECURE_ECC_P384_DATA_SIZE_WORDS] = {
		0xEA662A43U, 0xD380E26EU, 0x57AA933CU, 0x4DD77035U,
		0x5891AD86U, 0x7AB634EDU, 0x3E46D080U, 0xD97F2544U,
		0xBF70B8A4U, 0x9204B98FU, 0x940E3467U, 0x360D38F3U
	};

	static const u32 KatSignR_P384[XSECURE_ECC_P384_DATA_SIZE_WORDS] = {
		0x52D853B5U, 0x41531533U, 0x2D1B4AA6U, 0x6EAF0088U,
		0x4E88153DU, 0x9F0AB1AAU, 0x12A416D8U, 0x7A50E599U,
		0xB7CA0FA0U, 0x330C7507U, 0x3495767EU, 0x5886078DU
	};

	static const u32 KatSignS_P384[XSECURE_ECC_P384_DATA_SIZE_WORDS] = {
		0x7A36E1AAU, 0x329682AEU, 0xE17F691BU, 0xF3869DA0U,
		0xE32BDE69U, 0x6F78CDC4U, 0x89C8FF9FU, 0x449A3523U,
		0x82CC2114U, 0xFD14B06BU, 0xBF1BF8CCU, 0x2CC10023U
	};

	static const u32 HashVal_P384[XSECURE_ECC_P384_DATA_SIZE_WORDS] = {
		0x925FA874U, 0x331B36FBU, 0x13173C62U, 0x57633F17U,
		0x110BA0CDU, 0x9E3B9A7DU, 0x46DE70D2U, 0xB30870DBU,
		0xF3CA965DU, 0xADAA0A68U, 0x9573A993U, 0x1128C8B0U
	};

	static const u8 QxCord_P521[] = {
		0xC4U, 0xD5U, 0x85U, 0x18U, 0x17U, 0x8BU, 0xF1U, 0x8DU,
		0xB6U, 0xFEU, 0xECU, 0x0DU, 0x03U, 0xACU, 0xD8U, 0x05U,
		0x30U, 0x4BU, 0xE5U, 0xB1U, 0x56U, 0x70U, 0xA3U, 0x67U,
		0xA4U, 0x6CU, 0xDCU, 0x6BU, 0x3AU, 0x40U, 0xDFU, 0x59U,
		0x6EU, 0xA1U, 0xCCU, 0x10U, 0x64U, 0x8FU, 0xABU, 0xE9U,
		0x55U, 0xB2U, 0x96U, 0xD7U, 0x8EU, 0xDAU, 0xC1U, 0x17U,
		0xF1U, 0xF5U, 0x53U, 0xB4U, 0xFAU, 0x52U, 0x9CU, 0x30U,
		0x22U, 0x28U, 0x45U, 0x68U, 0x9AU, 0xEFU, 0x1EU, 0xE9U,
		0x98U, 0x00U,
	};

	static const u8 QyCord_P521[] = {
		0x2EU, 0xBCU, 0x46U, 0xD9U, 0x50U, 0xFDU, 0x32U, 0x46U,
		0x9AU, 0x99U, 0x92U, 0xF0U, 0xF5U, 0x13U, 0xE3U, 0x26U,
		0x15U, 0x23U, 0x27U, 0x83U, 0x66U, 0xADU, 0x83U, 0x4BU,
		0x2CU, 0x71U, 0x00U, 0x29U, 0xB7U, 0x76U, 0x43U, 0x55U,
		0xE8U, 0x7BU, 0xF1U, 0x5EU, 0x98U, 0x31U, 0x7FU, 0x8EU,
		0xD2U, 0xD7U, 0x48U, 0x6AU, 0x8DU, 0xB7U, 0xB4U, 0x50U,
		0x61U, 0x65U, 0x15U, 0x9BU, 0x4CU, 0x36U, 0xA4U, 0x1BU,
		0xCAU, 0x1CU, 0xFCU, 0xECU, 0x1AU, 0x32U, 0x0CU, 0x35U,
		0x64U, 0x01U,
	};

	static const u8 KatSignR_P521[] = {
		0xBAU, 0xE2U, 0xC3U, 0x5DU, 0xFEU, 0xCCU, 0x54U, 0x92U,
		0x1AU, 0x64U, 0x96U, 0x9BU, 0x7CU, 0xC3U, 0x91U, 0x28U,
		0x73U, 0x8AU, 0x4AU, 0x2BU, 0x6EU, 0xD5U, 0xA9U, 0x32U,
		0x04U, 0x8CU, 0x2FU, 0xF3U, 0xEEU, 0xEEU, 0x26U, 0x25U,
		0x1CU, 0xCDU, 0xDAU, 0x9AU, 0x26U, 0x79U, 0xDCU, 0x8FU,
		0x55U, 0x1DU, 0xFCU, 0x51U, 0x24U, 0xE6U, 0x2DU, 0x1EU,
		0xD8U, 0x74U, 0xADU, 0xD3U, 0xDDU, 0x40U, 0xA2U, 0xE7U,
		0xF7U, 0xE3U, 0x8CU, 0x10U, 0x57U, 0xCAU, 0xEDU, 0xC8U,
		0x40U, 0x01U,
	};

	static const u8 KatSignS_P521[] = {
		0xB1U, 0xB5U, 0x88U, 0xB9U, 0x1CU, 0x95U, 0x32U, 0x82U,
		0x9AU, 0x5CU, 0x1FU, 0xC6U, 0xB2U, 0x37U, 0xCEU, 0xE2U,
		0xEEU, 0x54U, 0x1CU, 0xAFU, 0x77U, 0x8EU, 0x37U, 0x61U,
		0x6BU, 0xEBU, 0xBDU, 0x55U, 0x84U, 0x29U, 0x9EU, 0xBDU,
		0x15U, 0x86U, 0x1EU, 0x2CU, 0x0AU, 0x0EU, 0xB2U, 0xEAU,
		0xB3U, 0x5DU, 0x54U, 0x3BU, 0x58U, 0x49U, 0x8DU, 0x7FU,
		0xC0U, 0xD7U, 0xBFU, 0x85U, 0x99U, 0x1DU, 0x65U, 0xA6U,
		0x6DU, 0x37U, 0x96U, 0x00U, 0x9DU, 0x22U, 0x15U, 0x2FU,
		0xD7U, 0x00U,
	};

	static const u8 HashVal_P521[] = {
		0x32U, 0xF9U, 0xE1U, 0x0BU, 0xE6U, 0x1DU, 0xF7U, 0xB6U,
		0xA8U, 0x67U, 0x17U, 0x58U, 0x8EU, 0x6DU, 0xD6U, 0xC0U,
		0x72U, 0x91U, 0xCDU, 0xDDU, 0x6CU, 0xBDU, 0xBEU, 0x2FU,
		0x13U, 0xFAU, 0x02U, 0x5BU, 0x02U, 0x90U, 0xAFU, 0x32U,
		0x5DU, 0x20U, 0x09U, 0xA7U, 0x1CU, 0x2CU, 0x58U, 0x94U,
		0x9FU, 0xBBU, 0x75U, 0xDCU, 0xE1U, 0x8DU, 0x36U, 0xD7U,
		0xCEU, 0xB1U, 0xB6U, 0x7CU, 0x7FU, 0xB7U, 0x25U, 0xF9U,
		0x00U, 0x1EU, 0xA3U, 0xEDU, 0xDEU, 0xE1U, 0xF0U, 0x9BU,
		0x00U, 0x00U,
	};

	XSecure_EllipticKey Key = {0U};
	XSecure_EllipticSign ExpectedSign = {0U};
	XSecure_EllipticCrvTyp CrvType = XSECURE_ECC_NIST_P384;
	const u8 *HashVal;
	u32 HashLen;

	if (AuthCurve == XSECURE_ECDSA_KAT_NIST_P384) {
		CrvType = XSECURE_ECC_NIST_P384;
		Key.Qx = (u8 *)QxCord_P384;
		Key.Qy = (u8 *)QyCord_P384;
		ExpectedSign.SignR = (u8*)KatSignR_P384;
		ExpectedSign.SignS = (u8*)KatSignS_P384;
		HashVal = (u8*)HashVal_P384;
		HashLen = XSECURE_ECC_P384_SIZE_IN_BYTES;
	}
	else if (AuthCurve == XSECURE_ECDSA_KAT_NIST_P521) {
		CrvType = XSECURE_ECC_NIST_P521;
		Key.Qx = (u8 *)QxCord_P521;
		Key.Qy = (u8 *)QyCord_P521;
		ExpectedSign.SignR = (u8 *)KatSignR_P521;
		ExpectedSign.SignS = (u8 *)KatSignS_P521;
		HashVal = (u8 *)HashVal_P521;
		HashLen = XSECURE_ECC_P521_SIZE_IN_BYTES;
	}
	else {
		xil_printf("Only NIST P-384 and NIST P-521 curves are"
			"supported for KAT");
		Status = (int)XSECURE_ELLIPTIC_KAT_INVLD_CRV_ERROR;
		goto END;
	}

	Status = XSecure_EllipticValidateKey(CrvType, &Key);
	if(Status != XST_SUCCESS) {
		Status = (int)XSECURE_ELLIPTIC_KAT_KEY_NOTVALID_ERROR;
		goto END;
	}

	Status = (int)XSECURE_ELLIPTIC_KAT_FAILED_ERROR;

	Status = XSecure_EllipticVerifySign(CrvType, HashVal, HashLen, &Key,
		&ExpectedSign);
	if(Status != XST_SUCCESS) {
		Status = (int)XSECURE_ELLIPTIC_KAT_FAILED_ERROR;
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function gets the curve related information
 *
 * @param	CrvTyp - Type of the elliptic curve
 *
 * @return	Crv    - Curve information
 *
 *****************************************************************************/
static EcdsaCrvInfo* XSecure_EllipticGetCrvData(XSecure_EllipticCrvTyp CrvTyp)
{
	u32 Index;
	EcdsaCrvInfo *Crv = NULL;
	u32 TotalCurves = XSecure_EllipticCrvsGetCount();

	for(Index = 0U; Index < TotalCurves; Index++) {
		if (XSecure_EllipticCrvsDb[Index].CrvType == (EcdsaCrvTyp)CrvTyp) {
			Crv = &XSecure_EllipticCrvsDb[Index];
			break;
		}
	}

	return Crv;
}

/*****************************************************************************/
/**
 * @brief	This function copies data from 32/64 bit address to
 *		local buffer.
 *
 * @param	Size 	- Length of data in bytes
 * @param	Dst     - Pointer to the destination buffer
 * @param	SrcAddr - Source address
 *
 *****************************************************************************/
static void XSecure_PutData(const u32 Size, u8 *Dst, const u64 SrcAddr)
{
	u32 Index = 0U;

	for (Index = 0U; Index < Size; Index++) {
		Dst[Index] = XSecure_InByte64((SrcAddr + Index));
	}
}

/*****************************************************************************/
/**
 * @brief	This function copies data to 32/64 bit address from
 *		local buffer.
 *
 * @param	Size 	- Length of data in bytes
 * @param	Src     - Pointer to the source buffer
 * @param	DstAddr - Destination address
 *
 *****************************************************************************/
static void XSecure_GetData(const u32 Size, const u8 *Src, const u64 DstAddr)
{
	u32 Index = 0U;

	for (Index = 0U; Index < Size; Index++) {
		XSecure_OutByte64((DstAddr + Index), Src[Index]);
	}
}
