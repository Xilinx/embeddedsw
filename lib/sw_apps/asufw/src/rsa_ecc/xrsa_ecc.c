/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       yog  09/10/26 Fix ECDSA ephemeral nonce for Brainpool curves.
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
#include "xtrng.h"
#include "xsha.h"
#include "xsha_hw.h"
#include "xasufw_perf.h"
#include "xil_sutil.h"

#ifdef XASU_RSA_ECC_ENABLE
/************************************ Constant Definitions ***************************************/
#define XRSA_ECC_ALGN_CRV_SIZE_IN_BYTES		(2U)	/**< Align ECDSA curve size in bytes */

#define XRSA_BASEADDRESS			(0xEBF50000U) /**< RSA base address */
#define XRSA_RESET_OFFSET			(0x00000040U) /**< RSA reset offset */

/* Return value in case of success from third-party library */
#define XRSA_ECC_SUCCESS			ELLIPTIC_SUCCESS /**< Success from third-party library */
/* Validate Public Key error codes from third-party library */
#define XRSA_ECC_KEY_ZERO			ELLIPTIC_KEY_ZERO
							/**< Error from third-party library if key is zero */
#define XRSA_ECC_KEY_WRONG_ORDER		ELLIPTIC_KEY_WRONG_ORDER
							/**< Error from third-party library if key is in wrong order */
#define XRSA_ECC_KEY_NOT_ON_CRV			ELLIPTIC_KEY_NOT_ON_CRV
							/**< Error from third-party library if key point is not on curve */
/* Verify Sign error codes from third-party library */
#define XRSA_ECC_BAD_SIGN			ELLIPTIC_BAD_SIGN
							/**< Error from third-party library if bad sign */
#define XRSA_ECC_VER_SIGN_INCORRECT_HASH_LEN	ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN
							/**< Error from third-party library if hash length is incorrect */
#define XRSA_ECC_VER_SIGN_R_ZERO		ELLIPTIC_VER_SIGN_R_ZERO /**< Error from third-party
									  * library if sign R is zero */
#define XRSA_ECC_VER_SIGN_S_ZERO		ELLIPTIC_VER_SIGN_S_ZERO /**< Error from third-party
									  * library if sign S is zero */
#define XRSA_ECC_VER_SIGN_R_ORDER_ERROR		ELLIPTIC_VER_SIGN_R_ORDER_ERROR
							/**< Error from third-party library if sign R is in wrong order */
#define XRSA_ECC_VER_SIGN_S_ORDER_ERROR		ELLIPTIC_VER_SIGN_S_ORDER_ERROR
							/**< Error from third-party library if sign S is in wrong order */
/* Generate sign error codes from third-party library */
#define XRSA_ECC_GEN_SIGN_BAD_R			ELLIPTIC_GEN_SIGN_BAD_R
							/**< Error from third-party library
							 * if generated sign has bad R. */
#define XRSA_ECC_GEN_SIGN_BAD_S			ELLIPTIC_GEN_SIGN_BAD_S
							/**< Error from third-party library if generated sign has bad S */
#define XRSA_ECC_GEN_SIGN_INCORRECT_HASH_LEN	ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN
							/**< Error from third-party library if hash length is incorrect */
#define XRSA_ECC_GEN_POINT_INVALID		(0x01)	/**< Error from third-party library
							 * if generated point is invalid */
/** Aligned Qy, Sign.s component offset for ECC key buffers */
#define XRSA_ECC_ALIGNED_COMPONENT_OFFSET	(XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES + \
						 XASUFW_VALUE_TWO)

/************************************** Type Definitions *****************************************/
/** This structure contains curve type and curve class for indexing. */
typedef struct {
	u16 CurveType; /**< Type of the curve */
	u16 CurveClass; /**< Class of the curve */
} XRsa_EccCrvIndex;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XRsa_EccEdHashCalc(XAsufw_Dma *DmaPtr, u32 CurveType, u64 DataAddr, u64 HashAddr,
		u32 DataSize);
static s32 XRsa_EccGenerateRandScalar(const EcdsaCrvInfo *CrvInfo, u32 CurveLen,
		const u8 *InputRandBuf, u8 ReduceModOrder, u8 *Scalar);

/************************************ Variable Definitions ***************************************/

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
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if memset fails while zeroizing local buffers.
 *	- XASUFW_DMA_SCRUB_FAIL, if the DMA secret-path scrub fails.
 *	- XASUFW_RSA_ECC_INCORRECT_CURVE, if input curvetype or curvelen is incorrect.
 *
 *************************************************************************************************/
s32 XRsa_EccGeneratePubKey(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PrivKeyAddr,
			   u64 PubKeyAddr)
{
	/**
	 * Capture the start time of the ECC public key generation operation using RSA core, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_ECC_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEcc = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 CurveSize = 0U;
	u8 PrivKey[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES] __attribute__((aligned(4))) = {0U};
	u8 PubKey[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES
			+ XASUFW_VALUE_TWO + XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES]
		__attribute__((aligned(4))) = {0U};
	EcdsaKey Key;
	EcdsaCrvInfo *Crv = NULL;

	/** Validate the input arguments. */
	if ((DmaPtr == NULL) || (PrivKeyAddr == 0U) || (PubKeyAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (Crv == NULL)) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	/**
	 * Validate CurveLen:
	 * - For Curve448: accept 56 bytes (XASU_ECC_CURVE448_PVT_KEY_SIZE_IN_BYTES)
	 * - For other curves: must match CurveSize
	 */
	if (((Crv->CrvType == ECDSA_CURVE448) &&
	     (CurveLen != XASU_ECC_CURVE448_PVT_KEY_SIZE_IN_BYTES)) ||
	    ((Crv->CrvType != ECDSA_CURVE448) && (CurveLen != CurveSize))) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	/**
	 * Zeroize the private key buffer before use to make sure for Curve448 57th byte is zero.
	 * Because, Curve448 private key is of 56 bytes.
	 */
	Status = Xil_SecureZeroize(PrivKey, XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/**
	 * Copy the private key to the local buffer. For Weierstrass curves the byte-order reversal
	 * rides the copy DMA; Edward/Montgomery curves take a plain copy.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448) &&
	    (Crv->CrvType != ECDSA_CURVE25519) && (Crv->CrvType != ECDSA_CURVE448)) {
		Status = XAsufw_DmaXfrInReverse(DmaPtr, PrivKeyAddr, PrivKey, CurveLen);
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, PrivKeyAddr, (u64)(UINTPTR)PrivKey, CurveLen, 0U);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + XRSA_ECC_ALIGNED_COMPONENT_OFFSET);

	/** Scrub the DMA secret datapath before releasing the core. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaScrubSecretPath(DmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_SCRUB_FAIL;
		goto END_CLR;
	}

	/** Release RSA core reset. */
	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	/** Generate public key with provided private key and curve type. */
	XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Ecdsa_GeneratePublicKey,
					   XASUFW_RSA_ECC_GEN_PUB_KEY_OPERATION_FAIL, XFihEcc, Status, END_CLR,
					   Crv, PrivKey, (EcdsaKey *)&Key, DmaPtr);
	/**
	 * Copy the generated public key to the destination address with byte-order reversal
	 * (endianness swap rides the copy DMA) for all curves except Ed25519 and Ed448.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448) &&
	    (Crv->CrvType != ECDSA_CURVE25519) && (Crv->CrvType != ECDSA_CURVE448)) {
		/** Weierstrass: reversal of Qx and Qy rides the copy-out DMA (per component). */
		Status = XAsufw_DmaXfrOutReverse(DmaPtr, PubKey, PubKeyAddr, CurveLen);
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)PubKey, PubKeyAddr, CurveLen, 0U);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_READ_DATA_FAIL;
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448) &&
	    (Crv->CrvType != ECDSA_CURVE25519) && (Crv->CrvType != ECDSA_CURVE448)) {
		Status = XAsufw_DmaXfrOutReverse(DmaPtr, PubKey + XRSA_ECC_ALIGNED_COMPONENT_OFFSET,
						 PubKeyAddr + CurveLen, CurveLen);
	} else {
		Status = XAsufw_DmaXfr(DmaPtr,
				       (u64)(UINTPTR)(PubKey + XRSA_ECC_ALIGNED_COMPONENT_OFFSET),
				       PubKeyAddr + CurveLen, CurveLen, 0U);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_READ_DATA_FAIL;
		goto END_CLR;
	}

	/** Validate the public key generated from the private key. */
	XFIH_CALL_GOTO(XRsa_EccValidatePubKey, XFihEcc, Status, END_CLR, DmaPtr, CurveType,
			CurveLen, PubKeyAddr);

	/**
	 * Perform pair wise consistency test using the key pair for all curve except
	 * Montgomery curves, since there is no signature generation/verification support for
	 * these curves.
	 */
	if ((Crv->CrvType != ECDSA_CURVE25519) && (Crv->CrvType != ECDSA_CURVE448)) {
		XFIH_CALL(XRsa_EccPwct, XFihEcc, Status, DmaPtr, CurveType, CurveLen, PrivKeyAddr,
			PubKeyAddr);
	}

	/**
	 * Measure and print the performance time for the ECC public key generation operation
	 * using RSA core, if performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_ECC_ID);

END_CLR:
	/** Zeroize local buffers. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcc, ClearStatus, (u8 *)(UINTPTR)PrivKey,
					XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey, sizeof(PubKey));
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
 *	- XASUFW_RSA_ECC_INCORRECT_CURVE, if input curvetype or curvelen is incorrect.
 *	- XASUFW_RSA_ECC_VALIDATE_PUB_KEY_FAIL, if public key validation fails.
 *
 *************************************************************************************************/
s32 XRsa_EccValidatePubKey(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PubKeyAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEcc = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u32 CurveSize = 0U;
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 PubKey[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES
			+ XASUFW_VALUE_TWO + XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES]
		__attribute__((aligned(4))) = {0U};
	EcdsaKey Key;
	EcdsaCrvInfo *Crv = NULL;

	/** Validate the input arguments. */
	if ((DmaPtr == NULL) || (PubKeyAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (Crv == NULL)) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	/**
	 * Validate CurveLen:
	 * - For Curve448: accept 56 bytes (XASU_ECC_CURVE448_PVT_KEY_SIZE_IN_BYTES)
	 * - For other curves: must match CurveSize
	 */
	if (((Crv->CrvType == ECDSA_CURVE448) &&
	     (CurveLen != XASU_ECC_CURVE448_PVT_KEY_SIZE_IN_BYTES)) ||
	    ((Crv->CrvType != ECDSA_CURVE448) && (CurveLen != CurveSize))) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	/** Copy the public key in; Weierstrass Qx/Qy reversal rides the DMA, others plain-copy. */
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448) &&
	    (Crv->CrvType != ECDSA_CURVE25519) && (Crv->CrvType != ECDSA_CURVE448)) {
		Status = XAsufw_DmaXfrInReverse(DmaPtr, PubKeyAddr, PubKey, CurveLen);
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr, (u64)(UINTPTR)PubKey, CurveLen, 0U);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448) &&
	    (Crv->CrvType != ECDSA_CURVE25519) && (Crv->CrvType != ECDSA_CURVE448)) {
		Status = XAsufw_DmaXfrInReverse(DmaPtr, PubKeyAddr + CurveLen,
						PubKey + XRSA_ECC_ALIGNED_COMPONENT_OFFSET,
						CurveLen);
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr + CurveLen,
				       (u64)(UINTPTR)(PubKey + XRSA_ECC_ALIGNED_COMPONENT_OFFSET),
				       CurveLen, 0U);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + XRSA_ECC_ALIGNED_COMPONENT_OFFSET);

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
		Status = XASUFW_RSA_ECC_VALIDATE_PUB_KEY_FAIL;
	} else {
		Status = XASUFW_SUCCESS;
	}

END_CLR:
	/** Zeroize local buffers. */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey, sizeof(PubKey));
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
 * @param	HashLen		Length of the hash buffer in bytes.
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
 *	- XASUFW_RSA_ECC_EPHEMERAL_KEY_GEN_FAIL, if ephemeral key generation fails.
 *	- XASUFW_RSA_ECC_TRNG_FAILED, if random number generation for the ephemeral key fails.
 *	- XASUFW_RSA_ECC_MOD_ORDER_FAILED, if reducing the ephemeral key modulo the curve
 *	order fails.
 *	- XASUFW_FAILURE, if sign generation fails due to other reasons.
 *	- XASUFW_RSA_ECC_GEN_SIGN_BAD_RAND_NUM, if bad signature is generated.
 *	- XASUFW_RSA_ECC_GEN_SIGN_INCORRECT_HASH_LEN, if invalid hash length is provided to the
 *	third party code.
 *	- XASUFW_RSA_ECC_HASH_CALC_FAIL, if hash calculation fails in case of Ed25519ph or Ed448ph.
 *	- XASUFW_RSA_ECC_INCORRECT_CURVE, if input curvetype or curvelen is incorrect.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if memset fails while zeroizing local buffers.
 *	- XASUFW_DMA_SCRUB_FAIL, if the DMA secret-path scrub fails.
 *	- Error code from XAsufw_UpdateBufStatus, if local buffer zeroization fails.
 *	- XASUFW_RSA_ECC_GEN_SIGN_FAIL, if signature generation fails in the third party code.
 *
 *************************************************************************************************/
s32 XRsa_EccGenerateSignature(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PrivKeyAddr,
	const u8 *EphemeralKeyPtr, u64 HashAddr, u32 HashLen, u64 SignAddr)
{
	/**
	 * Capture the start time of the ECC signature generation operation using RSA core, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_ECC_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEcc = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 CurveSize = 0U;
	u8 PrivKey[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES] __attribute__((aligned(4))) = {0U};
	u8 Signature[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES
			+ XASUFW_VALUE_TWO + XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES]
		__attribute__((aligned(4))) = {0U};
	u8 Hash[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES] __attribute__((aligned(4))) = {0U};
	u8 EphemeralKey[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES] __attribute__((aligned(4))) = {0U};
	u8 PubKey[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES
			+ XASUFW_VALUE_TWO + XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES]
		__attribute__((aligned(4))) = {0U};
	EcdsaSign Sign;
	EcdsaKey Key;
	EcdsaCrvInfo *Crv = NULL;
	u64 HashBufAddr = 0U;
	u32 HashBufLen = 0U;
	u32 HashPadLen = 0U;

	/** Validate the input arguments. */
	if ((DmaPtr == NULL) || (HashAddr == 0U) || (PrivKeyAddr == 0U) || (SignAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/**
	 * Error out if curve type is Curve25519 or Curve448 as they don't support
	 * signature operations.
	 */
	if ((CurveType == XASU_ECC_CURVE25519) || (CurveType == XASU_ECC_CURVE448)) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (Crv == NULL) || (CurveLen != CurveSize)) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	/** For curves other than Edward curves, */
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
		/** Validate the hash buffer length. */
		if ((HashLen == 0U) || (HashLen > XASU_SHA_512_HASH_LEN)) {
			Status = XASUFW_RSA_ECC_INVALID_PARAM;
			goto END;
		}

		/** Zeroize the hash buffer. */
		Status = Xil_SecureZeroize(Hash, XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_ZEROIZE_MEMSET_FAIL;
			goto END;
		}

		/**
		 * If input hash length is less than curve length, left pad the extra bytes with 0's
		 * and copy hash to the local buffer.
		 */
		HashPadLen  = (HashLen < CurveLen) ? (CurveLen - HashLen) : 0U;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, HashAddr, (u64)(UINTPTR)(Hash + HashPadLen), HashLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
			goto END_CLR;
		}
	}

	/** Copy the private key in; Weierstrass reversal rides the DMA, Edward curves plain-copy. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
		Status = XAsufw_DmaXfrInReverse(DmaPtr, PrivKeyAddr, PrivKey, CurveLen);
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, PrivKeyAddr, (u64)(UINTPTR)PrivKey, CurveLen, 0U);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}

	/** For curves other than Edward curves, */
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
		/**
		 * Reverse the hash byte order in place. The copy-in DMA above wrote HashLen bytes at an
		 * offset, so it cannot be ridden; the reversal over CurveSize is decoupled.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ReverseInPlace(DmaPtr, Hash, CurveSize);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END_CLR;
		}

		/**
		 * Prepare the ephemeral key (nonce 'k'):
		 * - If a key is provided (e.g., KAT/known vector), copy it to the local buffer
		 *   and change its endianness to the little-endian order expected by the core.
		 * - If no key is provided, the nonce is generated later (after the core reset is
		 *   released) using XRsa_EccGenerateRandScalar(), which reduces a random value
		 *   modulo the curve order 'n'. A raw curve-length draw must not be used directly,
		 *   as it can exceed 'n' (a large fraction of the time for Brainpool curves, whose
		 *   order is well below 2^bits), which the core rejects with an order error.
		 */
		if (EphemeralKeyPtr != NULL) {
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = Xil_SMemCpy((u8 *)EphemeralKey, CurveLen,
					(const u8 *)EphemeralKeyPtr, CurveLen, CurveLen);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_RSA_ECC_EPHEMERAL_KEY_GEN_FAIL;
				goto END_CLR;
			}

			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XAsufw_ReverseInPlace(DmaPtr, EphemeralKey, CurveLen);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
				goto END_CLR;
			}
		}
	}

	/** Scrub the DMA secret datapath before releasing the core. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaScrubSecretPath(DmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_SCRUB_FAIL;
		goto END_CLR;
	}

	/** Release RSA core reset. */
	XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	Sign.r = (u8 *)(UINTPTR)Signature;
	Sign.s = (u8 *)(UINTPTR)(Signature + XRSA_ECC_ALIGNED_COMPONENT_OFFSET);

	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
		/**
		 * For a TRNG-generated nonce, produce the ephemeral key in [1, n-1] by reducing
		 * a fresh random value modulo the curve order. The result is little-endian, which
		 * is the byte order the core expects for the nonce, so no reversal is needed. This
		 * must run after the core reset is released, as Ecdsa_ModEccOrder() uses the core.
		 */
		if (EphemeralKeyPtr == NULL) {
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XRsa_EccGenerateRandScalar(Crv, CurveLen, NULL, (u8)XASU_TRUE,
					EphemeralKey);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_EPHEMERAL_KEY_GEN_FAIL);
				goto END_CLR;
			}
		}

		/**
		 * For curves other than Edward curves,
		 * - Generate signature with provided inputs and curve type.
		 */
		XFIH_CALL(Ecdsa_GenerateSign, XFihEcc, Status, Crv, Hash, Crv->Bits, PrivKey,
			EphemeralKey, (EcdsaSign *)&Sign);
	} else {
		/**
		 * For Edward curves,
		 * - Generate the public key using the provided private key.
		 */
		Key.Qx = (u8 *)(UINTPTR)PubKey;
		Key.Qy = (u8 *)(UINTPTR)(PubKey + XRSA_ECC_ALIGNED_COMPONENT_OFFSET);

		XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Ecdsa_GeneratePublicKey,
				XASUFW_RSA_ECC_GEN_PUB_KEY_OPERATION_FAIL, XFihEcc, Status, END_CLR,
				Crv, PrivKey, (EcdsaKey *)&Key, DmaPtr);

		if (Crv->Class == ECDSA_ED_PH) {
			/** - Calculate the hash for the input message for Hash-based EdDSA mode. */
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XRsa_EccEdHashCalc(DmaPtr, (u32)Crv->CrvType, HashAddr, (u64)(UINTPTR)Hash,
					HashLen);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_RSA_ECC_HASH_CALC_FAIL;
				goto END_CLR;
			}
			HashBufAddr = (u64)(UINTPTR)Hash;
			HashBufLen = XASU_SHA_512_HASH_LEN;
		} else {
			HashBufAddr = HashAddr;
			HashBufLen = HashLen;
		}

		/** - Generate signature with provided inputs. */
		XFIH_CALL(Ecdsa_GenerateEdSign, XFihEcc, Status, Crv, (u8*)(UINTPTR)HashBufAddr,
			(HashBufLen * XASUFW_BYTE_LEN_IN_BITS), PrivKey, (EcdsaKey *)&Key,
			(EcdsaSign *)&Sign, DmaPtr);
	}
	if ((Status == XRSA_ECC_GEN_SIGN_BAD_R) ||
	    (Status == XRSA_ECC_GEN_SIGN_BAD_S)) {
		Status = XASUFW_RSA_ECC_GEN_SIGN_BAD_RAND_NUM;
	} else if ((Status == XRSA_ECC_GEN_SIGN_INCORRECT_HASH_LEN)) {
		Status = XASUFW_RSA_ECC_GEN_SIGN_INCORRECT_HASH_LEN;
	} else if ((Status != XRSA_ECC_SUCCESS)) {
		Status = XASUFW_RSA_ECC_GEN_SIGN_FAIL;
	} else {
		/**
		 *  Copy the generated signature to the destination address with byte-order reversal
		 *  (endianness swap rides the copy DMA) if the curve type is other than Edward curve.
		 */
		if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
			/** Weierstrass: reversal of r and s rides the copy-out DMA (per component). */
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XAsufw_DmaXfrOutReverse(DmaPtr, Signature, SignAddr, CurveLen);
		} else {
			/** Edward: r and s are little-endian, copied out per component (no reversal). */
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)Signature, SignAddr,
							CurveLen, 0U);
		}
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_READ_DATA_FAIL;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
			Status = XAsufw_DmaXfrOutReverse(DmaPtr,
							 Signature
							 + XRSA_ECC_ALIGNED_COMPONENT_OFFSET,
							 SignAddr + CurveLen, CurveLen);
		} else {
			Status = XAsufw_DmaXfr(DmaPtr,
					       (u64)(UINTPTR)(Signature + XRSA_ECC_ALIGNED_COMPONENT_OFFSET),
					       SignAddr + CurveLen, CurveLen, 0U);
		}
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_READ_DATA_FAIL;
			goto END_CLR;
		}
	}

	/**
	 * Measure and print the performance time for the ECC signature generation operation
	 * using RSA core, if performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_ECC_ID);

END_CLR:
	/** Zeroize local buffers. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcc, ClearStatus, (u8 *)(UINTPTR)PrivKey,
					XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)Signature, sizeof(Signature));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)Hash, XASU_SHA_512_HASH_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihEcc, ClearStatus, (u8 *)(UINTPTR)EphemeralKey,
					XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey, sizeof(PubKey));
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
 * @param	HashLen		Length of the hash buffer in bytes.
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
 *	-	XASUFW_RSA_ECC_INCORRECT_CURVE, if input curvetype or curvelen is incorrect.
 *	-	XASUFW_RSA_ECC_HASH_CALC_FAIL, if hash calculation fails in case of
 *						Ed25519ph or Ed448ph.
 *	-	XASUFW_ZEROIZE_MEMSET_FAIL, if memset fails while zeroizing local buffers.
 *	-	Error code from XAsufw_UpdateBufStatus, if local buffer zeroization fails.
 *	-	XASUFW_RSA_ECC_VER_SIGN_FAIL, if signature verification fails in the third party code.
 *
 *************************************************************************************************/
s32 XRsa_EccVerifySignature(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PubKeyAddr,
	u64 HashAddr, u32 HashLen, u64 SignAddr)
{
	/**
	 * Capture the start time of the ECC signature verification operation using RSA core, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_ECC_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEcc = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u32 CurveSize = 0U;
	u8 PubKey[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES
			+ XASUFW_VALUE_TWO + XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES]
		__attribute__((aligned(4))) = {0U};
	u8 Signature[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES
			+ XASUFW_VALUE_TWO + XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES]
		__attribute__((aligned(4))) = {0U};
	u8 Hash[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES] __attribute__((aligned(4))) = {0U};
	EcdsaSign Sign;
	EcdsaKey Key;
	EcdsaCrvInfo *Crv = NULL;
	u64 HashBufAddr = 0U;
	u32 HashBufLen = 0U;
	u32 HashPadLen = 0U;

	/** Validate the input arguments. */
	if ((DmaPtr == NULL) || (HashAddr == 0U) || (PubKeyAddr == 0U) || (SignAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/**
	 * Error out if curve type is Curve25519 or Curve448 as they don't support
	 * signature operations.
	 */
	if ((CurveType == XASU_ECC_CURVE25519) || (CurveType == XASU_ECC_CURVE448)) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (Crv == NULL) || (CurveLen != CurveSize)) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	/** For curves other than Edward curves, */
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
		/** Validate the hash buffer length. */
		if ((HashLen == 0U) || (HashLen > XASU_SHA_512_HASH_LEN)) {
			Status = XASUFW_RSA_ECC_INVALID_PARAM;
			goto END;
		}

		/** Zeroize the hash buffer. */
		Status = Xil_SecureZeroize(Hash, XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_ZEROIZE_MEMSET_FAIL;
			goto END;
		}

		/**
		 * If input hash length is less than curve length, left pad the extra bytes with 0's
		 * and copy hash to the local buffer.
		 */
		HashPadLen  = (HashLen < CurveLen) ? (CurveLen - HashLen) : 0U;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, HashAddr, (u64)(UINTPTR)(Hash + HashPadLen), HashLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
			goto END_CLR;
		}
	}

	/**
	 * Copy the public key and signature in. Weierstrass: each component (Qx, Qy, r, s) reversal
	 * rides its DMA, hash reversed in place; Edward curves take plain combined copies.
	 */
	if ((Crv->CrvType != ECDSA_ED25519) && (Crv->CrvType != ECDSA_ED448)) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfrInReverse(DmaPtr, PubKeyAddr, PubKey, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfrInReverse(DmaPtr, PubKeyAddr + CurveLen,
						PubKey + XRSA_ECC_ALIGNED_COMPONENT_OFFSET,
						CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfrInReverse(DmaPtr, SignAddr, Signature, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfrInReverse(DmaPtr, SignAddr + CurveLen,
						Signature + XRSA_ECC_ALIGNED_COMPONENT_OFFSET, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ReverseInPlace(DmaPtr, Hash, CurveSize);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
			goto END_CLR;
		}
	} else {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr, (u64)(UINTPTR)PubKey, CurveLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr + CurveLen,
				       (u64)(UINTPTR)(PubKey + XRSA_ECC_ALIGNED_COMPONENT_OFFSET),
				       CurveLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, SignAddr, (u64)(UINTPTR)Signature, CurveLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
			goto END_CLR;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, SignAddr + CurveLen,
				       (u64)(UINTPTR)(Signature + XRSA_ECC_ALIGNED_COMPONENT_OFFSET),
				       CurveLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
			goto END_CLR;
		}
	}

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + XRSA_ECC_ALIGNED_COMPONENT_OFFSET);

	Sign.r = (u8 *)(UINTPTR)Signature;
	Sign.s = (u8 *)(UINTPTR)(Signature + XRSA_ECC_ALIGNED_COMPONENT_OFFSET);

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
			Status = XRsa_EccEdHashCalc(DmaPtr, (u32)Crv->CrvType, HashAddr, (u64)(UINTPTR)Hash,
					HashLen);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_RSA_ECC_HASH_CALC_FAIL;
				goto END_CLR;
			}
			HashBufAddr = (u64)(UINTPTR)Hash;
			HashBufLen = XASU_SHA_512_HASH_LEN;
		} else {
			HashBufAddr = HashAddr;
			HashBufLen = HashLen;
		}
		XFIH_CALL(Ecdsa_VerifySign, XFihEcc, Status, Crv, (u8*)(UINTPTR)HashBufAddr,
			 (HashBufLen * XASUFW_BYTE_LEN_IN_BITS), (EcdsaKey *)&Key,
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
		Status = XASUFW_RSA_ECC_VER_SIGN_FAIL;
	} else {
		Status = XASUFW_SUCCESS;
		ReturnStatus = XASUFW_ECC_SIGNATURE_VERIFIED;
	}

	/**
	 * Measure and print the performance time for the ECC signature verification operation
	 * using RSA core, if performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_ECC_ID);

END_CLR:
	/** Zeroize local buffers. */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey, sizeof(PubKey));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)Signature, sizeof(Signature));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)Hash, XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/** Set RSA under reset. */
	XAsufw_CryptoCoreSetReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates an ECC scalar (private key or ephemeral nonce) from the
 *		TRNG (or a caller-supplied random buffer) for use by the ECC operations. It is
 *		the single place that draws the random value and maps a TRNG failure to
 *		XASUFW_RSA_ECC_TRNG_FAILED. When ReduceModOrder is XASU_TRUE, it draws
 *		(curve length + 64 bits) of data and reduces it modulo the curve order 'n' using
 *		Ecdsa_ModEccOrder() so the scalar is in [1, n-1] and effectively unbiased (as
 *		recommended by FIPS 186-4 section B.5.1). When ReduceModOrder is XASU_FALSE
 *		(Edward/Montgomery curves), it draws CurveLen bytes and returns the raw value
 *		without reduction.
 *
 * @param	CrvInfo		Pointer to the ECC curve information.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	InputRandBuf	Optional pointer to a caller-supplied random buffer. If NULL, the
 *				random value is generated using the TRNG.
 * @param	ReduceModOrder	XASU_TRUE to reduce the random value modulo the curve order (scalar
 *				in [1, n-1]); XASU_FALSE to return the raw random value.
 * @param	Scalar		Pointer to store the generated scalar in little-endian byte order
 *				(as emitted by Ecdsa_ModEccOrder()) when reduced.
 *
 * @return
 *	-	XASUFW_SUCCESS, if the scalar is generated successfully.
 *	-	XASUFW_RSA_ECC_TRNG_FAILED, if random number generation fails.
 *	-	XASUFW_RSA_ECC_MOD_ORDER_FAILED, if Ecdsa_ModEccOrder() fails.
 *	-	XASUFW_MEM_COPY_FAIL, if copying the raw random value to Scalar fails.
 *	-	Error code from XAsufw_UpdateBufStatus, if local buffer zeroization fails.
 *
 * @note	When ReduceModOrder is XASU_TRUE, the RSA core reset must already be released by
 *		the caller, as Ecdsa_ModEccOrder() uses the RSA core. The output is little-endian;
 *		reverse it if a big-endian scalar is required.
 *
 *************************************************************************************************/
static s32 XRsa_EccGenerateRandScalar(const EcdsaCrvInfo *CrvInfo, u32 CurveLen,
		const u8 *InputRandBuf, u8 ReduceModOrder, u8 *Scalar)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEcc = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 RandBuf[XRSA_ECC_P521_RANDBUFLEN_IN_BYTES] = {0U};
	const u8 *RandBufPtr = InputRandBuf;
	u8 RandGenLen = (u8)CurveLen;

	/**
	 * When the value is to be reduced modulo the curve order, draw (CurveLen + 64-bit) so the
	 * reduction is effectively unbiased (FIPS 186-4 section B.5.1), i.e.
	 *  - (CurveLen + 7U) bytes for curve NIST P-521.
	 *  - (CurveLen + 8U) bytes for all other curves.
	 * Otherwise (raw value) draw exactly CurveLen bytes.
	 */
	if (ReduceModOrder == (u8)XASU_TRUE) {
		if (CrvInfo->CrvType == ECDSA_NIST_P521) {
			RandGenLen = (u8)(CurveLen + XRSA_ECC_P521_RAND_NUM_GEN_EXTRA_BYTES);
		} else {
			RandGenLen = (u8)(CurveLen + XRSA_ECC_GENERIC_RAND_NUM_GEN_EXTRA_BYTES);
		}
	}

	/** Generate the random value using TRNG when no input buffer is provided. */
	if (InputRandBuf == NULL) {
		Status = XTrng_GetRandomNumbers(RandBuf, RandGenLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_TRNG_FAILED);
			goto END;
		}
		RandBufPtr = RandBuf;
	}

	/** For Edward/Montgomery curves, return the raw random value without reduction. */
	if (ReduceModOrder != (u8)XASU_TRUE) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SecureMemCpy(Scalar, CurveLen, RandBufPtr, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
		}
		goto END;
	}

	/**
	 * Reduce the random value modulo the curve order 'n' to obtain the scalar in little-endian
	 * order. Ecdsa_ModEccOrder() never returns an all-zero output for the (CurveLen + 64 bits)
	 * random input drawn here, so the resulting scalar is always in the range [1, n-1] and no
	 * explicit zero check is required.
	 */
	XFIH_CALL(Ecdsa_ModEccOrder, XFihEcc, Status, CrvInfo, RandBufPtr, Scalar);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_MOD_ORDER_FAILED;
		goto END;
	}

END:
	/** Zeroize the local random buffer. */
	if (InputRandBuf == NULL) {
		Status = XAsufw_UpdateBufStatus(Status, Xil_SecureZeroize(RandBuf,
					XRSA_ECC_P521_RANDBUFLEN_IN_BYTES));
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates the private key using TRNG and applies ModEccOrder to
 * 		the generated random number.
 *
 * @param	DmaPtr		Pointer to the DMA instance.
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
 *	-	XASUFW_RSA_ECC_MOD_ORDER_FAILED, if ModEccOrder fails.
 *	-	XASUFW_MEM_COPY_FAIL, if copying random number to PvtKey pointer fails.
 *	-	XASUFW_RSA_ECC_GEN_PVT_KEY_OPERATION_FAIL, if private key generation operation
 *						fails.
 *	-	XASUFW_RSA_CHANGE_ENDIANNESS_ERROR, if changing endianness fails.
 *	-	XASUFW_DMA_SCRUB_FAIL, if the DMA secret-path scrub fails.
 *
 *************************************************************************************************/
s32 XRsa_EccGeneratePvtKey(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u8 *PvtKey,
			   const u8 *InputRandBuf, u32 InputRandBufLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	EcdsaCrvInfo *CrvInfo = NULL;
	u32 CurveSize = 0U;
	u8 RandBufLen = 0U;

	/** Validate input parameters. */
	if ((PvtKey == NULL) || (DmaPtr == NULL)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/** Validate curve type, curve length and get Curve information. */
	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &CrvInfo);
	if ((CurveSize == 0U) || (CrvInfo == NULL)) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	/**
	 * Validate CurveLen:
	 * - For Curve448: accept 56 bytes (XASU_ECC_CURVE448_PVT_KEY_SIZE_IN_BYTES)
	 * - For other curves: must match CurveSize
	 */
	if (((CrvInfo->CrvType == ECDSA_CURVE448) &&
	     (CurveLen != XASU_ECC_CURVE448_PVT_KEY_SIZE_IN_BYTES)) ||
	    ((CrvInfo->CrvType != ECDSA_CURVE448) && (CurveLen != CurveSize))) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	/*
	 * Random buffer length is calculated based on the curve length:
	 * (CurveLen + 14 bytes) for P-521 and (CurveLen + 12 bytes) for all other curves.
	 */
	RandBufLen = (u8)(((CurveLen + XASUFW_VALUE_THREE) & ~(XASUFW_VALUE_THREE)) +
			XRSA_ECC_KEY_PAIR_GEN_EXTRA_BYTES);

	/** Validate input random buffer length. */
	if ((InputRandBuf != NULL) && (InputRandBufLen < RandBufLen)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/**
	 * For curves other than Edward and Montgomery curves,
	 * calculate the private key using third party code.
	 */
	if ((CrvInfo->CrvType != ECDSA_ED25519) && (CrvInfo->CrvType != ECDSA_ED448) &&
	    (CrvInfo->CrvType != ECDSA_CURVE25519) && (CrvInfo->CrvType != ECDSA_CURVE448)) {
		/** - Release Reset. */
		XAsufw_CryptoCoreReleaseReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

		/**
		 * - Generate the private key by reducing a random value (TRNG-generated when no
		 * input buffer is provided) modulo the curve order. The result is little-endian.
		 */
		Status = XRsa_EccGenerateRandScalar(CrvInfo, CurveLen, InputRandBuf,
				(u8)XASU_TRUE, PvtKey);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_GEN_PVT_KEY_OPERATION_FAIL);
			goto END;
		}

		/** Third party API returns little-endian; reverse the byte order in place. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ReverseInPlace(DmaPtr, PvtKey, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		}

		/** Scrub the DMA secret datapath */
		Status = XAsufw_UpdateBufStatus(Status,
				XAsufw_DmaScrubSecretPath(DmaPtr));
	} else {
		/**
		 * For edward and montgomery curves, generate the raw random value (when no input
		 * buffer is provided) and store it as the private key, without reduction. The key
		 * never transits the DMA datapath (CPU copy), so no DMA secret-path scrub is needed here.
		 */
		Status = XRsa_EccGenerateRandScalar(CrvInfo, CurveLen, InputRandBuf,
				(u8)XASU_FALSE, PvtKey);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_GEN_PVT_KEY_OPERATION_FAIL);
		}
	}

END:
	/** Set RSA under reset. */
	XAsufw_CryptoCoreSetReset(XRSA_BASEADDRESS, XRSA_RESET_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates the ECC key pair and validates them by performing PWCT.
 *
 * @param	DmaPtr		Pointer to DMA instance.
 * @param	CurveType	ECC Curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PvtKeyAddr	Address to store the generated private key.
 * @param	PubKeyAddr	Address to store the generated public key,
 * 				whose length shall be equal to double of CurveLen
 *
 * @return
 *	-	XASUFW_SUCCESS, if private key generation is successful.
 *	-	XASUFW_RSA_ECC_INVALID_PARAM, if any of the input parameter is invalid.
 *	-	XASUFW_RSA_ECC_GEN_PVT_KEY_OPERATION_FAIL, if private key generation operation fails.
 *	-	XASUFW_RSA_ECC_GEN_PUB_KEY_OPERATION_FAIL, if public key generation operation fails.
 *
 *************************************************************************************************/
s32 XRsa_EccGenKeyPair(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PvtKeyAddr,
		       u64 PubKeyAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	EcdsaKey Key;
	EcdsaCrvInfo *CrvInfo = NULL;
	u32 CurveSize = 0U;

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (PvtKeyAddr == 0U) || (PubKeyAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/** Generate private key. */
	Status = XRsa_EccGeneratePvtKey(DmaPtr, CurveType, CurveLen, (u8*)(UINTPTR)PvtKeyAddr, NULL,
					0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_GEN_PVT_KEY_OPERATION_FAIL);
		goto END;
	}

	/** Generate public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGeneratePubKey(DmaPtr, CurveType, CurveLen, PvtKeyAddr, PubKeyAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_GEN_PUB_KEY_OPERATION_FAIL);
	}

END:
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
	/**
	 * This is a configuration table to configure the curve type and curve class for all curves
	 * based on index.
	 */
	static const XRsa_EccCrvIndex XRsa_EccCrvIndexDb[] = {
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
		{ (u16)ECDSA_ED448, (u16)ECDSA_ED_PH },
		{ (u16)ECDSA_CURVE25519, (u16)ECDSA_MONTGOMERY },
		{ (u16)ECDSA_CURVE448, (u16)ECDSA_MONTGOMERY }
	};
	u32 Index;
	EcdsaCrvInfo *Crv = NULL;
	u32 TotalCurves = XRsa_EccCrvsGetCount();
	u16 CrvType;
	u16 CrvClass;

	if (CurveType > XASU_ECC_CURVE_TYPE_MAX) {
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
		CurveSize = XIL_SCEILDIV(u32, CrvInfo->Bits, XASUFW_BYTE_LEN_IN_BITS);
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
 *
 * @return
 *	-	XASUFW_SUCCESS, if ECDH shared secret is generated successfully.
 *	-	XASUFW_ECDH_INVALID_POINT_ON_CRV, if allocation of point failure.
 *	-	XASUFW_RSA_ECC_INVALID_PARAM, if any of the input parameters are invalid.
 *	-	XASUFW_RSA_ECC_WRITE_DATA_FAIL, if write data through DMA fails.
 *	-	XASUFW_RSA_ECC_READ_DATA_FAIL, if read data through DMA fails.
 *	-	XASUFW_ECDH_OTHER_ERROR, if operation fails due to any other error from
 *		third-party code.
 *	-	XASUFW_RSA_ECC_INCORRECT_CURVE, if input curvetype or curvelen is incorrect.
 *	-	XASUFW_ECDH_RAND_GEN_ERROR, if random number generation fails in the
 *		third-party shared secret computation.
 *	-	XASUFW_ZEROIZE_MEMSET_FAIL, if memset fails while zeroizing local buffers.
 *	-	XASUFW_DMA_SCRUB_FAIL, if the DMA secret-path scrub fails.
 *	-	Error code from XAsufw_UpdateBufStatus, if local buffer zeroization fails.
 *
 *************************************************************************************************/
s32 XRsa_EcdhGenSharedSecret(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PrivKeyAddr,
		u64 PubKeyAddr, u64 SharedSecretAddr)
{
	/**
	 * Capture the start time of the ECDH shared secret generation operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_ECC_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEcdh;
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 SharedSecret[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES] __attribute__((aligned(4))) = {0U};
	u8 PrivKey[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES] __attribute__((aligned(4))) = {0U};
	u8 PubKey[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES
			+ XASUFW_VALUE_TWO + XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES]
		__attribute__((aligned(4))) = {0U};
	EcdsaCrvInfo *Crv = NULL;
	EcdsaKey Key;
	u32 CurveSize = 0U;

	/** Validate the input arguments. */
	if ((DmaPtr == NULL) || (PrivKeyAddr == 0U) || (PubKeyAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	if (SharedSecretAddr == 0U) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	/** Error out if curve type is Ed25519 or Ed448 as they don't support ECDH operation. */
	if ((CurveType == XASU_ECC_NIST_ED25519) || (CurveType == XASU_ECC_NIST_ED448)) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	CurveSize = XRsa_EccValidateAndGetCrvInfo(CurveType, &Crv);
	if ((CurveSize == 0U) || (Crv == NULL)) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	/**
	 * Validate CurveLen:
	 * - For Curve448: accept 56 bytes (XASU_ECC_CURVE448_PVT_KEY_SIZE_IN_BYTES)
	 * - For other curves: must match CurveSize
	 */
	if (((Crv->CrvType == ECDSA_CURVE448) &&
	     (CurveLen != XASU_ECC_CURVE448_PVT_KEY_SIZE_IN_BYTES)) ||
	    ((Crv->CrvType != ECDSA_CURVE448) && (CurveLen != CurveSize))) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	/**
	 * Copy the public key to the local buffer. For Weierstrass curves the byte-order reversal of
	 * Qx and Qy rides the copy-in DMA (per component); Montgomery curves take a plain copy.
	 */
	if ((Crv->CrvType != ECDSA_CURVE25519) && (Crv->CrvType != ECDSA_CURVE448)) {
		Status = XAsufw_DmaXfrInReverse(DmaPtr, PubKeyAddr, PubKey, CurveLen);
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr, (u64)(UINTPTR)PubKey, CurveLen, 0U);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((Crv->CrvType != ECDSA_CURVE25519) && (Crv->CrvType != ECDSA_CURVE448)) {
		Status = XAsufw_DmaXfrInReverse(DmaPtr, PubKeyAddr + CurveLen,
						PubKey + XRSA_ECC_ALIGNED_COMPONENT_OFFSET,
						CurveLen);
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr + CurveLen,
				       (u64)(UINTPTR)(PubKey + XRSA_ECC_ALIGNED_COMPONENT_OFFSET),
				       CurveLen, 0U);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}

	Key.Qx = (u8 *)(UINTPTR)PubKey;
	Key.Qy = (u8 *)(UINTPTR)(PubKey + XRSA_ECC_ALIGNED_COMPONENT_OFFSET);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/**
	 * Zeroize the private key buffer before use to make sure for Curve448 57th byte is zero.
	 * Because, Curve448 private key is of 56 bytes.
	 */
	Status = Xil_SecureZeroize(PrivKey, XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END_CLR;
	}

	/**
	 * Copy the private key to the local buffer. For Weierstrass curves the byte-order reversal
	 * rides the copy DMA; Montgomery curves take a plain copy.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((Crv->CrvType != ECDSA_CURVE25519) && (Crv->CrvType != ECDSA_CURVE448)) {
		Status = XAsufw_DmaXfrInReverse(DmaPtr, PrivKeyAddr, PrivKey, CurveLen);
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, PrivKeyAddr, (u64)(UINTPTR)PrivKey, CurveLen, 0U);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_WRITE_DATA_FAIL;
		goto END_CLR;
	}

	/** Scrub the DMA secret datapath before releasing the core. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaScrubSecretPath(DmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_SCRUB_FAIL;
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
		goto END_CLR;
	}
	/**
	 * Copy the shared secret to the destination address using DMA. For Weierstrass curves the
	 * byte-order reversal rides the copy-out DMA; Montgomery curves take a plain copy.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((Crv->CrvType != ECDSA_CURVE25519) && (Crv->CrvType != ECDSA_CURVE448)) {
		Status = XAsufw_DmaXfrOutReverse(DmaPtr, SharedSecret, SharedSecretAddr, CurveLen);
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)SharedSecret, SharedSecretAddr,
					CurveLen, 0U);
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ECC_READ_DATA_FAIL;
	} else {
		ReturnStatus = XASUFW_RSA_ECDH_SUCCESS;
	}

	/** Scrub the DMA secret datapath after the shared secret has been copied out. */
	Status = XAsufw_UpdateBufStatus(Status, XAsufw_DmaScrubSecretPath(DmaPtr));

	/**
	 * Measure and print the performance time for the ECDH shared secret generation operation,
	 * if performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_ECC_ID);

END_CLR:

	/** Zeroize local copy of private key, public key and shared secret. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcdh, ClearStatus, (u8 *)(UINTPTR)PrivKey,
					XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)PubKey, sizeof(PubKey));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihEcdh, ClearStatus, (u8 *)(UINTPTR)SharedSecret,
					XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
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
 *	-	XASUFW_RSA_ECC_INCORRECT_CURVE, if input curvetype or curvelen is incorrect.
 *
 *************************************************************************************************/
s32 XRsa_EccPwct(XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen, u64 PrivKeyAddr,
	u64 PubKeyAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 Signature[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES + XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES] = {0U};
	const u8 Hash[XASU_ECC_P256_PVT_KEY_SIZE_IN_BYTES] = {
		0x8FU, 0xDFU, 0x4CU, 0x12U, 0x9AU, 0x90U, 0xBCU, 0x76U,
		0x38U, 0xA5U, 0x5BU, 0x22U, 0x9BU, 0xD4U, 0xAEU, 0x1BU,
		0x09U, 0x5DU, 0x6DU, 0x61U, 0x68U, 0xA0U, 0x43U, 0xC2U,
		0xC3U, 0x9FU, 0x53U, 0xE4U, 0xD8U, 0xC4U, 0xBEU, 0x5FU
	};

	if ((DmaPtr == NULL) || (PrivKeyAddr == 0U) || (PubKeyAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	if ((CurveType == XASU_ECC_CURVE25519) || (CurveType == XASU_ECC_CURVE448)) {
		Status = XASUFW_RSA_ECC_INCORRECT_CURVE;
		goto END;
	}

	Status = XRsa_EccGenerateSignature(DmaPtr, CurveType, CurveLen, PrivKeyAddr,
		 NULL, (u64)(UINTPTR)Hash, XASU_ECC_P256_PVT_KEY_SIZE_IN_BYTES,
		 (u64)(UINTPTR)Signature);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_PWCT_SIGN_GEN_FAIL);
		goto END_CLR;
	}

	/** Verify the generated signature using the provided public key and curve type. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccVerifySignature(DmaPtr, CurveType, CurveLen, PubKeyAddr,
			(u64)(UINTPTR)Hash, XASU_ECC_P256_PVT_KEY_SIZE_IN_BYTES,
			(u64)(UINTPTR)Signature);
	if ((Status != XASUFW_SUCCESS) || (ReturnStatus != XASUFW_ECC_SIGNATURE_VERIFIED)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_ECC_PWCT_SIGN_VER_FAIL);
	}
	/*
	 * Pair Wise Consistency Test validates the key pair during key generation.
	 * Here, the ReturnStatus is from signature verification, checked internally and
	 * does not need to be exposed to the client, so it is reset to XASUFW_FAILURE.
	 */
	ReturnStatus = XASUFW_FAILURE;

END_CLR:
	/** Zeroize the local buffers. */
	Status = XAsufw_UpdateBufStatus(Status, Xil_SecureZeroize((u8 *)(UINTPTR)Signature,
					XAsu_DoubleCurveLength(XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES)));

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
 *	- XASUFW_RSA_ECC_INVALID_PARAM, if CurveType is invalid.
 *
 *************************************************************************************************/
static s32 XRsa_EccEdHashCalc(XAsufw_Dma *DmaPtr, u32 CurveType, u64 DataAddr, u64 HashAddr,
			u32 DataSize)
{
	s32 Status = XASUFW_FAILURE;
	XSha *ShaInstancePtr = NULL;
	XAsu_ShaOperationCmd ShaOperation;

	if (CurveType == (u32)ECDSA_ED25519) {
		/** For Ed25519ph, get the SHA2 instance for SHA512 mode. */
		ShaInstancePtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
		ShaOperation.ShaMode = XASU_SHA_MODE_512;
	} else if (CurveType == (u32)ECDSA_ED448) {
		/** For Ed448ph, get the SHA3 instance for SHAKE256 mode. */
		ShaInstancePtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
		ShaOperation.ShaMode = XASU_SHA_MODE_SHAKE256;
	} else {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
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
#endif /* XASU_RSA_ECC_ENABLE */
/** @} */
