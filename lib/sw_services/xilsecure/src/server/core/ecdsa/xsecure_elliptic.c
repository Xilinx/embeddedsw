/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 5.2   yog  05/18/23 Updated the flow for Big Endian ECC Mode setting
*       yog  06/07/23 Added support for P-256 Curve
*       ng   07/05/23 Added support for system device tree flow
*       yog  08/07/23 Initialised trng before calling IpCores functions
*       am   08/18/23 Added XSecure_EllipticValidateAndGetCrvInfo and
*                     XSecure_EllipticGetCrvSize functions
*       yog  09/04/23 Restricted XSecure_ECCRandInit API support to VersalNet
*       vss  09/11/23 Fixed MISRA-C Rule 8.13 violation
* 5.3   kpt  03/22/24 Fixed Branch past initialization
* 5.4   yog  04/29/24 Fixed doxygen warnings.
*       mb   05/23/24 Added P192 curve support
*       mb   05/23/24 Added P224 urve support
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_ecdsa_server_apis XilSecure ECDSA Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xparameters.h"

#ifndef PLM_ECDSA_EXCLUDE
#include "xsecure_error.h"
#include "xsecure_elliptic.h"
#include "xsecure_ecdsa_rsa_hw.h"
#include "xsecure_utils.h"
#include "xil_sutil.h"
#include "xsecure_plat.h"
#ifdef SDT
#include "xsecure_config.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XSECURE_ECDSA_ALGN_CRV_SIZE_IN_BYTES	(2U)	/**< Align ECDSA curve size in bytes */

#define XSECURE_ECDSA_BITS_IN_BYTES	(8U)	/**< Bits in bytes */

/************************** Function Prototypes ******************************/
EcdsaCrvInfo* XSecure_EllipticGetCrvData(XSecure_EllipticCrvTyp CrvTyp);
static u32 XSecure_EllipticValidateAndGetCrvInfo(XSecure_EllipticCrvTyp CrvType,
	EcdsaCrvInfo** Crv);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * @brief	This function generates Public Key for a given curve type using
 *		private key where both keys located at 64 bit address
 *
 * @param	CrvType	Is a type of elliptic curve
 * @param	DAddr	Address of static private key
 * @param	KeyAddr	Pointer to public key address
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_ELLIPTIC_NON_SUPPORTED_CRV  When elliptic Curve is not supported
 *		 - XSECURE_ELLIPTIC_INVALID_PARAM  On invalid argument
 *		 - XSECURE_ELLIPTIC_GEN_KEY_ERR  Error in generating Public key
 *
 *****************************************************************************/
int XSecure_EllipticGenerateKey_64Bit(XSecure_EllipticCrvTyp CrvType,
	const u64 DAddr, const XSecure_EllipticKeyAddr *KeyAddr)
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

	OffSet = XSecure_EllipticValidateAndGetCrvInfo(CrvType, &Crv);
	if ((OffSet == 0U) || (Crv == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
		goto END;
	}

	if (KeyAddr == NULL) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	Size = OffSet;
	if (CrvType == XSECURE_ECC_NIST_P521) {
		OffSet += XSECURE_ECDSA_P521_ALIGN_BYTES;
	}

#ifdef VERSAL_NET
	Status = XST_FAILURE;
	Status = XSecure_ECCRandInit();
	if(Status != XST_SUCCESS) {
		goto END;
	}
#endif

	/* Store Priv key to local buffer */
	XSecure_PutData(Size, (u8*)D, DAddr);

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + OffSet);

	/* Place the hardware core into the reset */
	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);

	Status = XST_FAILURE;
	/** Generate public key with provided private key and curve type */
	Status = Ecdsa_GeneratePublicKey(Crv, D, (EcdsaKey *)&Key);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_ELLIPTIC_GEN_KEY_ERR;
	}
	/* Store key to destination address */
	XSecure_GetData(Size, (u8 *)PubKey, KeyAddr->Qx);
			XSecure_GetData(Size,
					(u8 *)(PubKey + OffSet),
					KeyAddr->Qy);

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
 * @param	CrvType	Is a type of elliptic curve
 * @param	D	Pointer to static private key
 * @param	Key	Pointer to public key
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_ELLIPTIC_NON_SUPPORTED_CRV  When elliptic Curve is not supported
 *		 - XSECURE_ELLIPTIC_INVALID_PARAM  On invalid argument
 *
 *****************************************************************************/
int XSecure_EllipticGenerateKey(XSecure_EllipticCrvTyp CrvType, const u8* D,
	const XSecure_EllipticKey *Key)
{
	volatile int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	XSecure_EllipticKeyAddr KeyAddr;

	if ((CrvType != XSECURE_ECC_NIST_P384) &&
		(CrvType != XSECURE_ECC_NIST_P521) &&
		(CrvType != XSECURE_ECC_NIST_P256) &&
		(CrvType != XSECURE_ECC_NIST_P192) &&
		(CrvType != XSECURE_ECC_NIST_P224)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	if ((D == NULL) || (Key == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	KeyAddr.Qx = (u64)(UINTPTR)Key->Qx;
	KeyAddr.Qy = (u64)(UINTPTR)Key->Qy;

	/** Generate public key with provided private key and curve type */
	Status = XSecure_EllipticGenerateKey_64Bit(CrvType, (u64)(UINTPTR)D,
			(XSecure_EllipticKeyAddr *) &KeyAddr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function generates signature for a given hash and curve
 *		type where data is located at 64-bit address.
 *
 * @param	CrvType		Type of elliptic curve
 * @param	HashInfo	Pointer to Hash Data i.e. Hash Address and length
 * @param	DAddr		Address of the static private key
 * @param	KAddr		Ephemeral private key
 * @param	SignAddr	Pointer to signature address
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_ELLIPTIC_INVALID_PARAM  On invalid argument
 *		 - XSECURE_ELLIPTIC_NON_SUPPORTED_CRV  If curve data pointer is NULL
 *		 - XSECURE_ELLIPTIC_GEN_SIGN_BAD_RAND_NUM  When Bad random number used
 *						for sign generation
 *		 - XSECURE_ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN  Incorrect hash length for sign
 *							generation
 *		 - XST_FAILURE  On any other failures
 *
 * @note
 * K, the ephemeral private key, shall be an unpredictable (cryptographically
 * secure) random number unique for each signature
 * Note that reuse or external predictability of this number generally breaks
 * the security of ECDSA
 *
 *****************************************************************************/
int XSecure_EllipticGenerateSignature_64Bit(XSecure_EllipticCrvTyp CrvType,
	const XSecure_EllipticHashData *HashInfo, const u64 DAddr,
	const u64 KAddr, const XSecure_EllipticSignAddr *SignAddr)
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
	u32 Size = 0U;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((HashInfo == NULL) || (SignAddr == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	HashLenTmp = HashInfo->Len;
	if ((HashInfo->Len > XSECURE_ECC_P521_SIZE_IN_BYTES) ||
		(HashLenTmp > XSECURE_ECC_P521_SIZE_IN_BYTES)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

#ifdef VERSAL_NET
	Status = XST_FAILURE;
	Status = XSecure_ECCRandInit();
	if(Status != XST_SUCCESS) {
		goto END;
	}
#endif

	OffSet = XSecure_EllipticValidateAndGetCrvInfo(CrvType, &Crv);
	if ((OffSet == 0U) || (Crv == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
		goto END;
	}

	Size = OffSet;
	if (CrvType == XSECURE_ECC_NIST_P521) {
		OffSet += XSECURE_ECDSA_P521_ALIGN_BYTES;
	}

	Status = XST_FAILURE;
	Status = Xil_SMemSet(PaddedHash, XSECURE_ECC_P521_SIZE_IN_BYTES,
				0U, XSECURE_ECC_P521_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Store Hash,D,K to local buffers */
	XSecure_PutData(HashInfo->Len, (u8 *)PaddedHash, HashInfo->Addr);
	XSecure_PutData(Size, (u8 *)D, DAddr);
	XSecure_PutData(Size, (u8 *)K, KAddr);

	Sign.r = (u8 *)Signature;
	Sign.s = (u8 *)(Signature + OffSet);

	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);

	/**
	 * Generate signature with provided hash, private key, ephemeral key
	 * and curve type.
	 */
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
		XSecure_GetData(Size, (u8 *)Signature, SignAddr->SignR);
		XSecure_GetData(Size, (u8 *)(Signature + OffSet), SignAddr->SignS);
		Status = XST_SUCCESS;
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
 * @param	CrvType	Type of elliptic curve
 * @param	Hash	Pointer to the hash for which sign has to be generated
 * @param	HashLen	Length of the hash in bytes
 * @param	D	Pointer to the static private key
 * @param	K	Ephemeral private key
 * @param	Sign	Pointer to the signature
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_ELLIPTIC_INVALID_PARAM  On invalid argument
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
	const u8* K, const XSecure_EllipticSign *Sign)
{
	volatile int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	XSecure_EllipticSignAddr SignAddr;
	XSecure_EllipticHashData HashInfo;

	if ((CrvType != XSECURE_ECC_NIST_P384) &&
		(CrvType != XSECURE_ECC_NIST_P521) &&
		(CrvType != XSECURE_ECC_NIST_P256) &&
		(CrvType != XSECURE_ECC_NIST_P192) &&
		(CrvType != XSECURE_ECC_NIST_P224)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	if ((Hash == NULL) || (D == NULL) || (K == NULL) || (Sign == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	if (HashLen > XSECURE_ECC_P521_SIZE_IN_BYTES) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	SignAddr.SignR = (u64)(UINTPTR)Sign->SignR;
	SignAddr.SignS = (u64)(UINTPTR)Sign->SignS;

	HashInfo.Addr = (u64)(UINTPTR)Hash;
	HashInfo.Len = HashLen;

	/**
	 * Generate signature with provided hash, private key, ephemeral key
	 * and curve type
	 */
	Status = XSecure_EllipticGenerateSignature_64Bit(CrvType,
			(XSecure_EllipticHashData *) &HashInfo,
			(u64)(UINTPTR)D,
			(u64)(UINTPTR)K,
			(XSecure_EllipticSignAddr *) &SignAddr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function validates the public key for a given curve type
 *		where key is located at 64-bit address.
 *
 * @param	CrvType	Type of elliptic curve
 * @param	KeyAddr	Pointer to public key address
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_ELLIPTIC_INVALID_PARAM  On invalid argument
 *		 - XSECURE_ELLIPTIC_NON_SUPPORTED_CRV  If curve data pointer is NULL
 *		 - XSECURE_ELLIPTIC_KEY_ZERO  When Public key is zero
 *		 - XSECURE_ELLIPTIC_KEY_WRONG_ORDER  Wrong order of Public key
 *		 - XSECURE_ELLIPTIC_KEY_NOT_ON_CRV  When Key is not found on the curve
 *		 - XST_FAILURE  On any other failures
 *
 *****************************************************************************/
int XSecure_EllipticValidateKey_64Bit(XSecure_EllipticCrvTyp CrvType,
		const XSecure_EllipticKeyAddr *KeyAddr)
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

	OffSet = XSecure_EllipticValidateAndGetCrvInfo(CrvType, &Crv);
	if ((OffSet == 0U) || (Crv == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
		goto END;
	}

	if (KeyAddr == NULL) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	Size = OffSet;
	if (CrvType == XSECURE_ECC_NIST_P521) {
		OffSet += XSECURE_ECDSA_P521_ALIGN_BYTES;
	}

	/* Store Pub key(Qx,Qy) to local buffer */
	XSecure_PutData(Size, (u8 *)PubKey, KeyAddr->Qx);
	XSecure_PutData(Size, (u8 *)(PubKey + OffSet), KeyAddr->Qy);

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + OffSet);

	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);

	/** Validate the public key for a given curve type */
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

END:
	XSecure_SetReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function validates the public key for a given curve type
 *
 * @param	CrvType	Type of elliptic curve
 * @param	Key	Pointer to the public key
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_ELLIPTIC_INVALID_PARAM  On invalid argument
 *
 *****************************************************************************/
int XSecure_EllipticValidateKey(XSecure_EllipticCrvTyp CrvType,
	const XSecure_EllipticKey *Key)
{
	volatile int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	XSecure_EllipticKeyAddr KeyAddr;

	if ((CrvType != XSECURE_ECC_NIST_P384) &&
		(CrvType != XSECURE_ECC_NIST_P521) &&
		(CrvType != XSECURE_ECC_NIST_P256) &&
		(CrvType != XSECURE_ECC_NIST_P192) &&
		(CrvType != XSECURE_ECC_NIST_P224)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	if (Key == NULL) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	KeyAddr.Qx = (u64)(UINTPTR)Key->Qx;
	KeyAddr.Qy = (u64)(UINTPTR)Key->Qy;

	/** Validate the public key for a given curve type */
	Status = XSecure_EllipticValidateKey_64Bit(CrvType,
			(XSecure_EllipticKeyAddr *) &KeyAddr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function verifies the signature for a given hash, key and
 *		curve type where data is located at 64-bit address
 *
 * @param	CrvType		Type of elliptic curve
 * @param	HashInfo	Pointer to Hash Data i.e. Hash Address and length
 * @param	KeyAddr		Pointer to public key address
 * @param	SignAddr	Pointer to signature address
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_ELLIPTIC_INVALID_PARAM  On invalid argument
 *		 - XSECURE_ELLIPTIC_NON_SUPPORTED_CRV  If curve data pointer is NULL
 *		 - XSECURE_ELLIPTIC_BAD_SIGN  When signature provided for verification is bad
 *		 - XSECURE_ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN  Incorrect hash length
 *						for sign verification
 *		 - XSECURE_ELLIPTIC_VER_SIGN_R_ZERO  R set to zero
 *		 - XSECURE_ELLIPTIC_VER_SIGN_S_ZERO  S set to zero
 *		 - XSECURE_ELLIPTIC_VER_SIGN_R_ORDER_ERROR  R is not within ECC order
 *		 - XSECURE_ELLIPTIC_VER_SIGN_S_ORDER_ERROR  S is not within ECC order
 *		 - XST_FAILURE  On any other failures
 *
 *****************************************************************************/
int XSecure_EllipticVerifySign_64Bit(XSecure_EllipticCrvTyp CrvType,
	const XSecure_EllipticHashData *HashInfo, const XSecure_EllipticKeyAddr *KeyAddr,
	const XSecure_EllipticSignAddr *SignAddr)
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

	if ((HashInfo == NULL) || (KeyAddr == NULL) || (SignAddr == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	HashLenTmp = HashInfo->Len;
	if ((HashInfo->Len > XSECURE_ECC_P521_SIZE_IN_BYTES) ||
		(HashLenTmp > XSECURE_ECC_P521_SIZE_IN_BYTES)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	OffSet = XSecure_EllipticValidateAndGetCrvInfo(CrvType, &Crv);
	if ((OffSet == 0U) || (Crv == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
		goto END;
	}

	Size = OffSet;
	if (CrvType == XSECURE_ECC_NIST_P521) {
		OffSet += XSECURE_ECDSA_P521_ALIGN_BYTES;
	}

	Status = XST_FAILURE;
	Status = Xil_SMemSet(PaddedHash, XSECURE_ECC_P521_SIZE_IN_BYTES,
				0U, XSECURE_ECC_P521_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Store Pub key(Qx,Qy) and Sign(SignR, SignS) to local buffers */
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

	/** Verify signature with provided hash, public key and curve type */
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
 * @param	CrvType	Type of elliptic curve
 * @param	Hash	Pointer to the hash for which sign has to be generated
 * @param	HashLen	Length of hash in bytes
 * @param	Key	Pointer to the public key
 * @param	Sign	Pointer to the signature
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_ELLIPTIC_INVALID_PARAM  On invalid argument
 *
 *****************************************************************************/
int XSecure_EllipticVerifySign(XSecure_EllipticCrvTyp CrvType, const u8 *Hash,
	const u32 HashLen, const XSecure_EllipticKey *Key, const XSecure_EllipticSign *Sign)
{
	volatile int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	XSecure_EllipticSignAddr SignAddr;
	XSecure_EllipticKeyAddr KeyAddr;
	XSecure_EllipticHashData HashInfo;

	if ((CrvType != XSECURE_ECC_NIST_P384) &&
		(CrvType != XSECURE_ECC_NIST_P521) &&
		(CrvType != XSECURE_ECC_NIST_P256) &&
		(CrvType != XSECURE_ECC_NIST_P192) &&
		(CrvType != XSECURE_ECC_NIST_P224)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	if ((Hash == NULL) || (Key == NULL) || (Sign == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	if (HashLen > XSECURE_ECC_P521_SIZE_IN_BYTES) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	SignAddr.SignR = (u64)(UINTPTR)Sign->SignR;
	SignAddr.SignS = (u64)(UINTPTR)Sign->SignS;

	KeyAddr.Qx = (u64)(UINTPTR)Key->Qx;
	KeyAddr.Qy = (u64)(UINTPTR)Key->Qy;

	HashInfo.Addr = (u64)(UINTPTR)Hash;
	HashInfo.Len = HashLen;

	/** Verify signature with provided hash, public key and curve type */
	Status = XSecure_EllipticVerifySign_64Bit(CrvType,
			(XSecure_EllipticHashData *) &HashInfo,
			(XSecure_EllipticKeyAddr *) &KeyAddr,
			(XSecure_EllipticSignAddr *) &SignAddr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function gets the curve related information
 *
 * @param	CrvTyp	Type of the elliptic curve
 *
 * @return
 *		 - Crv  Curve information
 *
 *****************************************************************************/
EcdsaCrvInfo* XSecure_EllipticGetCrvData(XSecure_EllipticCrvTyp CrvTyp)
{
	u32 Index;
	EcdsaCrvInfo *Crv = NULL;
	u32 TotalCurves = XSecure_EllipticCrvsGetCount();

	if ((CrvTyp != XSECURE_ECC_NIST_P384) &&
		(CrvTyp != XSECURE_ECC_NIST_P521) &&
		(CrvTyp != XSECURE_ECC_NIST_P256) &&
		(CrvTyp != XSECURE_ECC_NIST_P192) &&
		(CrvTyp != XSECURE_ECC_NIST_P224)) {
		goto END;
	}

	/** Get the curve data */
	for (Index = 0U; Index < TotalCurves; Index++) {
		if (XSecure_EllipticCrvsDb[Index].CrvType == (EcdsaCrvTyp)CrvTyp) {
			Crv = &XSecure_EllipticCrvsDb[Index];
			break;
		}
	}

END:
	return Crv;
}

/*****************************************************************************/
/**
 * @brief	This function copies data from 32/64 bit address to
 *		local buffer.
 *
 * @param	Size	Length of data in bytes
 * @param	Dst	Pointer to the destination buffer
 * @param	SrcAddr	Source address
 *
 *****************************************************************************/
void XSecure_PutData(const u32 Size, u8 *Dst, const u64 SrcAddr)
{
	u32 Index = 0U;
	s32 RIndex = (s32)Size - 1;

	/**
	 * Copies data from provided address to local buffer based on the
	 * endianness configured
	 */
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
 * @param	Size	Length of data in bytes
 * @param	Src	Pointer to the source buffer
 * @param	DstAddr	Destination address
 *
 *****************************************************************************/
void XSecure_GetData(const u32 Size, const u8 *Src, const u64 DstAddr)
{
	u32 Index = 0U;
	s32 RIndex = (s32)Size - 1;

	/**
	 * Copies data to provided address from local buffer based on the
	 * endianness configured
	 */
	for (Index = 0U; (Index < Size) && (RIndex >= 0); Index++, RIndex--) {
		if (XSECURE_ELLIPTIC_ENDIANNESS == XSECURE_ELLIPTIC_LITTLE_ENDIAN) {
			XSecure_OutByte64((DstAddr + Index), Src[Index]);
		}
		else {
			XSecure_OutByte64((DstAddr + Index), Src[RIndex]);
		}
	}
}

/*****************************************************************************/
/**
 * @brief	This function copies data to destination based on library
 *		endianness selection.
 *		- Changes the endianness when library is operating in little endian
 *		- Copies data without changing any endianness when library is
 *		operating in big endain.
 *
 * @param	Size	Length of data in bytes
 * @param	SrcAddr	Address of the source buffer
 * @param	DstAddr	Destination address
 *
 * @note	This is the helper function to convert the endianness as required.
 *
 *****************************************************************************/
void XSecure_FixEndiannessNCopy(const u32 Size, u64 DstAddr, const u64 SrcAddr)
{
	u32 Index = 0U;
	u32 RIndex = Size;

	for (Index = 0U; Index < Size; Index++, RIndex--) {
		if (XSECURE_ELLIPTIC_ENDIANNESS == XSECURE_ELLIPTIC_LITTLE_ENDIAN) {
			XSecure_OutByte64((DstAddr + Index), XSecure_InByte64((SrcAddr + (RIndex - 1U))));
		}
		else {
			XSecure_OutByte64((DstAddr + Index), XSecure_InByte64((SrcAddr + Index)));
		}
	}
}

/*****************************************************************************/
/**
 * @brief	This function validates and gets curve info and curve size in bytes
 *
 * @param	CrvType	Is a type of elliptic curve
 * @param	Crv	Pointer to EcdsaCrvInfo
 *
 * @return
 *		 - CrvSize  Size of curve in bytes
 *
 *****************************************************************************/
static u32 XSecure_EllipticValidateAndGetCrvInfo(XSecure_EllipticCrvTyp CrvType,
	EcdsaCrvInfo** Crv)
{
	u32 CrvSize = 0U;
	EcdsaCrvInfo* CrvInfo = (EcdsaCrvInfo *)XSecure_EllipticGetCrvData(CrvType);

	if (CrvInfo != NULL) {
		CrvSize = (u32)CrvInfo->Bits / XSECURE_ECDSA_BITS_IN_BYTES;
		CrvSize += (CrvSize % XSECURE_ECDSA_ALGN_CRV_SIZE_IN_BYTES);
		*Crv = CrvInfo;
	}

	return CrvSize;
}

/*****************************************************************************/
/**
 * @brief	This function gets curve size in bytes
 *
 * @param	CrvType	Is a type of elliptic curve
 *
 * @return
 *		 - CrvSize  Size of curve in bytes
 *
 *****************************************************************************/
u32 XSecure_EllipticGetCrvSize(const XSecure_EllipticCrvTyp CrvType)
{
	u32 CrvSize = 0U;
	const EcdsaCrvInfo* CrvInfo = (EcdsaCrvInfo *)XSecure_EllipticGetCrvData(CrvType);

	/** Get curve size */
	if (CrvInfo != NULL) {
		CrvSize = (u32)CrvInfo->Bits / XSECURE_ECDSA_BITS_IN_BYTES;
		CrvSize += (CrvSize % XSECURE_ECDSA_ALGN_CRV_SIZE_IN_BYTES);
	}

	return CrvSize;
}

#endif
/** @} */
