/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 4.7   am  01/07/22  Removed unused labels KEY_ERR and SIG_ERR by replacing
*                     XSECURE_TEMPORAL_CHECK with XSECURE_TEMPORAL_IMPL macros and
*                     status variable is reinitialized with XST_FAILURE before using
*                     it further in the respective functions
*       har  01/20/22 Added glitch checks for clearing keys in
*                     XSecure_EllipticGenerateKey_64Bit() and
*                     XSecure_EllipticGenerateSignature_64Bit()
*       har  02/16/22 Updated Status with ClearStatus only in case of success
*       dc   07/13/22 Modified static function XSecure_EllipticGetCrvData()
*                     to non static
* 5.0   kpt  07/24/22 Moved XSecure_EllipticKat into xsecure_kat.c
*       dc   08/26/22 Removed initializations of arrays
* 5.1   dc   03/30/23 Added support to accept the data in either big/little endian.
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"

#ifndef PLM_ECDSA_EXCLUDE
#include "xsecure_error.h"
#include "xsecure_elliptic.h"
#include "xsecure_ecdsa_rsa_hw.h"
#include "xsecure_utils.h"
#include "xil_util.h"
#include "xsecure_cryptochk.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XSECURE_ECDSA_P521_ALIGN_BYTES	2U
				/**< Size of NIST P-521 curve is 66 bytes. This macro is used
				to make the address word aligned */

/************************** Function Prototypes ******************************/
EcdsaCrvInfo* XSecure_EllipticGetCrvData(XSecure_EllipticCrvTyp CrvTyp);
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
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_ELLIPTIC_NON_SUPPORTED_CRV - When elliptic Curve is not supported
 *	-	XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *	-	XSECURE_ELLIPTIC_GEN_KEY_ERR - Error in generating Public key
 *
 *****************************************************************************/
int XSecure_EllipticGenerateKey_64Bit(XSecure_EllipticCrvTyp CrvType,
	const u64 DAddr, XSecure_EllipticKeyAddr *KeyAddr)
{
	volatile int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	volatile int ClearStatus = XST_FAILURE;
	volatile int ClearStatusTmp = XST_FAILURE;
	EcdsaCrvInfo *Crv = NULL;
	u8 PubKey[XSECURE_ECC_P521_SIZE_IN_BYTES +
		XSECURE_ECDSA_P521_ALIGN_BYTES +
		XSECURE_ECC_P521_SIZE_IN_BYTES];
	u8 D[XSECURE_ECC_P521_SIZE_IN_BYTES];
	EcdsaKey Key;
	u32 Size = 0U;
	u32 OffSet = 0U;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
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
	ClearStatus = Xil_SecureZeroize((u8*)D, XSECURE_ECC_P521_SIZE_IN_BYTES);
	ClearStatusTmp = Xil_SecureZeroize((u8*)D, XSECURE_ECC_P521_SIZE_IN_BYTES);
	if (Status == XST_SUCCESS) {
		Status = ClearStatusTmp | ClearStatus;
	}

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
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_ELLIPTIC_NON_SUPPORTED_CRV - When elliptic Curve is not supported
 *	-	XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *	-	XSECURE_ELLIPTIC_GEN_KEY_ERR - Error in generating Public key
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
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *	-	XSECURE_ELLIPTIC_GEN_SIGN_BAD_RAND_NUM - When Bad random number used
 *						for sign generation
 *	-	XSECURE_ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN - Incorrect hash length for sign
 *							generation
 *	-	XST_FAILURE - On failure
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
	volatile int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	volatile int GenStatus = XST_FAILURE;
	volatile int GenStatusTmp = XST_FAILURE;
	volatile int ClearStatus = XST_FAILURE;
	volatile int ClearStatusTmp = XST_FAILURE;
	EcdsaCrvInfo *Crv = NULL;
	u8 PaddedHash[XSECURE_ECC_P521_SIZE_IN_BYTES];
	u8 D[XSECURE_ECC_P521_SIZE_IN_BYTES];
	u8 K[XSECURE_ECC_P521_SIZE_IN_BYTES];
	u8 Signature[XSECURE_ECC_P521_SIZE_IN_BYTES +
		XSECURE_ECDSA_P521_ALIGN_BYTES +
		XSECURE_ECC_P521_SIZE_IN_BYTES];

	EcdsaSign Sign;
	volatile u32 HashLenTmp = 0xFFFFFFFFU;
	u32 OffSet = 0U;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	if ((CrvType != XSECURE_ECC_NIST_P384) &&
			(CrvType != XSECURE_ECC_NIST_P521)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	Status = Xil_SMemSet(PaddedHash, XSECURE_ECC_P521_SIZE_IN_BYTES,
				0U, XSECURE_ECC_P521_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
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
		XSECURE_TEMPORAL_IMPL(GenStatus, GenStatusTmp, Ecdsa_GenerateSign,
			Crv, PaddedHash, Crv->Bits, D, K, (EcdsaSign *)&Sign);

		if ((GenStatus == ELLIPTIC_GEN_SIGN_BAD_R) ||
			(GenStatusTmp == ELLIPTIC_GEN_SIGN_BAD_R) ||
			(GenStatus == ELLIPTIC_GEN_SIGN_BAD_S) ||
			(GenStatusTmp == ELLIPTIC_GEN_SIGN_BAD_S)) {
			Status = (int)XSECURE_ELLIPTIC_GEN_SIGN_BAD_RAND_NUM;
		}
		else if ((GenStatus == ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN) ||
			(GenStatusTmp == ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN)) {
			Status = (int)XSECURE_ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN;
		}
		else if ((GenStatus != ELLIPTIC_SUCCESS) ||
			(GenStatusTmp != ELLIPTIC_SUCCESS)) {
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
	ClearStatus = Xil_SecureZeroize((u8*)D, XSECURE_ECC_P521_SIZE_IN_BYTES);
	ClearStatusTmp = Xil_SecureZeroize((u8*)D, XSECURE_ECC_P521_SIZE_IN_BYTES);
	if (Status == XST_SUCCESS) {
		Status = ClearStatusTmp | ClearStatus;
	}

	ClearStatus = Xil_SecureZeroize((u8*)K, XSECURE_ECC_P521_SIZE_IN_BYTES);
	ClearStatusTmp = Xil_SecureZeroize((u8*)K, XSECURE_ECC_P521_SIZE_IN_BYTES);
	if (Status == XST_SUCCESS) {
		Status = ClearStatusTmp | ClearStatus;
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
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *	-	XSECURE_ELLIPTIC_GEN_SIGN_BAD_RAND_NUM - When Bad random number used
 *						for sign generation
 *	-	XSECURE_ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN - Incorrect hash
 *							length for sign generation
 *	-	XST_FAILURE - On failure
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
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *	-	XSECURE_ELLIPTIC_KEY_ZERO - When Public key is zero
 *	-	XSECURE_ELLIPTIC_KEY_WRONG_ORDER - Wrong order of Public key
 *	-	XSECURE_ELLIPTIC_KEY_NOT_ON_CRV - When Key is not found on the curve
 *	-	XST_FAILURE - On failure
 *
 *****************************************************************************/
int XSecure_EllipticValidateKey_64Bit(XSecure_EllipticCrvTyp CrvType,
		XSecure_EllipticKeyAddr *KeyAddr)
{
	volatile int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	volatile int ValidateStatus = XST_FAILURE;
	volatile int ValidateStatusTmp = XST_FAILURE;
	EcdsaCrvInfo *Crv = NULL;
	EcdsaKey Key;
	u8 PubKey[XSECURE_ECC_P521_SIZE_IN_BYTES +
		XSECURE_ECDSA_P521_ALIGN_BYTES +
		XSECURE_ECC_P521_SIZE_IN_BYTES];
	u32 Size = 0U;
	u32 OffSet = 0U;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
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
		XSECURE_TEMPORAL_IMPL(ValidateStatus, ValidateStatusTmp,
			Ecdsa_ValidateKey, Crv, (EcdsaKey *)&Key);

		if ((ValidateStatus == ELLIPTIC_KEY_ZERO) ||
			(ValidateStatusTmp == ELLIPTIC_KEY_ZERO)) {
			Status = (int)XSECURE_ELLIPTIC_KEY_ZERO;
		}
		else if ((ValidateStatus == ELLIPTIC_KEY_WRONG_ORDER) ||
			(ValidateStatusTmp == ELLIPTIC_KEY_WRONG_ORDER)) {
			Status = (int)XSECURE_ELLIPTIC_KEY_WRONG_ORDER;
		}
		else if ((ValidateStatus == ELLIPTIC_KEY_NOT_ON_CRV) ||
			(ValidateStatusTmp == ELLIPTIC_KEY_NOT_ON_CRV)) {
			Status = (int)XSECURE_ELLIPTIC_KEY_NOT_ON_CRV;
		}
		else if ((ValidateStatus != ELLIPTIC_SUCCESS) ||
			(ValidateStatusTmp != ELLIPTIC_SUCCESS)) {
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
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *	-	XSECURE_ELLIPTIC_KEY_ZERO - When Public key is zero
 *	-	XSECURE_ELLIPTIC_KEY_WRONG_ORDER - Wrong order of Public key
 *	-	XSECURE_ELLIPTIC_KEY_NOT_ON_CRV - When Key is not found on the curve
 *	-	XST_FAILURE - On failure
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
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *	-	XSECURE_ELLIPTIC_BAD_SIGN - When signature provided for verification is bad
 *	-	XSECURE_ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN - Incorrect hash length
 *						for sign verification
 *	-	XSECURE_ELLIPTIC_VER_SIGN_R_ZERO - R set to zero
 *	-	XSECURE_ELLIPTIC_VER_SIGN_S_ZERO - S set to zero
 *	-	XSECURE_ELLIPTIC_VER_SIGN_R_ORDER_ERROR - R is not within ECC order
 *	-	XSECURE_ELLIPTIC_VER_SIGN_S_ORDER_ERROR - S is not within ECC order
 *	-	XST_FAILURE - On failure
 *
 *****************************************************************************/
int XSecure_EllipticVerifySign_64Bit(XSecure_EllipticCrvTyp CrvType,
	XSecure_EllipticHashData *HashInfo, XSecure_EllipticKeyAddr *KeyAddr,
	XSecure_EllipticSignAddr *SignAddr)
{
	volatile int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	volatile int VerifyStatus = XST_FAILURE;
	volatile int VerifyStatusTmp = XST_FAILURE;
	EcdsaCrvInfo *Crv = NULL;
	u8 PaddedHash[XSECURE_ECC_P521_SIZE_IN_BYTES];
	volatile u32 HashLenTmp = 0xFFFFFFFFU;
	u8 PubKey[XSECURE_ECC_P521_SIZE_IN_BYTES +
		XSECURE_ECDSA_P521_ALIGN_BYTES +
		XSECURE_ECC_P521_SIZE_IN_BYTES];
	u8 Signature[XSECURE_ECC_P521_SIZE_IN_BYTES +
		XSECURE_ECDSA_P521_ALIGN_BYTES +
		XSECURE_ECC_P521_SIZE_IN_BYTES];
	EcdsaKey Key;
	EcdsaSign Sign;
	u32 OffSet = 0U;
	u32 Size = 0U;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
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

	Status = Xil_SMemSet(PaddedHash, XSECURE_ECC_P521_SIZE_IN_BYTES,
				0U, XSECURE_ECC_P521_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
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
		XSECURE_TEMPORAL_IMPL(VerifyStatus, VerifyStatusTmp, Ecdsa_VerifySign,
			Crv, PaddedHash, Crv->Bits, (EcdsaKey *)&Key, (EcdsaSign *)&Sign);

		if ((ELLIPTIC_BAD_SIGN == VerifyStatus) ||
			(ELLIPTIC_BAD_SIGN == VerifyStatusTmp)) {
			Status = (int)XSECURE_ELLIPTIC_BAD_SIGN;
		}
		else if ((ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN == VerifyStatus) ||
			(ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN == VerifyStatusTmp)) {
			Status = (int)XSECURE_ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN;
		}
		else if ((ELLIPTIC_VER_SIGN_R_ZERO == VerifyStatus) ||
			(ELLIPTIC_VER_SIGN_R_ZERO == VerifyStatusTmp)) {
			Status = (int)XSECURE_ELLIPTIC_VER_SIGN_R_ZERO;
		}
		else if ((ELLIPTIC_VER_SIGN_S_ZERO == VerifyStatus) ||
			(ELLIPTIC_VER_SIGN_S_ZERO == VerifyStatusTmp)) {
			Status = (int)XSECURE_ELLIPTIC_VER_SIGN_S_ZERO;
		}
		else if ((ELLIPTIC_VER_SIGN_R_ORDER_ERROR == VerifyStatus) ||
			(ELLIPTIC_VER_SIGN_R_ORDER_ERROR == VerifyStatusTmp)) {
			Status = (int)XSECURE_ELLIPTIC_VER_SIGN_R_ORDER_ERROR;
		}
		else if ((ELLIPTIC_VER_SIGN_S_ORDER_ERROR == VerifyStatus) ||
			(ELLIPTIC_VER_SIGN_S_ORDER_ERROR == VerifyStatusTmp)) {
			Status = (int)XSECURE_ELLIPTIC_VER_SIGN_S_ORDER_ERROR;
		}
		else if ((ELLIPTIC_SUCCESS != VerifyStatus) ||
			(ELLIPTIC_SUCCESS != VerifyStatusTmp)) {
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
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *	-	XSECURE_ELLIPTIC_BAD_SIGN - When signature provided for verification is bad
 *	-	XSECURE_ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN - Incorrect hash length
 *						for sign verification
 *	-	XSECURE_ELLIPTIC_VER_SIGN_R_ZERO - R set to zero
 *	-	XSECURE_ELLIPTIC_VER_SIGN_S_ZERO - S set to zero
 *	-	XSECURE_ELLIPTIC_VER_SIGN_R_ORDER_ERROR - R is not within ECC order
 *	-	XSECURE_ELLIPTIC_VER_SIGN_S_ORDER_ERROR - S is not within ECC order
 *	-	XST_FAILURE - On failure
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
 * @brief	This function gets the curve related information
 *
 * @param	CrvTyp - Type of the elliptic curve
 *
 * @return
 *	-	Crv - Curve information
 *
 *****************************************************************************/
EcdsaCrvInfo* XSecure_EllipticGetCrvData(XSecure_EllipticCrvTyp CrvTyp)
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
	s32 RIndex = (s32)Size - 1;

	for (Index = 0U; (Index < Size) && (RIndex >= 0); Index++, RIndex--) {
		if (XSECURE_ELLIPTIC_ENDIANNESS == XSECURE_ELLIPTIC_LITTLE_ENDIAN) {
			Dst[Index] = XSecure_InByte64((SrcAddr + Index));
		}
		else {
			Dst[Index] = XSecure_InByte64((SrcAddr + (u64)RIndex));
		}
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
	s32 RIndex = (s32)Size - 1;

	for (Index = 0U; (Index < Size) && (RIndex >= 0); Index++, RIndex--) {
		if (XSECURE_ELLIPTIC_ENDIANNESS == XSECURE_ELLIPTIC_LITTLE_ENDIAN) {
			XSecure_OutByte64((DstAddr + Index), Src[Index]);
		}
		else {
			XSecure_OutByte64((DstAddr + Index), Src[RIndex]);
		}
	}
}

#endif
