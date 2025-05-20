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

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
/** Message to be used for pair wise consistency test. */
static const u8 MsgPwctRsaEcc[XASU_ECC_P521_SIZE_IN_BYTES] = {
	0x8FU, 0xDFU, 0x4CU, 0x12U, 0x9AU, 0x90U, 0xBCU, 0x76U,
	0x38U, 0xA5U, 0x5BU, 0x22U, 0x9BU, 0xD4U, 0xAEU, 0x1BU,
	0x09U, 0x5DU, 0x6DU, 0x61U, 0x68U, 0xA0U, 0x43U, 0xC2U,
	0xC3U, 0x9FU, 0x53U, 0xE4U, 0xD8U, 0xC4U, 0xBEU, 0x5FU,
	0x7FU, 0x6BU, 0x93U, 0xD1U, 0x68U, 0xA8U, 0x3FU, 0x8DU,
	0x43U, 0xB2U, 0x9FU, 0x25U, 0x5BU, 0x56U, 0xC8U, 0xD5U,
	0x40U, 0x5BU, 0x1EU, 0xE2U, 0x0FU, 0x9FU, 0x05U, 0x29U,
	0x06U, 0xFBU, 0xE5U, 0x0BU, 0xE6U, 0x7BU, 0xAFU, 0x7AU,
	0x56U, 0xC8U
};

/** Ephemeral Key to be used for pair wise consistency test. */
static const u8 EKeyPwctRsaEcc[XASU_ECC_P521_SIZE_IN_BYTES] = {
	0x36U, 0x77U, 0xFBU, 0xF9U, 0xBBU, 0x2DU, 0x96U, 0xA3U,
	0x1BU, 0x01U, 0x11U, 0x08U, 0x57U, 0x93U, 0x8CU, 0xC4U,
	0x9DU, 0x9AU, 0x30U, 0xA4U, 0xE0U, 0x0EU, 0x9CU, 0xD4U,
	0xB5U, 0x5DU, 0x97U, 0x77U, 0x58U, 0x0CU, 0x84U, 0xC7U,
	0x0CU, 0x67U, 0x48U, 0x94U, 0xE8U, 0x53U, 0xD3U, 0x6BU,
	0xBEU, 0xC6U, 0xC2U, 0x1FU, 0xDCU, 0xFCU, 0x7BU, 0xD1U,
	0xF8U, 0x2BU, 0x72U, 0xD3U, 0xA4U, 0xC2U, 0x8EU, 0x10U,
	0xD8U, 0x25U, 0x5DU, 0x21U, 0x33U, 0xD5U, 0xCAU, 0x38U,
	0xCAU, 0x38U
};

#ifdef XASUFW_ENABLE_PERF_MEASUREMENT
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
 *
 *************************************************************************************************/
s32 XRsa_EccGeneratePubKey(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PrivKeyAddr,
			   u64 PubKeyAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
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

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PrivKey, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + CurveLen);

	/** Release RSA core reset. */
	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	/** Generate public key with provided private key and curve type. */
	XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Ecdsa_GeneratePublicKey,
					   XASUFW_RSA_ECC_GEN_PUB_KEY_OPERATION_FAIL, XFihVar, Status, END_CLR,
					   Crv, PrivKey, (EcdsaKey *)&Key);
	/**
	 * Change endianness of the generated public key and copy it to destination address
	 * using DMA.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PubKey, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PubKey + CurveLen, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)PubKey, PubKeyAddr,
					XAsu_DoubleCurveLength(CurveLen), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_READ_DATA_FAIL;
	}

	/** Validate the public key generated from the private key. */
	XFIH_CALL_GOTO(XRsa_EccValidatePubKey, XFihVar, Status, END_CLR, DmaPtr, CurveType, CurveLen, PubKeyAddr);

	/** Perform pair wise consistency test using the key pair. */
	XFIH_CALL(XRsa_EccPwct, XFihVar, Status, DmaPtr, CurveType, CurveLen, PrivKeyAddr,
		PubKeyAddr);

END_CLR:
	/** Zeroize local key copy. */
	XFIH_CALL(Xil_SecureZeroize, XFihVar, ClearStatus, (u8 *)(UINTPTR)PrivKey,
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
 *
 *************************************************************************************************/
s32 XRsa_EccValidatePubKey(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PubKeyAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
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

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PubKey, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PubKey + CurveLen, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + CurveLen);

	/** Release RSA core reset. */
	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	/** Validate public key with provided public key and curve type. */
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
	/** Zeroize local key copy. */
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
 * 		given private key associated with the elliptic curve using RSA core.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	CurveType	ECC curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PrivKeyAddr	Address of the private key buffer, whose length shall be equal to
 * 				CurveLen.
 * @param	EphemeralKeyPtr	Pointer to the ephemeral key buffer, whose length shall be
 * 				CurveLen.
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
 *	- XASUFW_RSA_ECC_GEN_SIGN_BAD_RAND_NUM, if bad random number used for sign generation.
 *	- XASUFW_RSA_ECC_GEN_SIGN_INCORRECT_HASH_LEN, if incorrect hash length is provided
 *					for sign generation.
 *	- XASUFW_FAILURE, if sign generation fails due to other reasons.
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
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 CurveSize = 0U;
	u8 PrivKey[XASU_ECC_P521_SIZE_IN_BYTES];
	u8 Signature[XASU_ECC_P521_SIZE_IN_BYTES + XASU_ECC_P521_SIZE_IN_BYTES];
	u8 Hash[XASU_ECC_P521_SIZE_IN_BYTES];
	u8 EphemeralKey[XASU_ECC_P521_SIZE_IN_BYTES];
	EcdsaSign Sign;
	EcdsaCrvInfo *Crv = NULL;

	/** Validate the input arguments. */
	if ((DmaPtr == NULL) || (HashAddr == 0U) || (PrivKeyAddr == 0U) || (SignAddr == 0U) ||
			(EphemeralKeyPtr == NULL)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (Crv == NULL)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	if ((CurveLen != CurveSize) || (HashBufLen != CurveSize)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/** Copy ephemeral key to local address and change the endianness of the data. */
	Status = Xil_SMemCpy((u8 *)EphemeralKey, CurveLen, (const u8 *)EphemeralKeyPtr, CurveLen, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(EphemeralKey, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	/**
	 * Copy private key and hash to local address using DMA and change the endianness
	 * of the data.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, PrivKeyAddr, (u64)(UINTPTR)PrivKey, CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PrivKey, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, HashAddr, (u64)(UINTPTR)Hash, HashBufLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(Hash, HashBufLen);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	Sign.r = (u8 *)(UINTPTR)Signature;
	Sign.s = (u8 *)(UINTPTR)(Signature + CurveLen);

	/** Release RSA core reset. */
	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	/** Generate signature with provided inputs and curve type. */
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
		/**
		 *  Change endianness of the generated signature and copy it to destination address
		 *  using DMA.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(Signature, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			goto END_CLR;
		}
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(Signature + CurveLen, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			goto END_CLR;
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
	/** Zeroize local key copy. */
	XFIH_CALL(Xil_SecureZeroize, XFihVar, ClearStatus, (u8 *)(UINTPTR)PrivKey,
					XASU_ECC_P521_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)Signature,
					XAsu_DoubleCurveLength(XASU_ECC_P521_SIZE_IN_BYTES));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)Hash, XASU_ECC_P521_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)EphemeralKey, XASU_ECC_P521_SIZE_IN_BYTES);
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
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 CurveSize = 0U;
	u8 PubKey[XASU_ECC_P521_SIZE_IN_BYTES + XASU_ECC_P521_SIZE_IN_BYTES];
	u8 Signature[XASU_ECC_P521_SIZE_IN_BYTES + XASU_ECC_P521_SIZE_IN_BYTES];
	u8 Hash[XASU_ECC_P521_SIZE_IN_BYTES];
	EcdsaSign Sign;
	EcdsaKey Key;
	EcdsaCrvInfo *Crv = NULL;

	/** Validate the input arguments. */
	if ((DmaPtr == NULL) || (HashAddr == 0U) || (PubKeyAddr == 0U) || (SignAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (Crv == NULL)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	if ((CurveLen != CurveSize) || (HashBufLen != CurveSize)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/**
	 * Copy signature, public key and hash to local address using DMA and change the
	 * endianness of the data.
	 */
	Status = XAsufw_DmaXfr(DmaPtr, HashAddr, (u64)(UINTPTR)Hash, HashBufLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(Hash, HashBufLen);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr, (u64)(UINTPTR)PubKey,
			       XAsu_DoubleCurveLength(CurveLen), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PubKey, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PubKey + CurveLen, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + CurveLen);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, SignAddr, (u64)(UINTPTR)Signature,
			       XAsu_DoubleCurveLength(CurveLen), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(Signature, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(Signature + CurveLen, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	Sign.r = (u8 *)(UINTPTR)Signature;
	Sign.s = (u8 *)(UINTPTR)(Signature + CurveLen);

	/** Release RSA core reset. */
	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	/** Verify the signature with provided inputs and curve type. */
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

	/**
	 * Measure and print the performance time for the ECC signature verification operation
	 * using RSA core, if performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(__func__);

END_CLR:
	/** Zeroize local key copy. */
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
 *
 * @return
 *	-	XASUFW_SUCCESS, if private key generation is successful.
 *	-	XASUFW_RSA_ECC_INVALID_PARAM, if any of the input parameter is invalid.
 *	-	XASUFW_RSA_ECC_INCORRECT_CURVE, if input curvetype or curvelen is incorrect.
 *	-	XASUFW_RSA_ECC_TRNG_FAILED, if random number generation fails.
 *	-	XASUFW_RSA_ECC_MOD_ORDER_FAILED, if ModEccOrder fails.
 *
 *************************************************************************************************/
s32 XRsa_EccGeneratePvtKey(u32 CurveType, u32 CurveLen, u8* PvtKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 RandBuf[XASU_ECC_P521_SIZE_IN_BYTES] = {0U};
	EcdsaCrvInfo *CrvInfo = NULL;
	u32 CurveSize = 0U;

	/** Validate input parameters. */
	/**
	 * TODO: Remove (CurveType != XASU_ECC_NIST_P384) validation when Ecdsa_ModEccOrder()
	 * API supports all curves.
	 */
	if ((PvtKey == NULL) || (CurveType != XASU_ECC_NIST_P384)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/** Validate curve type, curve length and get Curve information. */
	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &CrvInfo);
	if ((CurveSize == 0U) || (CurveLen != CurveSize) || (CrvInfo == NULL)) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	/** Generate random number for calculating private key. */
	Status = XAsufw_TrngGetRandomNumbers(RandBuf, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_TRNG_FAILED);
		goto END;
	}

	/** Release Reset. */
	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	/**
	 * Calculate the private key with the random number generated by TRNG using
	 * Ecdsa_ModEccOrder API.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Ecdsa_ModEccOrder(CrvInfo, RandBuf, PvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_MOD_ORDER_FAILED;
	}

END:
	/** Zeroize the local buffer. */
	Status = XAsufw_UpdateBufStatus(Status, Xil_SecureZeroize((u8 *)(UINTPTR)RandBuf,
			XASU_ECC_P521_SIZE_IN_BYTES));

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

	if ((CurveType != XRSA_ECC_CURVE_TYPE_NIST_P521) &&
	    (CurveType != XRSA_ECC_CURVE_TYPE_NIST_P192) &&
	    (CurveType != XRSA_ECC_CURVE_TYPE_NIST_P224) &&
	    (CurveType != XRSA_ECC_CURVE_TYPE_NIST_P256) &&
	    (CurveType != XRSA_ECC_CURVE_TYPE_NIST_P384) &&
	    (CurveType != XRSA_ECC_CURVE_TYPE_BRAINPOOL_P256) &&
	    (CurveType != XRSA_ECC_CURVE_TYPE_BRAINPOOL_P320) &&
	    (CurveType != XRSA_ECC_CURVE_TYPE_BRAINPOOL_P384) &&
	    (CurveType != XRSA_ECC_CURVE_TYPE_BRAINPOOL_P512)) {
		goto END;
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
		CurveSize += (CurveSize % XRSA_ECC_ALGN_CRV_SIZE_IN_BYTES);
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
 *	-	XASUFW_ECDH_OTHER_ERROR, if operation fails due to any other error from IP cores.
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
		goto END_CLR;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((PubKey + CurveLen), CurveLen);
	if (Status != XASUFW_SUCCESS) {
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
		goto END_CLR;
	}

	/** Release RSA core reset. */
	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	/** Generate ECDH shared secret using IPCores. */
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

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)SharedSecret, XASU_ECC_P521_SIZE_IN_BYTES);
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

	if ((DmaPtr == NULL) || (PrivKeyAddr == 0U) || (PubKeyAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	Status = XRsa_EccGenerateSignature(DmaPtr, CurveType, CurveLen, PrivKeyAddr,
			EKeyPwctRsaEcc, (u64)(UINTPTR)MsgPwctRsaEcc, CurveLen,
			(u64)(UINTPTR)Signature);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_PWCT_SIGN_GEN_FAIL);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccVerifySignature(DmaPtr, CurveType, CurveLen, PubKeyAddr,
			(u64)(UINTPTR)MsgPwctRsaEcc, CurveLen,
			(u64)(UINTPTR)Signature);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_PWCT_SIGN_VER_FAIL);
	}

END_CLR:
	Status = XAsufw_UpdateBufStatus(Status, Xil_SecureZeroize((u8 *)(UINTPTR)Signature,
					XAsu_DoubleCurveLength(XASU_ECC_P521_SIZE_IN_BYTES)));

END:
	return Status;
}
/** @} */
