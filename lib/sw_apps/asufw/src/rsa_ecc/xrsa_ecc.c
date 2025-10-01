/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xrsa_ecc.c
 *
 * This file contains implementation of the interface functions for RSA hardware engine.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  07/11/24 Initial release
 *       yog  08/19/24 Received Dma instance from handler
 *       yog  08/25/24 Integrated FIH library
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *       ss   12/02/24 Added support for ECDH
 * 1.1   yog  02/21/25 Changed the API XRsa_EccValidateAndGetCrvInfo() to be non-static
 *       yog  03/21/25 Added PWCT support
 *       yog  03/24/25 Added XRsa_EccGeneratePrivKey() API
 * 1.2   am   05/20/25 Integrated performance measurement macros
 *       yog  07/11/25 Added support for Edward curves.
 *       kd   07/23/25 Fixed gcc warnings
 *       rmv  07/16/25 Update XRsa_EccGeneratePvtKey() function to add support for providing
 *                     random number as optional parameter
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xrsa_ecc_server_apis RSA ECC Server APIs
* @{
*/
/*************************************** Include Files *******************************************/
#include "Rsa.h"
#include "xrsa_ecc.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasu_eccinfo.h"
#include "xfih.h"
#include "xasufw_trnghandler.h"
#include "xsha.h"
#include "xsha_hw.h"

/************************************ Constant Definitions ***************************************/
#define XRSA_ECC_ALGN_CRV_SIZE_IN_BYTES		(2U)	/**< Align ECDSA curve size in bytes */

#define XRSA_BASEADDRESS			(0xEBF50000U) /**< RSA base address */
#define XRSA_RESET_OFFSET			(0x00000040U) /**< RSA reset offset */

#define XECDH_SHARED_SEC_OBJ_ID_SIZE		(0X4U)
						/**< Size of shared secret object ID in bytes */

/* Return value in case of success from IP Cores */
#define XRSA_ECC_SUCCESS			ELLIPTIC_SUCCESS /**< Success from IP Cores */
/* Validate Public Key error codes from IP Cores */
#define XRSA_ECC_KEY_ZERO			ELLIPTIC_KEY_ZERO
							/**< Error from IP Cores if key is zero */
#define XRSA_ECC_KEY_WRONG_ORDER		ELLIPTIC_KEY_WRONG_ORDER
							/**< Error from IP Cores if key is in wrong order */
#define XRSA_ECC_KEY_NOT_ON_CRV			ELLIPTIC_KEY_NOT_ON_CRV
							/**< Error from IP Cores if key point is not on curve */
/* Verify Sign error codes from IP Cores */
#define XRSA_ECC_BAD_SIGN			ELLIPTIC_BAD_SIGN /**< Error from IP Cores if bad sign */
#define XRSA_ECC_VER_SIGN_INCORRECT_HASH_LEN	ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN
							/**< Error from IP Cores if hash length is incorrect */
#define XRSA_ECC_VER_SIGN_R_ZERO		ELLIPTIC_VER_SIGN_R_ZERO /**< Error from IP Cores
									  * if sign R is zero */
#define XRSA_ECC_VER_SIGN_S_ZERO		ELLIPTIC_VER_SIGN_S_ZERO /**< Error from IP Cores
									  * if sign S is zero */
#define XRSA_ECC_VER_SIGN_R_ORDER_ERROR		ELLIPTIC_VER_SIGN_R_ORDER_ERROR
							/**< Error from IP Cores if sign R is in wrong order */
#define XRSA_ECC_VER_SIGN_S_ORDER_ERROR		ELLIPTIC_VER_SIGN_S_ORDER_ERROR
							/**< Error from IP Cores if sign S is in wrong order */
/* Generate sign error codes from IP Cores */
#define XRSA_ECC_GEN_SIGN_BAD_R			ELLIPTIC_GEN_SIGN_BAD_R
							/**< Error from IP Cores
							 * if generated sign has bad R. */
#define XRSA_ECC_GEN_SIGN_BAD_S			ELLIPTIC_GEN_SIGN_BAD_S
							/**< Error from IP Cores if generated sign has bad S */
#define XRSA_ECC_GEN_SIGN_INCORRECT_HASH_LEN	ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN
							/**< Error from IP Cores if hash length is incorrect */
#define XRSA_ECC_GEN_POINT_INVALID		(0x01)	/**< Error from IP Cores
							 * if generated point is invalid */

/************************************** Type Definitions *****************************************/
/** This structure contains curve type and curve class for indexing. */
typedef struct {
	u16 CurveType; /**< Type of the curve */
	u16 CurveClass; /**< Class of the curve */
} XRsa_EccCrvIndex;

/**
 * This is a configuration table to configure the curve type and curve class for all curves
 * based on index.
 */
XRsa_EccCrvIndex XRsa_EccCrvIndexDb[] = {
	{ (u16)ECDSA_NIST_P256, (u16)ECDSA_PRIME },
	{ (u16)ECDSA_NIST_P384, (u16)ECDSA_PRIME },
	{ (u16)ECDSA_NIST_P192, (u16)ECDSA_PRIME },
	{ (u16)ECDSA_NIST_P224, (u16)ECDSA_PRIME },
	{ (u16)ECDSA_NIST_P521, (u16)ECDSA_PRIME },
	{ (u16)ECDSA_BRAINPOOL_P256, (u16)ECDSA_PRIME },
	{ (u16)ECDSA_BRAINPOOL_P320, (u16)ECDSA_PRIME },
	{ (u16)ECDSA_BRAINPOOL_P384, (u16)ECDSA_PRIME },
	{ (u16)ECDSA_BRAINPOOL_P512, (u16)ECDSA_PRIME },
	{ (u16)ECDSA_ED25519, (u16)ECDSA_PRIME },
	{ (u16)ECDSA_ED448, (u16)ECDSA_PRIME },
	{ (u16)ECDSA_ED25519, (u16)ECDSA_ED_PH },
	{ (u16)ECDSA_ED448, (u16)ECDSA_ED_PH }
};

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XRsa_EccHashCalc(XAsufw_Dma *DmaPtr, u32 CurveType, u64 DataAddr, u64 HashAddr,
		u32 DataSize);

/************************************ Variable Definitions ***************************************/

#if XASUFW_ENABLE_PERF_MEASUREMENT
static u64 StartTime; /**< Performance measurement start time. */
static XAsufw_PerfTime PerfTime; /**< Structure holding performance timing results. */
#endif

/*************************************************************************************************/
/**
 * @brief	This function generates the public key using the provided private key for the
 * 		specified elliptic curve using RSA core.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	CurveType	ECC curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PrivKeyAddr	Address of the private key buffer, whose length shall be equal to CurveLen.
 * @param	PubKeyAddr	Address of the buffer to store the generated public key, whose
 * 				length shall be	equal to double of CurveLen as it contains both
 * 				Qx, Qy components.
 *
 * @return
 *	- XASUFW_SUCCESS, if public key generated successfully.
 *	- XASUFW_RSA_ECC_INVALID_PARAM, if any input parameter is invalid.
 *	- XASUFW_RSA_ECC_WRITE_DATA_FAIL, if write data through DMA fails.
 *	- XASUFW_RSA_ECC_GEN_PUB_KEY_OPERATION_FAIL, if public key generation operation fails.
 *	- XASUFW_RSA_ECC_READ_DATA_FAIL, if read data through DMA fails
 *	- XASUFW_RSA_ECC_PWCT_SIGN_GEN_FAIL, if sign generation fails in PWCT.
 *	- XASUFW_RSA_ECC_PWCT_SIGN_VER_FAIL, if sign verification fails in PWCT.
 *	- XASUFW_RSA_CHANGE_ENDIANNESS_ERROR, if endianness change fails.
 *
 *************************************************************************************************/
s32 XRsa_EccGeneratePubKey(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PrivKeyAddr,
			   u64 PubKeyAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEcc = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 CurveSize = 0U;
	u8 PrivKey[XASU_ECC_P521_SIZE_IN_BYTES];
	u8 PubKey[XASU_ECC_P521_SIZE_IN_BYTES + XASU_ECC_P521_SIZE_IN_BYTES];
	EcdsaKey Key;
	EcdsaCrvInfo *Crv = NULL;

	/** Validate the input arguments. */
	if ((DmaPtr == NULL) || (PrivKeyAddr == 0U) || (PubKeyAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (Crv == NULL)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	if (CurveLen != CurveSize) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/** Copy private key to local address using DMA and change endianness of the data. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, PrivKeyAddr, (u64)(UINTPTR)PrivKey, CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}

	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(PrivKey, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END_CLR;
		}
	}

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + CurveLen);

	/** Release RSA core reset. */
	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	/** Generate public key with provided private key and curve type. */
	XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Ecdsa_GeneratePublicKey,
					   XASUFW_RSA_ECC_GEN_PUB_KEY_OPERATION_FAIL, XFihEcc, Status, END_CLR,
					   Crv, PrivKey, (EcdsaKey *)&Key, DmaPtr);
	/**
	 * Change endianness of the generated public key and copy it to destination address
	 * using DMA for all curves except Ed25519 and Ed448.
	 */
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(PubKey, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(PubKey + CurveLen, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END_CLR;
		}
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)PubKey, PubKeyAddr,
					XAsu_DoubleCurveLength(CurveLen), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_READ_DATA_FAIL;
		goto END_CLR;
	}

	/** Validate the public key generated from the private key. */
	XFIH_CALL_GOTO(XRsa_EccValidatePubKey, XFihEcc, Status, END_CLR, DmaPtr, CurveType,
			CurveLen, PubKeyAddr);

	/** Perform pair wise consistency test using the key pair. */
	XFIH_CALL(XRsa_EccPwct, XFihEcc, Status, DmaPtr, CurveType, CurveLen, PrivKeyAddr,
		PubKeyAddr);

END_CLR:
	/** Zeroize local buffers. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcc, ClearStatus, (u8 *)(UINTPTR)PrivKey,
					XASU_ECC_P521_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey,
			XAsu_DoubleCurveLength(XASU_ECC_P521_SIZE_IN_BYTES));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/** Set RSA under reset. */
	XAsufw_CryptoCoreSetReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the provided ECDSA public key for the specified elliptic
 * 		curve using RSA core.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	CurveType	ECC curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PubKeyAddr	Address of the public key buffer, whose length shall be equal to
 * 				double of CurveLen as it contains both Qx, Qy components.
 *
 * @return
 *	- XASUFW_SUCCESS, if public key provided is valid.
 *	- XASUFW_RSA_ECC_INVALID_PARAM, if any input parameter is invalid.
 *	- XASUFW_RSA_ECC_WRITE_DATA_FAIL, if write data through DMA fails.
 *	- XASUFW_RSA_ECC_READ_DATA_FAIL, if read data through DMA fails.
 *	- XASUFW_RSA_ECC_PUBLIC_KEY_ZERO, if public key is zero.
 *	- XASUFW_RSA_ECC_PUBLIC_KEY_WRONG_ORDER, if public key is in wrong order.
 *	- XASUFW_RSA_ECC_PUBLIC_KEY_NOT_ON_CRV, if public key is not on curve.
 *	- XASUFW_FAILURE, if validation fails due to other reasons.
 *	- XASUFW_RSA_CHANGE_ENDIANNESS_ERROR, if endianness change fails.
 *
 *************************************************************************************************/
s32 XRsa_EccValidatePubKey(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PubKeyAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEcc = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u32 CurveSize = 0U;
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 PubKey[XASU_ECC_P521_SIZE_IN_BYTES + XASU_ECC_P521_SIZE_IN_BYTES];
	EcdsaKey Key;
	EcdsaCrvInfo *Crv = NULL;

	/** Validate the input arguments. */
	if ((DmaPtr == NULL) || (PubKeyAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (Crv == NULL)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	if (CurveLen != CurveSize) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/** Copy public key to local address using DMA and change endianness of the data. */
	Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr, (u64)(UINTPTR)PubKey,
			       XAsu_DoubleCurveLength(CurveLen), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}

	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(PubKey, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(PubKey + CurveLen, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END_CLR;
		}
	}

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + CurveLen);

	/** Release RSA core reset. */
	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	/** Validate public key with provided public key and curve type. */
	XFIH_CALL(Ecdsa_ValidateKey, XFihEcc, Status, Crv, (EcdsaKey *)&Key);
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
	/** Zeroize local buffers. */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey,
			XAsu_DoubleCurveLength(XASU_ECC_P521_SIZE_IN_BYTES));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/** Set RSA under reset. */
	XAsufw_CryptoCoreSetReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates an ECDSA signature for the provided hash by using the
 * 		given private key associated with the elliptic curve.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	CurveType	ECC curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PrivKeyAddr	Address of the private key buffer, whose length shall be equal to
 * 				CurveLen.
 * @param	EphemeralKeyPtr	Pointer to the ephemeral key buffer, whose length shall be
 * 				CurveLen. This pointer shall be unused in the case of Edward curves.
 * @param	HashAddr	Address of the hash on which signature has to be generated.
 * @param	HashBufLen	Length of the hash in bytes.
 * @param	SignAddr	Address of the buffer to store the generated signature, whose
 * 				length shall be equal to double of CurveLen as it contains both
 * 				r, s components.
 *
 * @return
 *	- XASUFW_SUCCESS, if signature generation is successful.
 *	- XASUFW_RSA_ECC_INVALID_PARAM, if any of the input parameter is invalid.
 *	- XASUFW_RSA_ECC_WRITE_DATA_FAIL, if write data through DMA fails.
 *	- XASUFW_RSA_ECC_READ_DATA_FAIL, if read data through DMA fails.
 *	- XASUFW_RSA_CHANGE_ENDIANNESS_ERROR, if endianness change fails.
 *	- XASUFW_FAILURE, if sign generation fails due to other reasons.
 *	- XASUFW_RSA_ECC_GEN_SIGN_BAD_RAND_NUM, if bad signature is generated.
 *	- XASUFW_RSA_ECC_GEN_SIGN_INCORRECT_HASH_LEN, if invalid hash length is provided to the
 *	third party code.
 *	- XASUFW_RSA_ECC_HASH_CALC_FAIL, if hash calculation fails in case of Ed25519ph or Ed448ph.
 *
 *************************************************************************************************/
s32 XRsa_EccGenerateSignature(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PrivKeyAddr,
	const u8 *EphemeralKeyPtr, u64 HashAddr, u32 HashBufLen, u64 SignAddr)
{
	/**
	 * Capture the start time of the ECC signature generation operation using RSA core, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START();

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEcc = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 CurveSize = 0U;
	u8 PrivKey[XASU_ECC_P521_SIZE_IN_BYTES];
	u8 Signature[XASU_ECC_P521_SIZE_IN_BYTES + XASU_ECC_P521_SIZE_IN_BYTES];
	u8 Hash[XASU_ECC_P521_SIZE_IN_BYTES];
	u8 EphemeralKey[XASU_ECC_P521_SIZE_IN_BYTES];
	u8 PubKey[XASU_ECC_P521_SIZE_IN_BYTES + XASU_ECC_P521_SIZE_IN_BYTES];
	EcdsaSign Sign;
	EcdsaKey Key;
	EcdsaCrvInfo *Crv = NULL;
	u64 HashBufAddr = 0U;
	u32 HashLen = 0U;

	/** Validate the input arguments. */
	if ((DmaPtr == NULL) || (HashAddr == 0U) || (PrivKeyAddr == 0U) || (SignAddr == 0U) ||
			(EphemeralKeyPtr == NULL)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (Crv == NULL) || (CurveLen != CurveSize)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/** For curves other than Edward curves, */
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
		/** Validate the hash buffer length. */
		if ((HashBufLen == 0U) || (HashBufLen > XASU_SHA_512_HASH_LEN)) {
			Status = XASUFW_ECC_INVALID_PARAM;
			goto END;
		}

		/** - Copy Hash to local address using DMA. */
		Status = XAsufw_DmaXfr(DmaPtr, HashAddr, (u64)(UINTPTR)Hash, HashBufLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
			goto END_CLR;
		}

		/**
		 * - If input hash length is less than curve length, pad the extra bytes with 0's.
		 * - If input hash length is greater than curve length, take first curve length bytes
		 * as hash.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XRsa_EccPrepareHashForSignature(Hash, CurveSize, HashBufLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_HASH_BUF_PAD_FAIL;
			goto END;
		}
	}

	/** Copy private key to local address using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, PrivKeyAddr, (u64)(UINTPTR)PrivKey, CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}

	/** For curves other than Edward curves, */
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
		/** - Change endianness of the hash and private key. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(PrivKey, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(Hash, CurveSize);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END_CLR;
		}

		/** - Copy ephemeral key to local address and change the endianness of it. */
		Status = Xil_SMemCpy((u8 *)EphemeralKey, CurveLen, (const u8 *)EphemeralKeyPtr, CurveLen, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(EphemeralKey, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END_CLR;
		}
	}
	/** Release RSA core reset. */
	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	Sign.r = (u8 *)(UINTPTR)Signature;
	Sign.s = (u8 *)(UINTPTR)(Signature + CurveLen);

	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
		/**
		 * For curves other than Edward curves,
		 * - Generate signature with provided inputs and curve type.
		 */
		XFIH_CALL(Ecdsa_GenerateSign, XFihEcc, Status, Crv, Hash, Crv->Bits, PrivKey, EphemeralKey,
		  (EcdsaSign *)&Sign);
	} else {
		/**
		 * For Edward curves,
		 * - Generate the public key using the provided private key.
		 */
		Key.Qx = (u8 *)(UINTPTR)PubKey;
		Key.Qy = (u8 *)(UINTPTR)(PubKey + CurveLen);

		XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Ecdsa_GeneratePublicKey,
				XASUFW_RSA_ECC_GEN_PUB_KEY_OPERATION_FAIL, XFihEcc, Status, END_CLR,
				Crv, PrivKey, (EcdsaKey *)&Key, DmaPtr);

		if (Crv->Class == ECDSA_ED_PH) {
			/** - Calculate the hash for the input message for Hash-based EdDSA mode. */
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XRsa_EccHashCalc(DmaPtr, (u32)Crv->CrvType, HashAddr, (u64)(UINTPTR)Hash,
					HashBufLen);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_RSA_ECC_HASH_CALC_FAIL;
				goto END;
			}
			HashBufAddr = (u64)(UINTPTR)Hash;
			HashLen = XASU_SHA_512_HASH_LEN;
		} else {
			HashBufAddr = HashAddr;
			HashLen = HashBufLen;
		}

		/** - Generate signature with provided inputs. */
		XFIH_CALL(Ecdsa_GenerateEdSign, XFihEcc, Status, Crv, (u8*)(UINTPTR)HashBufAddr,
			(HashLen * XASUFW_BYTE_LEN_IN_BITS), PrivKey, (EcdsaKey *)&Key,
			(EcdsaSign *)&Sign, DmaPtr);
	}
	if ((Status == XRSA_ECC_GEN_SIGN_BAD_R) ||
	    (Status == XRSA_ECC_GEN_SIGN_BAD_S)) {
		Status = XASUFW_RSA_ECC_GEN_SIGN_BAD_RAND_NUM;
	} else if ((Status == XRSA_ECC_GEN_SIGN_INCORRECT_HASH_LEN)) {
		Status = XASUFW_RSA_ECC_GEN_SIGN_INCORRECT_HASH_LEN;
	} else if ((Status != XRSA_ECC_SUCCESS)) {
		Status = XASUFW_FAILURE;
	} else {
		/**
		 *  Change endianness of the generated signature and copy it to destination address
		 *  using DMA if the curve type provided is other than Edward curve.
		 */
		if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XAsufw_ChangeEndianness(Signature, CurveLen);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
				goto END_CLR;
			}
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XAsufw_ChangeEndianness(Signature + CurveLen, CurveLen);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
				goto END_CLR;
			}
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)Signature, SignAddr,
						XAsu_DoubleCurveLength(CurveLen), 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_READ_DATA_FAIL;
		}
	}

	/**
	 * Measure and print the performance time for the ECC signature generation operation
	 * using RSA core, if performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(__func__);

END_CLR:
	/** Zeroize local buffers. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcc, ClearStatus, (u8 *)(UINTPTR)PrivKey,
					XASU_ECC_P521_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)Signature,
					XAsu_DoubleCurveLength(XASU_ECC_P521_SIZE_IN_BYTES));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)Hash, XASU_ECC_P521_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihEcc, ClearStatus, (u8 *)(UINTPTR)EphemeralKey,
					XASU_ECC_P521_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey,
					XAsu_DoubleCurveLength(XASU_ECC_P521_SIZE_IN_BYTES));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/** Set RSA under reset. */
	XAsufw_CryptoCoreSetReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function verifies the validity of an ECDSA signature for the provided hash
 * 		using the provided ecc public key using RSA core.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	CurveType	ECC Curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PubKeyAddr	Address of the public key buffer, whose length shall be equal to
 * 				double of CurveLen as it contains both Qx, Qy components.
 * @param	HashAddr	Address of the hash for which signature has to be verified.
 * @param	HashBufLen	Length of the hash in bytes.
 * @param	SignAddr	Address of the signature buffer, whose length shall be equal to
 * 				double of CurveLen as it contains both r, s components.
 *
 * @return
 *	-	XASUFW_SUCCESS, if signature provided is valid.
 *	-	XASUFW_RSA_ECC_INVALID_PARAM, if any of the input parameter is invalid.
 *	-	XASUFW_RSA_ECC_WRITE_DATA_FAIL, if write data through DMA fails.
 *	-	XASUFW_RSA_ECC_BAD_SIGN, if signature provided for verification is bad.
 *	-	XASUFW_RSA_ECC_VER_SIGN_INCORRECT_HASH_LEN, if incorrect hash length for sign
 *						verification.
 *	-	XASUFW_RSA_ECC_VER_SIGN_R_ZERO, if provided R is zero.
 *	-	XASUFW_RSA_ECC_VER_SIGN_S_ZERO, if provided S is zero.
 *	-	XASUFW_RSA_ECC_VER_SIGN_R_ORDER_ERROR, if R is not within ECC order.
 *	-	XASUFW_RSA_ECC_VER_SIGN_S_ORDER_ERROR, if S is not within ECC order.
 *	-	XASUFW_RSA_CHANGE_ENDIANNESS_ERROR, if endianness change fails.
 *	-	XASUFW_FAILURE, if operation fails due to any other reasons.
 *
 *************************************************************************************************/
s32 XRsa_EccVerifySignature(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PubKeyAddr,
	u64 HashAddr, u32 HashBufLen, u64 SignAddr)
{
	/**
	 * Capture the start time of the ECC signature verification operation using RSA core, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START();

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEcc = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 CurveSize = 0U;
	u8 PubKey[XASU_ECC_P521_SIZE_IN_BYTES + XASU_ECC_P521_SIZE_IN_BYTES];
	u8 Signature[XASU_ECC_P521_SIZE_IN_BYTES + XASU_ECC_P521_SIZE_IN_BYTES];
	u8 Hash[XASU_ECC_P521_SIZE_IN_BYTES];
	EcdsaSign Sign;
	EcdsaKey Key;
	EcdsaCrvInfo *Crv = NULL;
	u64 HashBufAddr = 0U;
	u32 HashLen = 0U;

	/** Validate the input arguments. */
	if ((DmaPtr == NULL) || (HashAddr == 0U) || (PubKeyAddr == 0U) || (SignAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (Crv == NULL) || (CurveLen != CurveSize)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/** For curves other than Edward curves, */
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
		/** Validate the hash buffer length. */
		if ((HashBufLen == 0U) || (HashBufLen > XASU_SHA_512_HASH_LEN)) {
			Status = XASUFW_ECC_INVALID_PARAM;
			goto END;
		}

		/** - Copy Hash to local address using DMA. */
		Status = XAsufw_DmaXfr(DmaPtr, HashAddr, (u64)(UINTPTR)Hash, HashBufLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
			goto END_CLR;
		}

		/**
		 * - If input hash length is less than curve length, pad the extra bytes with 0's.
		 * - If input hash length is greater than curve length, take first curve length bytes
		 * as hash.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XRsa_EccPrepareHashForSignature(Hash, CurveSize, HashBufLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_HASH_BUF_PAD_FAIL;
			goto END;
		}
	}

	/** Copy signature and public key to local address using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr, (u64)(UINTPTR)PubKey,
			       XAsu_DoubleCurveLength(CurveLen), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, SignAddr, (u64)(UINTPTR)Signature,
			       XAsu_DoubleCurveLength(CurveLen), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}

	/** For the curves other than Edward curves, */
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
		/** - Change endianness of the hash, public key and the signature. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(Hash, CurveSize);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(PubKey, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(PubKey + CurveLen, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(Signature, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(Signature + CurveLen, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END_CLR;
		}
	}

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + CurveLen);

	Sign.r = (u8 *)(UINTPTR)Signature;
	Sign.s = (u8 *)(UINTPTR)(Signature + CurveLen);

	/** Release RSA core reset. */
	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	/** Verify the signature with provided inputs and curve type. */
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
		XFIH_CALL(Ecdsa_VerifySign, XFihEcc, Status, Crv, Hash, Crv->Bits,
			 (EcdsaKey *)&Key, (EcdsaSign *)&Sign, DmaPtr);
	} else {

		if (Crv->Class == ECDSA_ED_PH) {
			/** Calculate the hash for the input message for Hash-based EdDSA mode. */
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XRsa_EccHashCalc(DmaPtr, (u32)Crv->CrvType, HashAddr, (u64)(UINTPTR)Hash,
					HashBufLen);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_RSA_ECC_HASH_CALC_FAIL;
				goto END;
			}
			HashBufAddr = (u64)(UINTPTR)Hash;
			HashLen = XASU_SHA_512_HASH_LEN;
		} else {
			HashBufAddr = HashAddr;
			HashLen = HashBufLen;
		}
		XFIH_CALL(Ecdsa_VerifySign, XFihEcc, Status, Crv, (u8*)(UINTPTR)HashBufAddr,
			 (HashLen * XASUFW_BYTE_LEN_IN_BITS), (EcdsaKey *)&Key,
			 (EcdsaSign *)&Sign, DmaPtr);
	}
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
		ReturnStatus = XASUFW_ECC_SIGNATURE_VERIFIED;
	}

	/**
	 * Measure and print the performance time for the ECC signature verification operation
	 * using RSA core, if performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(__func__);

END_CLR:
	/** Zeroize local buffers. */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey,
					XAsu_DoubleCurveLength(XASU_ECC_P521_SIZE_IN_BYTES));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)Signature,
					XAsu_DoubleCurveLength(XASU_ECC_P521_SIZE_IN_BYTES));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)Hash, XASU_ECC_P521_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/** Set RSA under reset. */
	XAsufw_CryptoCoreSetReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates the private key using TRNG and applies ModEccOrder to
 * 		the generated random number.
 *
 * @param	CurveType	ECC Curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PvtKey		Pointer to store the generated private key.
 * @param	InputRandBuf	Pointer to variable containing random buffer.
 * @param	InputRandBufLen	Length of random buffer.
 *
 * @return
 *	-	XASUFW_SUCCESS, if private key generation is successful.
 *	-	XASUFW_RSA_ECC_INVALID_PARAM, if any of the input parameter is invalid.
 *	-	XASUFW_RSA_ECC_INCORRECT_CURVE, if input curvetype or curvelen is incorrect.
 *	-	XASUFW_RSA_ECC_TRNG_FAILED, if random number generation fails.
 *	-	XASUFW_RSA_CHANGE_ENDIANNESS_ERROR, if changing edianness is failed.
 *	-	XASUFW_RSA_ECC_MOD_ORDER_FAILED, if ModEccOrder fails.
 *
 *************************************************************************************************/
s32 XRsa_EccGeneratePvtKey(u32 CurveType, u32 CurveLen, u8 *PvtKey, u8 *InputRandBuf,
			   u32 InputRandBufLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 RandBuf[XRSA_ECC_P521_RANDBUFLEN_IN_BYTES] = {0U};
	u8 *RandBufPtr = InputRandBuf;
	EcdsaCrvInfo *CrvInfo = NULL;
	u32 CurveSize = 0U;
	u8 RandBufLen = 0U;

	/** Validate input parameters. */
	if (PvtKey == NULL) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/** Validate curve type, curve length and get Curve information. */
	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &CrvInfo);
	if ((CurveSize == 0U) || (CurveLen != CurveSize) || (CrvInfo == NULL)) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	RandBufLen = (u8)((CurveLen & ~(XASUFW_VALUE_THREE)) + XRSA_ECC_KEY_PAIR_GEN_EXTRA_BYTES);

	/** Validate input random buffer length. */
	if ((InputRandBuf != NULL) && (InputRandBufLen < RandBufLen)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/** Generate random number for calculating private key. */
	if (InputRandBuf == NULL) {
		Status = XAsufw_TrngGetRandomNumbers(RandBuf,
						     (CurveLen +
						      XRSA_ECC_RAND_NUM_GEN_EXTRA_BYTES));
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_TRNG_FAILED);
			goto END;
		}
		RandBufPtr = RandBuf;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(RandBufPtr, RandBufLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	/** Release Reset. */
	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	/**
	 * Calculate the private key with the random number generated by TRNG using
	 * Ecdsa_ModWithEccOrder API.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Ecdsa_ModWithEccOrder(CrvInfo, RandBufPtr, RandBufLen,
				PvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_MOD_ORDER_FAILED;
	}

END:
	if (InputRandBuf == NULL) {
		/** Zeroize the local buffer. */
		Status = XAsufw_UpdateBufStatus(Status, Xil_SecureZeroize(RandBufPtr,
						XASU_ECC_P521_SIZE_IN_BYTES));
	}

	/** Set RSA under reset. */
	XAsufw_CryptoCoreSetReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function gets the curve related information.
 *
 * @param	CurveType	ECC Curve type.
 *
 * @return
 *		- Pointer to curve information, if CurveType is valid.
 *		- NULL, if CurveType is invalid.
 *
 *************************************************************************************************/
EcdsaCrvInfo *XRsa_EccGetCrvData(u32 CurveType)
{
	u32 Index;
	EcdsaCrvInfo *Crv = NULL;
	u32 TotalCurves = XRsa_EccCrvsGetCount();
	u16 CrvType;
	u16 CrvClass;

	if (CurveType > XASU_ECC_NIST_ED448_PH) {
		goto END;
	}

	CrvType = XRsa_EccCrvIndexDb[CurveType].CurveType;
	CrvClass = XRsa_EccCrvIndexDb[CurveType].CurveClass;

	for (Index = 0U; Index < TotalCurves; Index++) {
		if ((XRsa_EccCrvsDb[Index].CrvType == (EcdsaCrvTyp)CrvType) &&
		    (XRsa_EccCrvsDb[Index].Class == (EcdsaCrvClass)CrvClass)) {
			Crv = &XRsa_EccCrvsDb[Index];
			break;
		}
	}

END:
	return Crv;
}

/*************************************************************************************************/
/**
 * @brief	This function validates and gets curve info and curve size in bytes.
 *
 * @param	CurveType	Type of elliptic curve
 * @param	Crv		Pointer to EcdsaCrvInfo
 *
 * @return
 *		- Size of curve in bytes, if CurveType is valid.
 *		- Zero, if CurveType is invalid.
 *
 *************************************************************************************************/
u32 XRsa_EccValidateAndGetCrvInfo(u32 CurveType, EcdsaCrvInfo **Crv)
{
	u32 CurveSize = 0U;
	EcdsaCrvInfo *CrvInfo = XRsa_EccGetCrvData(CurveType);

	if (CrvInfo != NULL) {
		CurveSize = (u32)CrvInfo->Bits / XASUFW_BYTE_LEN_IN_BITS;
		if (CrvInfo->CrvType == ECDSA_NIST_P521) {
			CurveSize += (CurveSize % XRSA_ECC_ALGN_CRV_SIZE_IN_BYTES);
		}
		*Crv = CrvInfo;
	}

	return CurveSize;
}

/*************************************************************************************************/
/**
 * @brief	This function generates an ECDH shared secret by using the given public key and
 * 		given private key associated with the elliptic curve.
 *
 * @param	DmaPtr - Pointer to DMA instance.
 * @param	CurveType - Curve Type of the keys used to generate shared secret
 * @param	CurveLen - Length of the curve in bytes.
 * @param	PrivKeyAddr - 64-bit address of the private key
 * @param	PubKeyAddr - 64-bit address of public key
 * @param	SharedSecretAddr - 64-bit address of buffer for storing shared secret
 * @param	SharedSecretObjIdAddr - 64-bit address of buffer for storing shared secret ID
 *
 * @return
 *	-	XASUFW_SUCCESS, if ECDH shared secret is generated successfully.
 *	-	XASUFW_ECDH_INVALID_POINT_ON_CRV, if allocation of point failure.
 *	-	XASUFW_RSA_ECC_INVALID_PARAM, if any of the input parameters are invalid.
 *	-	XASUFW_RSA_ECC_WRITE_DATA_FAIL, if write data through DMA fails.
 *	-	XASUFW_RSA_ECC_READ_DATA_FAIL, if read data through DMA fails.
 *	-	XASUFW_ECDH_OTHER_ERROR, if operation fails due to any other error from
 *		third-party code.
 *	-	XASUFW_RSA_CHANGE_ENDIANNESS_ERROR, if endianness change fails.
 *	-	XASUFW_FAILURE, if operation fails due to any other reasons.
 *
 *************************************************************************************************/
s32 XRsa_EcdhGenSharedSecret(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PrivKeyAddr,
		u64 PubKeyAddr, u64 SharedSecretAddr, u64 SharedSecretObjIdAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEcdh;
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 SharedSecret[XASU_ECC_P521_SIZE_IN_BYTES];
	u8 PrivKey[XASU_ECC_P521_SIZE_IN_BYTES];
	u8 PubKey[XASU_ECC_P521_SIZE_IN_BYTES + XASU_ECC_P521_SIZE_IN_BYTES];
	u8 SharedSecretObjId[XECDH_SHARED_SEC_OBJ_ID_SIZE];
	EcdsaCrvInfo *Crv = NULL;
	EcdsaKey Key;
	u32 CurveSize = 0U;

	/** Validate the input arguments. */
	if ((DmaPtr == NULL) || (PrivKeyAddr == 0U) || (PubKeyAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	if ((SharedSecretAddr == 0U) && (SharedSecretObjIdAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (CurveLen != CurveSize) || (Crv == NULL)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/** Copy public key to local address using DMA and change endianness of the data. */
	Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr, (u64)(UINTPTR)PubKey,
			       XAsu_DoubleCurveLength(CurveLen), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PubKey, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((PubKey + CurveLen), CurveLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END_CLR;
	}

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + CurveLen);

	/** Copy private key to local address using DMA and change endianness of the data. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, PrivKeyAddr, (u64)(UINTPTR)PrivKey, CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PrivKey, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END_CLR;
	}

	/** Release RSA core reset. */
	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	/** Generate ECDH shared secret using third-party library. */
	XFIH_CALL(Ecdsa_CDH_Q, XFihEcdh, Status, Crv, PrivKey, (EcdsaKey *)&Key, SharedSecret);

	/** Set RSA under reset. */
	XAsufw_CryptoCoreSetReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	if ((XRSA_ECC_GEN_POINT_INVALID == Status)) {
		Status = XASUFW_ECDH_INVALID_POINT_ON_CRV;
		goto END_CLR;
	} else if ((XASUFW_TRNG_INVALID_PARAM == Status) || (XASUFW_TRNG_INVALID_STATE == Status)
			|| (XASUFW_TRNG_INVALID_BUF_SIZE == Status)) {
				Status = XASUFW_ECDH_RAND_GEN_ERROR;
				goto END_CLR;

	} else if (XRSA_ECC_SUCCESS == Status) {
		Status = XASUFW_SUCCESS;
	} else{
		Status = XASUFW_ECDH_OTHER_ERROR;
	}
	/**
	 * Copy shared secret to destination address if not null using DMA and
	 * change endianness of the data.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(SharedSecret, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (SharedSecretAddr == 0U) {
		/* TODO: Store shared secret in keyvault and update SharedSecretObjId */
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)SharedSecretObjId,
					SharedSecretObjIdAddr, XECDH_SHARED_SEC_OBJ_ID_SIZE, 0U);
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)SharedSecret, SharedSecretAddr,
					CurveLen, 0U);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_READ_DATA_FAIL;
	} else {
		ReturnStatus = XASUFW_RSA_ECDH_SUCCESS;
	}

END_CLR:

	/** Zeroize local copy of private key, public key, shared secret objectID and shared secret. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcdh, ClearStatus, (u8 *)(UINTPTR)PrivKey,
					XASU_ECC_P521_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey,
					XAsu_DoubleCurveLength(XASU_ECC_P521_SIZE_IN_BYTES));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)SharedSecretObjId, XECDH_SHARED_SEC_OBJ_ID_SIZE);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihEcdh, ClearStatus, (u8 *)(UINTPTR)SharedSecret,
					XASU_ECC_P521_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs ECC pair wise consistency test for RSA core
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	CurveType	ECC Curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PrivKeyAddr	Address of the private key buffer, whose length shall be equal to
 * 				CurveLen.
 * @param	PubKeyAddr	Address of the public key buffer, whose length shall be equal to
 * 				double of CurveLen as it contains both Qx, Qy components.
 *
 * @return
 *	-	XASUFW_SUCCESS, if signature provided is valid.
 *	-	XASUFW_RSA_ECC_INVALID_PARAM, if any of the input parameter is invalid.
 *	-	XASUFW_RSA_ECC_PWCT_SIGN_GEN_FAIL, if sign generation fails.
 *	-	XASUFW_RSA_ECC_PWCT_SIGN_VER_FAIL, if sign verification fails.
 *
 *************************************************************************************************/
s32 XRsa_EccPwct(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PrivKeyAddr,
	u64 PubKeyAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 Signature[XASU_ECC_P521_SIZE_IN_BYTES + XASU_ECC_P521_SIZE_IN_BYTES];
	u8 EphemeralKey[XASU_ECC_P521_SIZE_IN_BYTES];
	const u8 Hash[XASU_ECC_P256_SIZE_IN_BYTES] = {
		0x8FU, 0xDFU, 0x4CU, 0x12U, 0x9AU, 0x90U, 0xBCU, 0x76U,
		0x38U, 0xA5U, 0x5BU, 0x22U, 0x9BU, 0xD4U, 0xAEU, 0x1BU,
		0x09U, 0x5DU, 0x6DU, 0x61U, 0x68U, 0xA0U, 0x43U, 0xC2U,
		0xC3U, 0x9FU, 0x53U, 0xE4U, 0xD8U, 0xC4U, 0xBEU, 0x5FU
	};

	if ((DmaPtr == NULL) || (PrivKeyAddr == 0U) || (PubKeyAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/** Generate the ephemeral key using TRNG */
	Status = XAsufw_TrngGetRandomNumbers(EphemeralKey, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_EPHEMERAL_KEY_GEN_FAIL;
		goto END_CLR;
	}

	/** Generate signature using the provided private key and curve type. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGenerateSignature(DmaPtr, CurveType, CurveLen, PrivKeyAddr,
		 EphemeralKey, (u64)(UINTPTR)Hash, XASU_ECC_P256_SIZE_IN_BYTES,
		 (u64)(UINTPTR)Signature);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_PWCT_SIGN_GEN_FAIL);
		goto END_CLR;
	}

	/** Verify the generated signature using the provided public key and curve type. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccVerifySignature(DmaPtr, CurveType, CurveLen, PubKeyAddr,
			(u64)(UINTPTR)Hash, XASU_ECC_P256_SIZE_IN_BYTES, (u64)(UINTPTR)Signature);
	if ((Status != XASUFW_SUCCESS) || (ReturnStatus != XASUFW_ECC_SIGNATURE_VERIFIED)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_PWCT_SIGN_VER_FAIL);
	}
	ReturnStatus = XASUFW_FAILURE;

END_CLR:
	/** Zeroize the local buffers. */
	Status = XAsufw_UpdateBufStatus(Status, Xil_SecureZeroize((u8 *)(UINTPTR)Signature,
					XAsu_DoubleCurveLength(XASU_ECC_P521_SIZE_IN_BYTES)));

	Status = XAsufw_UpdateBufStatus(Status, Xil_SecureZeroize((u8 *)(UINTPTR)EphemeralKey,
					XASU_ECC_P521_SIZE_IN_BYTES));

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the HashBufLen and aligns the hash buffer length to the
 *		specified curve size by padding with 0's if less than curve length.
 *
 * @param	HashAddr	Address of the hash buffer provided.
 * @param	CurveSize	ECC Curve size.
 * @param	HashBufLen	Length of the hash buffer.
 *
 * @return
 *	- XASUFW_SUCCESS, if hash buffer alignment with CurveSize is successful.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if padding fails.
 *	- XASUFW_RSA_ECC_INVALID_PARAM, if Hash buffer length is NULL.
 *
 *************************************************************************************************/
s32 XRsa_EccPrepareHashForSignature(u8* HashPtr, u32 CurveSize, u32 HashBufLen)
{
	s32 Status = XASUFW_FAILURE;
	u32 HashDiffLen = 0U;

	/** Validate Hash Buffer Length. */
	if (HashBufLen == 0U) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/** Pad the hash buffer with 0's if less than curve length to align with curve length. */
	if (HashBufLen < CurveSize) {
		HashDiffLen = CurveSize - HashBufLen;
		Status = Xil_SMemSet((HashPtr + HashBufLen), HashDiffLen, 0U, HashDiffLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		}
	} else {
		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function calculates the hash for the input message for Hash-based EdDSA mode.
 * 		For Ed25519ph mode, it uses SHA-512 and for Ed448ph mode, it uses SHAKE256. Length
 * 		of the hash is 64 bytes for both modes.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	CurveType	ECC Curve type.
 * @param	DataAddr	Address of the input data buffer.
 * @param	HashAddr	Address of the hash buffer at which the generated hash to be stored.
 * @param	DataSize	Length of the input data in bytes.
 *
 * @return
 *	- XASUFW_SUCCESS, if Hash calculation is successful.
 *	- XASUFW_SHA_INVALID_PARAM, if ShaInstancePtr is NULL.
 *
 *************************************************************************************************/
static s32 XRsa_EccHashCalc(XAsufw_Dma *DmaPtr, u32 CurveType, u64 DataAddr, u64 HashAddr,
			u32 DataSize)
{
	s32 Status = XASUFW_FAILURE;
	XSha *ShaInstancePtr = NULL;
	XAsu_ShaOperationCmd ShaOperation;

	if (CurveType == (u32)ECDSA_ED25519) {
		/** For Ed25519ph, get the SHA2 instance for SHA512 mode. */
		ShaInstancePtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
		ShaOperation.ShaMode = XASU_SHA_MODE_512;
	} else {
		/** For Ed448ph, get the SHA3 instance for SHAKE256 mode. */
		ShaInstancePtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
		ShaOperation.ShaMode = XASU_SHA_MODE_SHAKE256;
	}
	/** Validate the SHA instance pointer. */
	if (ShaInstancePtr == NULL) {
		Status = XASUFW_SHA_INVALID_PARAM;
		goto END;
	}
	/** Set up the SHA operation parameters. */
	ShaOperation.HashBufSize = XASU_SHA_512_HASH_LEN;
	ShaOperation.DataAddr = DataAddr;
	ShaOperation.HashAddr = HashAddr;
	ShaOperation.DataSize = DataSize;

	/** Calculate the hash. */
	Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaOperation);

END:
	return Status;
}
/** @} */
