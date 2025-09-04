/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file x509_asufw.c
*
* This file contains the implementation of the ASUFW platform specific code for X.509 certificate
* generation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------------------------
* 1.0   rmv  05/19/25 Initial release
*
* </pre>
*
**************************************************************************************************/
/**
 * @addtogroup x509_apis X.509 APIs
 * @{
 */
/*************************************** Include Files *******************************************/
#include "x509_asufw.h"
#include "x509_cert.h"
#include "xasu_eccinfo.h"
#include "xasu_shainfo.h"
#include "xasufw_status.h"
#include "xasufw_trnghandler.h"
#include "xasufw_util.h"
#include "xecc.h"
#include "xil_types.h"
#include "xsha.h"
#include "xsha_hw.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define X509_XECC_CURVE_TYPE		XECC_CURVE_TYPE_NIST_P384	/**< ECC mode used for
									X.509 certificate signing */
#define X509_ECC_SIGN_SIZE_IN_BYTES	XASU_ECC_P384_SIZE_IN_BYTES	/**< ECC Signature size in
									bytes */
#define X509_SHA_MODE			XASU_SHA_MODE_384		/**< SHA mode used for
									X.509 certificate digest
									calculation */
#define X509_SHA_HASH_LEN		XASU_SHA_384_HASH_LEN		/**< SHA Length in bytes */

/************************************ Function Prototypes ****************************************/
static s32 X509_GenerateSignEcc(const u8 *Hash, u32 HashLen, const u8 *Sign, u32 SignLen,
				u32 *SignActualLen, u8 *PvtKey, const void *PlatformData);
static s32 X509_ShaDigest(const u8 *Buf, u32 DataLen, const u8 *Hash, u32 HashBufLen,
			  u32 *HashLen, const void *PlatformData);
static s32 X509_VerifySignEcc(const u8 *Hash, u32 HashLen, const u8 *PubKey, const u8 *Sign,
			      u32 SignLen, const void *PlatformData);

/*********************************** Variable Definitions ****************************************/

/*************************************************************************************************/
/**
 * @brief	This function configures X.509 for ASUFW.
 *
 * @return
 *	- XASUFW_SUCCESS, if X.509 is configured successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 X509_CfgInitialize(void)
{
	s32 Status = XASUFW_FAILURE;
	X509_InitData InitData;

	/** Assign signature algorithm based on the configurations. */
	InitData.SignType = X509_SIGN_TYPE_ECC_SHA3_384;
	InitData.GenerateDigest = X509_ShaDigest;
	InitData.GenerateSignature = X509_GenerateSignEcc;
	InitData.VerifySignature = X509_VerifySignEcc;

	Status = X509_Init(&InitData);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates ECC signature.
 *
 * @param	Hash		Pointer to variable containing hash.
 * @param	HashLen		Length of hash.
 * @param	Sign		Pointer to variable which stores signature.
 * @param	SignLen 	Length of signature buffer.
 * @param	SignActualLen 	Pointer to variable which stores actual length of signature.
 * @param	PvtKey		Pointer to variable containing private-key.
 * @param	PlatformData	Pointer contains platform related data.
 *
 * @return
 *	- XASUFW_SUCCESS, if signature is calculated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_INVALID_PLAT_DATA, if platform data is invalid.
 *	- XASUFW_X509_INVALID_BUFFER_SIZE, if buffer size is invalid.
 *	- XASUFW_X509_ECC_EPHEMERAL_KEY_GEN_FAIL, if ephemeral key is not generated.
 *	- XASUFW_X509_GEN_SIGN_ECC_FAIL, if signature is not generated.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_GenerateSignEcc(const u8 *Hash, u32 HashLen, const u8 *Sign, u32 SignLen,
				u32 *SignActualLen, u8 *PvtKey, const void *PlatformData)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XEcc *EccInstance = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	u8 EphemeralKey[X509_ECC_SIGN_SIZE_IN_BYTES] = {0U};
	const X509_PlatData *PlatData = (X509_PlatData *)PlatformData;
	u32 SignatureSize = XAsu_DoubleCurveLength(X509_ECC_SIGN_SIZE_IN_BYTES);

	/** Validate input parameters. */
	if ((Hash == NULL) || (Sign == NULL) || (SignActualLen == NULL) || (PvtKey == NULL) ||
	    (PlatData == NULL)) {
		Status = XASUFW_X509_INVALID_PLAT_DATA;
		goto END;
	}

	/* Validate signature buffer length. */
	if (SignLen < SignatureSize) {
		Status = XASUFW_X509_INVALID_BUFFER_SIZE;
		goto END;
	}

	/** Generate ephemeral key using TRNG. */
	Status = XAsufw_TrngGetRandomNumbers(EphemeralKey, X509_ECC_SIGN_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_ECC_EPHEMERAL_KEY_GEN_FAIL);
		goto END;
	}

	/* Generate signature on the hash data. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcc_GenerateSignature(EccInstance, PlatData->DmaPtr, X509_XECC_CURVE_TYPE,
					X509_ECC_SIGN_SIZE_IN_BYTES, (u64)(UINTPTR)PvtKey,
					EphemeralKey, (u64)(UINTPTR)Hash,
					HashLen, (u64)(UINTPTR)Sign);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_SIGN_ECC_FAIL);
		goto END;
	}
	*SignActualLen = SignatureSize;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function calculates SHA digest.
 *
 * @param	Buf		Pointer to variable containing data.
 * @param	DataLen		Data length.
 * @param	Hash		Pointer to variable which stores calculated hash.
 * @param	HashBufLen	Length of hash buffer.
 * @param	HashLen		Pointer to variable which stores actual length of calculated hash.
 * @param	PlatformData	Pointer containing platform related data.
 *
 * @return
 *	- XASUFW_SUCCESS, if digest is calculated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_INVALID_PLAT_DATA, if platform data is invalid.
 *	- XASUFW_X509_INVALID_BUFFER_SIZE, if hash buffer size is invalid.
 *	- XASUFW_X509_SHA_DIGEST_FAIL, if digest calculation is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_ShaDigest(const u8 *Buf, u32 DataLen, const u8 *Hash, u32 HashBufLen,
			  u32 *HashLen, const void *PlatformData)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XAsu_ShaOperationCmd ShaParam;
	const X509_PlatData *PlatData = (X509_PlatData *)PlatformData;

	/** Validate input parameters. */
	if ((Buf == NULL) || (Hash == NULL) || (HashLen == NULL) || (PlatData == NULL)) {
		Status = XASUFW_X509_INVALID_PLAT_DATA;
		goto END;
	}

	/** Validate hash buffer length. */
	if (HashBufLen < X509_SHA_HASH_LEN) {
		Status = XASUFW_X509_INVALID_BUFFER_SIZE;
		goto END;
	}

	/** Initialize sha parameters. */
	ShaParam.DataAddr = (u64)(UINTPTR)Buf;
	ShaParam.HashAddr = (u64)(UINTPTR)Hash;
	ShaParam.DataSize = DataLen;
	ShaParam.HashBufSize = X509_SHA_HASH_LEN;
	ShaParam.ShaMode = X509_SHA_MODE;
	ShaParam.IsLast = (u8)XASU_TRUE;
	ShaParam.OperationFlags = (XASU_SHA_START | XASU_SHA_UPDATE | XASU_SHA_FINISH);

	/** Calculate SHA-3 digest on given data. */
	Status = XSha_Digest(PlatData->ShaPtr, PlatData->DmaPtr, &ShaParam);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_SHA_DIGEST_FAIL);
		goto END;
	}

	*HashLen = X509_SHA_HASH_LEN;

END:
	return Status;
}

/*************************************************************************************************/
 /**
 * @brief	This function verifies ECC signature.
 *
 * @param	Hash		Pointer to variable containing hash.
 * @param	HashLen		Hash length.
 * @param	PubKey		Pointer to variable containing public key.
 * @param	Sign		Pointer to variable containing signature.
 * @param	SignLen		Signature length.
 * @param	PlatformData	Pointer containing platform related data.
 *
 * @return
 *	- XASUFW_SUCCESS, if ECC signature is verified successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_INVALID_PLAT_DATA, if platform data is invalid.
 *	- XASUFW_X509_INVALID_DATA, if data is invalid.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_VerifySignEcc(const u8 *Hash, u32 HashLen, const u8 *PubKey, const u8 *Sign,
				u32 SignLen, const void *PlatformData)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XEcc *EccInstance = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	const X509_PlatData *PlatData = (X509_PlatData *)PlatformData;

	/** Validate input parameters. */
	if ((Sign == NULL) || (Hash == NULL) || (PubKey == NULL) || (PlatData == NULL) ||
	    (HashLen == 0U)) {
		Status = XASUFW_X509_INVALID_PLAT_DATA;
		goto END;
	}

	/** Validate signature length. */
	if (SignLen != XAsu_DoubleCurveLength(XASU_ECC_P384_SIZE_IN_BYTES)) {
		Status = XASUFW_X509_INVALID_DATA;
		goto END;
	}

	/** Verify ECC signature. */
	Status = XEcc_VerifySignature(EccInstance, PlatData->DmaPtr, XASU_ECC_NIST_P384,
				      XASU_ECC_P384_SIZE_IN_BYTES, (u64)(UINTPTR)PubKey,
				      (u64)(UINTPTR)Hash, HashLen, (u64)(UINTPTR)Sign);

END:
	return Status;
}
/** @} */
