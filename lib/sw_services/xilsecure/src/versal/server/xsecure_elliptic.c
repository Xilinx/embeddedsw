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

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XSECURE_ECDSA_KAT_NIST_P384	0U
#define XSECURE_ECDSA_KAT_NIST_P521	2U

/************************** Function Prototypes ******************************/
static EcdsaCrvInfo* XSecure_EllipticGetCrvData(XSecure_EllipticCrvTyp CrvTyp);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
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
	int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	EcdsaCrvInfo *Crv = NULL;

	if ((CrvType != XSECURE_ECC_NIST_P384) &&
			(CrvType != XSECURE_ECC_NIST_P521)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	if ((D == NULL) || (Key == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);

	Crv = XSecure_EllipticGetCrvData(CrvType);
	if(Crv != NULL) {
		Status = Ecdsa_GeneratePublicKey(Crv, D, (EcdsaKey *)Key);
		if (Status != XST_SUCCESS) {
			Status = (int)XSECURE_ELLIPTIC_GEN_KEY_ERR;
		}
	}

END:
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
int XSecure_EllipticGenerateSignature(XSecure_EllipticCrvTyp CrvType, const u8* Hash,
	const u32 HashLen, const u8* D, const u8* K, XSecure_EllipticSign *Sign)
{
	int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	volatile int GenStatus = XST_FAILURE;
	EcdsaCrvInfo *Crv = NULL;
	u8 PaddedHash[XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	volatile u32 HashLenTmp = 0xFFFFFFFFU;

	if ((CrvType != XSECURE_ECC_NIST_P384) &&
			(CrvType != XSECURE_ECC_NIST_P521)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	if ((D == NULL) || (K == NULL) || (Hash == NULL) || (Sign == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	HashLenTmp = HashLen;
	if ((HashLen > XSECURE_ECC_P521_SIZE_IN_BYTES) ||
		(HashLenTmp > XSECURE_ECC_P521_SIZE_IN_BYTES)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);

	Crv = XSecure_EllipticGetCrvData(CrvType);
	if(Crv != NULL) {
		Xil_MemCpy(PaddedHash, Hash, HashLen);
		XSECURE_TEMPORAL_CHECK(SIG_ERR, GenStatus, Ecdsa_GenerateSign,
			Crv, PaddedHash, Crv->Bits, D, K, (EcdsaSign *)Sign);

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
	int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	volatile int ValidateStatus = XST_FAILURE;
	EcdsaCrvInfo *Crv = NULL;

	if ((CrvType != XSECURE_ECC_NIST_P384) &&
		(CrvType != XSECURE_ECC_NIST_P521)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	if (Key == NULL) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);

	Crv = XSecure_EllipticGetCrvData(CrvType);
	if(Crv != NULL) {
		XSECURE_TEMPORAL_CHECK(KEY_ERR, ValidateStatus, Ecdsa_ValidateKey,
			Crv, (EcdsaKey *)Key);

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
	int Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
	volatile int VerifyStatus = XST_FAILURE;
	EcdsaCrvInfo *Crv = NULL;
	u8 PaddedHash[XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	volatile u32 HashLenTmp = 0xFFFFFFFFU;

	if ((CrvType != XSECURE_ECC_NIST_P384) && (CrvType != XSECURE_ECC_NIST_P521)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	if ((Hash == NULL) || (Key == NULL) || (Sign == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	HashLenTmp = HashLen;
	if ((HashLen > XSECURE_ECC_P521_SIZE_IN_BYTES) ||
		(HashLenTmp > XSECURE_ECC_P521_SIZE_IN_BYTES)) {
		Status = (int)XSECURE_ELLIPTIC_INVALID_PARAM;
		goto END;
	}

	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
		XSECURE_ECDSA_RSA_RESET_OFFSET);

	Crv = XSecure_EllipticGetCrvData(CrvType);
	if(Crv != NULL) {
		Xil_MemCpy(PaddedHash, Hash, HashLen);
		XSECURE_TEMPORAL_CHECK(SIG_ERR, VerifyStatus, Ecdsa_VerifySign,
			Crv, PaddedHash, Crv->Bits, (EcdsaKey *)Key, (EcdsaSign *)Sign);

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
