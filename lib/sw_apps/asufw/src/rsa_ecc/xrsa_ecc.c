/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xrsa_ecc.c
* @addtogroup Overview
* @{
* This file contains implementation of the interface functions for RSA hardware engine.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   yog  07/11/24 Initial release
*       yog  08/19/24 Received Dma instance from handler
*       yog  08/25/24 Integrated FIH library
*
* </pre>
*
**************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "Rsa.h"
#include "xrsa_ecc.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasu_eccinfo.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/
#define XRSA_ECC_BITS_IN_BYTES			(8U)	/**< Bits in bytes */
#define XRSA_ECC_ALGN_CRV_SIZE_IN_BYTES		(2U)	/**< Align ECDSA curve size in bytes */

/**< Return value in case of success */
#define XRSA_ECC_SUCCESS			ELLIPTIC_SUCCESS

/**< Validate Public Key error codes */
#define XRSA_ECC_KEY_ZERO			ELLIPTIC_KEY_ZERO
#define XRSA_ECC_KEY_WRONG_ORDER		ELLIPTIC_KEY_WRONG_ORDER
#define XRSA_ECC_KEY_NOT_ON_CRV			ELLIPTIC_KEY_NOT_ON_CRV

/**< Verify Sign error codes */
#define XRSA_ECC_BAD_SIGN			ELLIPTIC_BAD_SIGN
#define XRSA_ECC_VER_SIGN_INCORRECT_HASH_LEN	ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN
#define XRSA_ECC_VER_SIGN_R_ZERO		ELLIPTIC_VER_SIGN_R_ZERO
#define XRSA_ECC_VER_SIGN_S_ZERO		ELLIPTIC_VER_SIGN_S_ZERO
#define XRSA_ECC_VER_SIGN_R_ORDER_ERROR		ELLIPTIC_VER_SIGN_R_ORDER_ERROR
#define XRSA_ECC_VER_SIGN_S_ORDER_ERROR		ELLIPTIC_VER_SIGN_S_ORDER_ERROR

/**< Generate sign error codes */
#define XRSA_ECC_GEN_SIGN_BAD_R			ELLIPTIC_GEN_SIGN_BAD_R
#define XRSA_ECC_GEN_SIGN_BAD_S			ELLIPTIC_GEN_SIGN_BAD_S
#define XRSA_ECC_GEN_SIGN_INCORRECT_HASH_LEN	ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static u32 XRsa_EccValidateAndGetCrvInfo(u32 CurveType, EcdsaCrvInfo **Crv);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function generates public key for a given curve type using	private key.
 *
 * @param	DmaPtr		Pointer to DMA instance.
 * @param	CurveType	ECC curve type.
 * @param	PrivKeyAddr	Address of private key.
 *				Length of the private key shall be CurveLen.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PubKeyAddr	Address of public key.
 *				Length of the public key shall be double of CurveLen since
 *				public key has both Qx, Qy components.
 *
 * @return
 *	-	XASUFW_SUCCESS, On success
 *	-	XASUFW_RSA_ECC_INVALID_PARAM, if received parameter is invalid
 *	-	XASUFW_RSA_ECC_WRITE_DATA_FAIL, if write data through DMA fails
 *	-	XASUFW_RSA_ECC_GEN_PUB_KEY_OPERATION_FAIL, if public key generation operation fails
 *	-	XASUFW_RSA_ECC_READ_DATA_FAIL, if read data through DMA fails
 *
 *************************************************************************************************/
s32 XRsa_EccGeneratePubKey(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PrivKeyAddr,
			   u64 PubKeyAddr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	s32 ClearStatus = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	s32 ClearStatusTmp = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	u32 CurveSize = 0U;
	u8 PrivKey[XRSA_ECC_P521_SIZE_IN_BYTES];
	u8 PubKey[XRSA_ECC_P521_SIZE_IN_BYTES + XRSA_ECC_P521_SIZE_IN_BYTES];
	EcdsaKey Key;
	EcdsaCrvInfo *Crv = NULL;

	/* Validate the input arguments */
	if ((PrivKeyAddr == 0U) || (PubKeyAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (Crv == NULL)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if (CurveLen != CurveSize) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/* Enable endianness for write and read operations */
	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	/* Store private key to local address by changing endianness */
	Status = XAsufw_DmaXfr(DmaPtr, PrivKeyAddr, (u64)(UINTPTR)PrivKey, CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END_CLR);
	}

	Status = XAsufw_ChangeEndianness(PrivKey, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END_CLR);
	}

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + CurveLen);

	Status = XASUFW_FAILURE;
	/** Generate public key with provided private key and curve type */
	XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Ecdsa_GeneratePublicKey,
					   XASUFW_RSA_ECC_GEN_PUB_KEY_OPERATION_FAIL, XFihVar, Status, END_CLR,
					   Crv, PrivKey, (EcdsaKey *)&Key);

	/* Store public key to destination address after endianness change */
	Status = XAsufw_ChangeEndianness(PubKey, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END_CLR);
	}

	Status = XAsufw_ChangeEndianness((PubKey + CurveLen), CurveLen);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END_CLR);
	}

	XFIH_CALL_WITH_SPECIFIC_ERROR(XAsufw_DmaXfr, XASUFW_RSA_ECC_READ_DATA_FAIL, XFihVar, Status,
				      DmaPtr, (u64)(UINTPTR)PubKey, PubKeyAddr,
				      XAsu_DoubleCurveLength(CurveLen), 0U);

END_CLR:
	/* Zeroize local key copy */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PrivKey, XRSA_ECC_P521_SIZE_IN_BYTES);
	ClearStatusTmp = Xil_SecureZeroize((u8 *)(UINTPTR)PrivKey, XRSA_ECC_P521_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, (ClearStatus  | ClearStatusTmp));

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey,
					XAsu_DoubleCurveLength(XRSA_ECC_P521_SIZE_IN_BYTES));
	ClearStatusTmp = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey,
					   XAsu_DoubleCurveLength(XRSA_ECC_P521_SIZE_IN_BYTES));
	Status = XAsufw_UpdateBufStatus(Status, (ClearStatus  | ClearStatusTmp));
END:
	XAsufw_CryptoCoreSetReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates public key for a given curve type.
 *
 * @param	DmaPtr		Pointer to DMA instance.
 * @param	CurveType	ECC curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PubKeyAddr	Address of public key.
 *				Length of the public key shall be double of CurveLen since
 *				public key has both Qx, Qy components.
 *
 *
 * @return
 *	-	XASUFW_SUCCESS, On success
 *	-	XASUFW_RSA_ECC_INVALID_PARAM, if received parameter is invalid
 *	-	XASUFW_RSA_ECC_WRITE_DATA_FAIL, if write data through DMA fails
 *	-	XASUFW_RSA_ECC_READ_DATA_FAIL, if read data through DMA fails
 *	-	XASUFW_RSA_ECC_PUBLIC_KEY_ZERO, if pubic key is zero
 *	-	XASUFW_RSA_ECC_PUBLIC_KEY_WRONG_ORDER, if public key is in wrong order
 *	-	XASUFW_RSA_ECC_PUBLIC_KEY_NOT_ON_CRV, if public key is not on curve
 *	-	XASUFW_FAILURE, if validation fails due to other reasons
 *
 *************************************************************************************************/
s32 XRsa_EccValidatePubKey(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PubKeyAddr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u32 CurveSize = 0U;
	s32 ClearStatus = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	s32 ClearStatusTmp = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	u8 PubKey[XRSA_ECC_P521_SIZE_IN_BYTES + XRSA_ECC_P521_SIZE_IN_BYTES];
	EcdsaKey Key;
	EcdsaCrvInfo *Crv = NULL;

	/* Validate the input arguments */
	if (PubKeyAddr == 0U) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (Crv == NULL)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if (CurveLen != CurveSize) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/* Store public key to local address by changing endianness */
	Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr, (u64)(UINTPTR)PubKey,
			       XAsu_DoubleCurveLength(CurveLen), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END_CLR);
	}

	Status = XAsufw_ChangeEndianness(PubKey, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END_CLR);
	}

	Status = XAsufw_ChangeEndianness((PubKey + CurveLen), CurveLen);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END_CLR);
	}

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + CurveLen);

	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	Status = XASUFW_FAILURE;
	/** Generate public key with provided private key and curve type */
	XFIH_CALL(Ecdsa_ValidateKey, XFihVar, Status, Crv, (EcdsaKey *)&Key);
	if (Status == XRSA_ECC_KEY_ZERO) {
		Status = XASUFW_RSA_ECC_PUBLIC_KEY_ZERO;
	} else if (Status == XRSA_ECC_KEY_WRONG_ORDER) {
		Status = XASUFW_RSA_ECC_PUBLIC_KEY_WRONG_ORDER;
	} else if (Status == XRSA_ECC_KEY_NOT_ON_CRV) {
		Status = XASUFW_RSA_ECC_PUBLIC_KEY_NOT_ON_CRV;
	} else if (Status != XRSA_ECC_SUCCESS) {
		Status = XASUFW_FAILURE;
	} else {
		Status = XASUFW_SUCCESS;
	}

END_CLR:
	/* Zeroize local key copy */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey,
					XAsu_DoubleCurveLength(XRSA_ECC_P521_SIZE_IN_BYTES));
	ClearStatusTmp = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey,
					   XAsu_DoubleCurveLength(XRSA_ECC_P521_SIZE_IN_BYTES));
	Status = XAsufw_UpdateBufStatus(Status, (ClearStatus  | ClearStatusTmp));

END:
	XAsufw_CryptoCoreSetReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates signature for a given curve type, hash and private key.
 *
 * @param	DmaPtr		Pointer to DMA instance.
 * @param	CurveType	ECC curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PrivKeyAddr	Address of the private key.
 *				Length of the private key shall be CurveLen.
 * @param	EphemeralKeyPtr	Pointer to ephemeral key.
 *				Length of the ephemeral key shall be CurveLen.
 * @param	HashAddr	Address of hash to which signature is to be generated.
 * @param	HashBufLen	Length of the hash in bytes.
 * @param	SignAddr	Address to store the generated Signature.
 *				Length of the signature shall be double of CurveLen since sign
 *				has both r, s components.
 *
 * @return
 *	-	XASUFW_SUCCESS, On successful operation
 *	-	XASUFW_RSA_ECC_INVALID_PARAM, if any of the input parameters are invalid
 *	-	XASUFW_RSA_ECC_WRITE_DATA_FAIL, if write data through DMA fails
 *	-	XASUFW_RSA_ECC_READ_DATA_FAIL, if read data through DMA fails
 *	-	XASUFW_RSA_ECC_EPHEMERAL_KEY_GEN_FAIL, if ephemeral key generation fails
 *	-	XASUFW_RSA_ECC_GEN_SIGN_BAD_RAND_NUM, if bad random number used for sign generation
 *	-	XASUFW_RSA_ECC_GEN_SIGN_INCORRECT_HASH_LEN, if incorrect hash length is provided
 *					for sign generation
 *	-	XASUFW_FAILURE, if sign generation fails due to other reasons
 *
 *************************************************************************************************/
s32 XRsa_EccGenerateSignature(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PrivKeyAddr,
			      const u8 *EphemeralKeyPtr, u64 HashAddr, u32 HashBufLen, u64 SignAddr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	s32 ClearStatus = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	s32 ClearStatusTmp = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	u32 CurveSize = 0U;
	u8 PrivKey[XRSA_ECC_P521_SIZE_IN_BYTES];
	u8 Signature[XRSA_ECC_P521_SIZE_IN_BYTES + XRSA_ECC_P521_SIZE_IN_BYTES];
	u8 Hash[XRSA_ECC_P521_SIZE_IN_BYTES];
	u8 EphemeralKey[XRSA_ECC_P521_SIZE_IN_BYTES];
	EcdsaSign Sign;
	EcdsaCrvInfo *Crv = NULL;

	/* Validate the input arguments */
	if ((HashAddr == 0U) || (PrivKeyAddr == 0U) || (SignAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (Crv == NULL)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if ((CurveLen != CurveSize) || (HashBufLen != CurveSize)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/* Store inputs from 64/32 bit address to local buffer
	 * Store ephemeral key to local address by changing endianness
	 */
	Status = Xil_SMemCpy((u8 *)EphemeralKey, CurveLen, (u8 *)EphemeralKeyPtr, CurveLen, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END_CLR);
	}
	Status = XAsufw_ChangeEndianness(EphemeralKey, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END_CLR);
	}

	/* Store private key to local address by changing endianness */
	Status = XAsufw_DmaXfr(DmaPtr, PrivKeyAddr, (u64)(UINTPTR)PrivKey, CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END_CLR);
	}
	Status = XAsufw_ChangeEndianness(PrivKey, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END_CLR);
	}

	/* Store Hash to local address by changing endianness */
	Status = XAsufw_DmaXfr(DmaPtr, HashAddr, (u64)(UINTPTR)Hash, HashBufLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END_CLR);
	}
	Status = XAsufw_ChangeEndianness(Hash, HashBufLen);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END_CLR);
	}

	Sign.r = (u8 *)(UINTPTR)Signature;
	Sign.s = (u8 *)(UINTPTR)(Signature + CurveLen);

	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	Status = XASUFW_FAILURE;
	XFIH_CALL(Ecdsa_GenerateSign, XFihVar, Status, Crv, Hash, Crv->Bits, PrivKey, EphemeralKey,
		  (EcdsaSign *)&Sign);
	if ((Status == XRSA_ECC_GEN_SIGN_BAD_R) ||
	    (Status == XRSA_ECC_GEN_SIGN_BAD_S)) {
		Status = XASUFW_RSA_ECC_GEN_SIGN_BAD_RAND_NUM;
	} else if ((Status == XRSA_ECC_GEN_SIGN_INCORRECT_HASH_LEN)) {
		Status = XASUFW_RSA_ECC_GEN_SIGN_INCORRECT_HASH_LEN;
	} else if ((Status != XRSA_ECC_SUCCESS)) {
		Status = XASUFW_FAILURE;
	} else {
		/* Store Sign to destination address after changing endianness */
		Status = XAsufw_ChangeEndianness(Signature, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END_CLR);
		}
		Status = XAsufw_ChangeEndianness((Signature + CurveLen), CurveLen);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END_CLR);
		}

		XFIH_CALL_WITH_SPECIFIC_ERROR(XAsufw_DmaXfr, XASUFW_RSA_ECC_READ_DATA_FAIL,
					      XFihVar, Status, DmaPtr, (u64)(UINTPTR)Signature, SignAddr,
					      XAsu_DoubleCurveLength(CurveLen), 0U);
	}

END_CLR:
	/* Zeroize local key copy */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PrivKey, XRSA_ECC_P521_SIZE_IN_BYTES);
	ClearStatusTmp = Xil_SecureZeroize((u8 *)(UINTPTR)PrivKey, XRSA_ECC_P521_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, (ClearStatus  | ClearStatusTmp));

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)EphemeralKey, XRSA_ECC_P521_SIZE_IN_BYTES);
	ClearStatusTmp = Xil_SecureZeroize((u8 *)(UINTPTR)EphemeralKey, XRSA_ECC_P521_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, (ClearStatus  | ClearStatusTmp));

END:
	XAsufw_CryptoCoreSetReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function verifies signature for a given curve type.
 *
 * @param	DmaPtr		Pointer to DMA instance.
 * @param	CurveType	ECC Curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PubKeyAddr	Address of the public key.
 *				Length of the public key shall be double of CurveLen since
 *				public key has both Qx, Qy components.
 * @param	HashAddr	Address of hash.
 * @param	HashBufLen	Length of hash in bytes.
 * @param	SignAddr	Address of the signature to be verified.
 *				Length of the signature shall be double of CurveLen since sign
 *				has both r, s components.
 *
 * @return
 *	-	XASUFW_SUCCESS, On successful operation
 *	-	XASUFW_RSA_ECC_INVALID_PARAM, if any of the input parameters are invalid
 *	-	XASUFW_RSA_ECC_WRITE_DATA_FAIL, if write data through DMA fails
 *	-	XASUFW_RSA_ECC_READ_DATA_FAIL, if read data through DMA fails
 *	-	XASUFW_RSA_ECC_BAD_SIGN, if signature provided for verification is bad
 *	-	XASUFW_RSA_ECC_VER_SIGN_INCORRECT_HASH_LEN, if incorrect hash length for sign
 *						verification
 *	-	XASUFW_RSA_ECC_VER_SIGN_R_ZERO, if provided R is zero
 *	-	XASUFW_RSA_ECC_VER_SIGN_S_ZERO, if provided S is zero
 *	-	XASUFW_RSA_ECC_VER_SIGN_R_ORDER_ERROR, if R is not within ECC order
 *	-	XASUFW_RSA_ECC_VER_SIGN_S_ORDER_ERROR, if S is not within ECC order
 *	-	XASUFW_FAILURE, if operation fails due to any other reasons
 *
 *************************************************************************************************/
s32 XRsa_EccVerifySignature(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PubKeyAddr,
			    u64 HashAddr, u32 HashBufLen, u64 SignAddr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	s32 ClearStatus = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	s32 ClearStatusTmp = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	u32 CurveSize = 0U;
	u8 PubKey[XRSA_ECC_P521_SIZE_IN_BYTES + XRSA_ECC_P521_SIZE_IN_BYTES];
	u8 Signature[XRSA_ECC_P521_SIZE_IN_BYTES + XRSA_ECC_P521_SIZE_IN_BYTES];
	u8 Hash[XRSA_ECC_P521_SIZE_IN_BYTES];
	EcdsaSign Sign;
	EcdsaKey Key;
	EcdsaCrvInfo *Crv = NULL;

	/* Validate the input arguments */
	if ((HashAddr == 0U) || (PubKeyAddr == 0U) || (SignAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (Crv == NULL)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if ((CurveLen != CurveSize) || (HashBufLen != CurveSize)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/* Store inputs from 64/32 bit address to local buffer
	 * Store hash to local address by changing endianness
	 */
	Status = XAsufw_DmaXfr(DmaPtr, HashAddr, (u64)(UINTPTR)Hash, HashBufLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END_CLR);
	}
	Status = XAsufw_ChangeEndianness(Hash, HashBufLen);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END_CLR);
	}

	/* Store public key to local address by changing endianness */
	Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr, (u64)(UINTPTR)PubKey,
			       XAsu_DoubleCurveLength(CurveLen), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END_CLR);
	}
	Status = XAsufw_ChangeEndianness(PubKey, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END_CLR);
	}
	Status = XAsufw_ChangeEndianness((PubKey + CurveLen), CurveLen);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END_CLR);
	}

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + CurveLen);

	/* Store signature to local address by changing endianness */
	Status = XAsufw_DmaXfr(DmaPtr, SignAddr, (u64)(UINTPTR)Signature,
			       XAsu_DoubleCurveLength(CurveLen), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END_CLR);
	}
	Status = XAsufw_ChangeEndianness(Signature, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END_CLR);
	}
	Status = XAsufw_ChangeEndianness((Signature + CurveLen), CurveLen);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END_CLR);
	}

	Sign.r = (u8 *)(UINTPTR)Signature;
	Sign.s = (u8 *)(UINTPTR)(Signature + CurveLen);

	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	Status = XASUFW_FAILURE;
	XFIH_CALL(Ecdsa_VerifySign, XFihVar, Status, Crv, Hash, Crv->Bits, (EcdsaKey *)&Key,
		  (EcdsaSign *)&Sign);
	if ((XRSA_ECC_BAD_SIGN == Status)) {
		Status = XASUFW_RSA_ECC_BAD_SIGN;
	} else if ((XRSA_ECC_VER_SIGN_INCORRECT_HASH_LEN == Status)) {
		Status = XASUFW_RSA_ECC_VER_SIGN_INCORRECT_HASH_LEN;
	} else if ((XRSA_ECC_VER_SIGN_R_ZERO == Status)) {
		Status = XASUFW_RSA_ECC_VER_SIGN_R_ZERO;
	} else if ((XRSA_ECC_VER_SIGN_S_ZERO == Status)) {
		Status = XASUFW_RSA_ECC_VER_SIGN_S_ZERO;
	} else if ((XRSA_ECC_VER_SIGN_R_ORDER_ERROR == Status)) {
		Status = XASUFW_RSA_ECC_VER_SIGN_R_ORDER_ERROR;
	} else if ((XRSA_ECC_VER_SIGN_S_ORDER_ERROR == Status)) {
		Status = XASUFW_RSA_ECC_VER_SIGN_S_ORDER_ERROR;
	} else if ((XRSA_ECC_SUCCESS != Status)) {
		Status = XASUFW_FAILURE;
	} else {
		Status = XASUFW_SUCCESS;
	}
END_CLR:
	/* Zeroize local key copy */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey,
					XAsu_DoubleCurveLength(XRSA_ECC_P521_SIZE_IN_BYTES));
	ClearStatusTmp = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey,
					   XAsu_DoubleCurveLength(XRSA_ECC_P521_SIZE_IN_BYTES));
	Status = XAsufw_UpdateBufStatus(Status, (ClearStatus  | ClearStatusTmp));

END:
	XAsufw_CryptoCoreSetReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function gets the curve related information
 *
 * @param	CurveType	ECC Curve type.
 *
 * @return
 *		Crv - Curve information
 *
 *************************************************************************************************/
EcdsaCrvInfo *XRsa_EccGetCrvData(u32 CurveType)
{
	u32 Index;
	EcdsaCrvInfo *Crv = NULL;
	u32 TotalCurves = XRsa_EccCrvsGetCount();

	if ((CurveType != XRSA_ECC_CURVE_TYPE_NIST_P521) &&
	    (CurveType != XRSA_ECC_CURVE_TYPE_NIST_P192) &&
	    (CurveType != XRSA_ECC_CURVE_TYPE_NIST_P224) &&
	    (CurveType != XRSA_ECC_CURVE_TYPE_BRAINPOOL_P256) &&
	    (CurveType != XRSA_ECC_CURVE_TYPE_BRAINPOOL_P320) &&
	    (CurveType != XRSA_ECC_CURVE_TYPE_BRAINPOOL_P384) &&
	    (CurveType != XRSA_ECC_CURVE_TYPE_BRAINPOOL_P512)) {
		XFIH_GOTO(END);
	}

	for (Index = 0U; Index < TotalCurves; Index++) {
		if (XRsa_EccCrvsDb[Index].CrvType == (EcdsaCrvTyp)CurveType) {
			Crv = &XRsa_EccCrvsDb[Index];
			break;
		}
	}

END:
	return Crv;
}

/*************************************************************************************************/
/**
 * @brief	This function validates and gets curve info and curve size in bytes
 *
 * @param	CrvType	Is a type of elliptic curve
 * @param	Crv	Pointer to EcdsaCrvInfo
 *
 * @return
 *		CurveSize - Size of curve in bytes
 *
 *************************************************************************************************/
static u32 XRsa_EccValidateAndGetCrvInfo(u32 CurveType, EcdsaCrvInfo **Crv)
{
	u32 CurveSize = 0U;
	EcdsaCrvInfo *CrvInfo = XRsa_EccGetCrvData(CurveType);

	if (CrvInfo != NULL) {
		CurveSize = (u32)CrvInfo->Bits / XRSA_ECC_BITS_IN_BYTES;
		CurveSize += (CurveSize % XRSA_ECC_ALGN_CRV_SIZE_IN_BYTES);
		*Crv = CrvInfo;
	}

	return CurveSize;
}
/** @} */
