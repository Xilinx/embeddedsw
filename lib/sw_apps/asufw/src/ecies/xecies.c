/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
 *
 * @file xecies.c
 *
 * This file contains the implementation of the ECIES APIs.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------------------------------------
 * 1.0   yog  02/20/25 Initial release
 *       yog  03/24/25 Used XRsa_EccGeneratePrivKey() API in ECIES encryption operation.
 *       yog  04/04/25 Performing AesKeyClear operation in XEcies_AesCompute() API
 *       LP   04/07/25 Added HKDF support for key generation
 * 1.1   am   05/18/25 Fixed implicit conversion of operands
 *
 * </pre>
 *
 *
 **************************************************************************************************/
/**
* @addtogroup xecies_server_apis ECIES Server APIs
* @{
*/

/*************************************** Include Files *******************************************/
#include "xecies.h"
#include "xrsa_ecc.h"
#include "xasu_eccinfo.h"
#include "xhkdf.h"
#include "xhmac.h"
#include "xecc.h"
#include "xrsa_ecc.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xfih.h"
#include "xasu_ecies_common.h"
#include "xasu_def.h"

#ifdef XASU_ECIES_ENABLE
/************************************** Type Definitions *****************************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
static s32 XEcies_HkdfGenerate(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
		const XAsu_EciesParams *EciesParams, const u8* SharedSecretPtr, const u8* KOutPtr);
static s32 XEcies_AesCompute(XAes *AesInstancePtr, XAsufw_Dma *DmaPtr,
		const XAsu_EciesParams *EciesParams, const u8 *Key, u8 OperationType);

/************************************** Macros Definitions ***************************************/

/************************************** Function Definitions *************************************/

/*************************************************************************************************/
/**
 *
 * @brief	This function performs ECIES encryption operation.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaInstancePtr	Pointer to the XSha instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	EciesParams	Pointer to the XAsu_EciesParams structure.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ECIES encryption is successful.
 * 	- XASUFW_ECIES_INVALID_PARAM, if input parameters are invalid.
 * 	- XASUFW_ECIES_PVT_KEY_GEN_FAILURE, if private key generation fails.
 * 	- XASUFW_ECIES_PUB_KEY_GEN_FAILURE, if public key generation fails.
 * 	- XASUFW_ECIES_ECDH_FAILURE, if ECDH operation fails.
 * 	- XASUFW_ECIES_HKDF_FAILURE, if HKDF operation fails.
 * 	- XASUFW_ECIES_AES_FAILURE, if AES operation fails.
 *
 *************************************************************************************************/
s32 XEcies_Encrypt(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, XAes *AesInstancePtr,
		   const XAsu_EciesParams *EciesParams)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEcies = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 SharedSecret[XASU_ECC_P521_SIZE_IN_BYTES] = {0U};
	u8 KOut[XASU_AES_KEY_SIZE_256BIT_IN_BYTES] = {0U};
	u8 PrivKey[XASU_ECC_P521_SIZE_IN_BYTES] = {0U};

	/** Validate the input parameters. */
	if ((DmaPtr == NULL) || (ShaInstancePtr == NULL) || (AesInstancePtr == NULL)) {
		Status = XASUFW_ECIES_INVALID_PARAM;
		goto END;
	}

	/** Validate ECIES parameters. */
	Status = XAsu_ValidateEciesParameters(EciesParams);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_ECIES_INVALID_PARAM;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Generate the private key. */
	Status = XRsa_EccGeneratePvtKey(EciesParams->EccCurveType, EciesParams->EccKeyLength,
					PrivKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_PVT_KEY_GEN_FAILURE);
		goto END;
	}

	/** Calculate public key using the generated private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGeneratePubKey(DmaPtr, EciesParams->EccCurveType,
				EciesParams->EccKeyLength, (u64)(UINTPTR)PrivKey,
				EciesParams->TxKeyAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_PUB_KEY_GEN_FAILURE);
		goto END;
	}

	/** Execute ECDH to compute the shared secret using the Tx Private key and Rx public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EcdhGenSharedSecret(DmaPtr, EciesParams->EccCurveType,
		EciesParams->EccKeyLength, (u64)(UINTPTR)PrivKey, EciesParams->RxKeyAddr,
		(u64)(UINTPTR)SharedSecret, 0U);

	/** Zeroize the private key immediately after use. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcies, ClearStatus, PrivKey,
			XASU_ECC_P521_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	if ((Status != XASUFW_SUCCESS) || (ClearStatus != XASUFW_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_ECDH_FAILURE);
		goto END;
	}

	/** Generate the encryption key from the shared secret using KDF.  */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcies_HkdfGenerate(DmaPtr, ShaInstancePtr, EciesParams, SharedSecret, KOut);

	/** Zeroize the shared secret immediately after use. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcies, ClearStatus, SharedSecret,
			XASU_ECC_P521_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	if ((Status != XASUFW_SUCCESS) || (ClearStatus != XASUFW_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_HKDF_FAILURE);
		goto END;
	}

	/**
	 * Perform the AES operation to encrypt the provided plaintext using the key derived
	 * from the KDF, while also generating a MAC.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcies_AesCompute(AesInstancePtr, DmaPtr, EciesParams, KOut,
				   XASU_AES_ENCRYPT_OPERATION);

	/** Zeroize the key derived from KDF immediately after use. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcies, ClearStatus, KOut,
			XASU_AES_KEY_SIZE_256BIT_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	if ((Status != XASUFW_SUCCESS) || (ClearStatus != XASUFW_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_AES_FAILURE);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 *
 * @brief	This function performs ECIES decryption operation.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaInstancePtr	Pointer to the XSha instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	EciesParams	Pointer to the XAsu_EciesParams structure.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ECIES decryption is successful.
 * 	- XASUFW_ECIES_INVALID_PARAM, if input parameters are invalid.
 * 	- XASUFW_ECIES_ECDH_FAILURE, if ECDH operation fails.
 * 	- XASUFW_ECIES_HKDF_FAILURE, if HKDF operation fails.
 * 	- XASUFW_ECIES_AES_FAILURE, if AES operation fails.
 *
 *************************************************************************************************/
s32 XEcies_Decrypt(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, XAes *AesInstancePtr,
		   const XAsu_EciesParams *EciesParams)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEcies = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 SharedSecret[XASU_ECC_P521_SIZE_IN_BYTES] = {0U};
	u8 KOut[XASU_AES_KEY_SIZE_256BIT_IN_BYTES] = {0U};

	/** Validate the input parameters. */
	if ((DmaPtr == NULL) || (ShaInstancePtr == NULL) || (AesInstancePtr == NULL)) {
		Status = XASUFW_ECIES_INVALID_PARAM;
		goto END;
	}

	/** Validate ECIES parameters. */
	Status = XAsu_ValidateEciesParameters(EciesParams);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECIES_INVALID_PARAM;
		goto END;
	}

	/** Execute ECDH to compute the shared secret using the Rx Private key and Tx public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EcdhGenSharedSecret(DmaPtr, EciesParams->EccCurveType,
		EciesParams->EccKeyLength, EciesParams->RxKeyAddr, EciesParams->TxKeyAddr,
		(u64)(UINTPTR)SharedSecret, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_ECDH_FAILURE);
		goto END;
	}

	/** Generate the decryption key from the shared secret using KDF.  */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcies_HkdfGenerate(DmaPtr, ShaInstancePtr, EciesParams, SharedSecret, KOut);

	/** Zeroize the shared secret immediately after use. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcies, ClearStatus, SharedSecret,
		XASU_ECC_P521_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	if ((Status != XASUFW_SUCCESS) || (ClearStatus != XASUFW_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_HKDF_FAILURE);
		goto END;
	}

	/**
	 * Perform the AES operation to decrypt the provided ciphertext using the key derived
	 * from the KDF, and verify the received MAC against the generated MAC.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcies_AesCompute(AesInstancePtr, DmaPtr, EciesParams, KOut,
				   XASU_AES_DECRYPT_OPERATION);

	/** Zeroize the key derived from KDF immediately after use. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcies, ClearStatus, KOut,
			XASU_AES_KEY_SIZE_256BIT_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	if ((Status != XASUFW_SUCCESS) || (ClearStatus != XASUFW_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_AES_FAILURE);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 *
 * @brief	This function performs the HKDF operation as part of ECIES.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaInstancePtr	Pointer to the XSha instance.
 * @param	EciesParams	Pointer to the ECIES structure containing user input parameters.
 * @param	SharedSecretPtr	Pointer to the shared secret.
 * @param	KOutPtr		Pointer to store the key out.
 *
 * @return
 * 	- XASUFW_SUCCESS, if HKDF Generate operation is successful.
 * 	- Errors codes from HKDF, if HKDF operation fails.
 *
 *************************************************************************************************/
static s32 XEcies_HkdfGenerate(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
		const XAsu_EciesParams *EciesParams, const u8* SharedSecretPtr, const u8* KOutPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XAsu_HkdfParams HkdfParams;

	/** Provide inputs to XAsu_HkdfParams structure. */
	if (EciesParams->AesKeySize == XASU_AES_KEY_SIZE_128_BITS) {
		HkdfParams.KdfParams.KeyOutLen = XASU_AES_KEY_SIZE_128BIT_IN_BYTES;
	} else {
		HkdfParams.KdfParams.KeyOutLen = XASU_AES_KEY_SIZE_256BIT_IN_BYTES;
	}
	HkdfParams.KdfParams.ShaType = (u8)EciesParams->ShaType;
	HkdfParams.KdfParams.ShaMode = (u8)EciesParams->ShaMode;
	HkdfParams.KdfParams.KeyInAddr = (u64)(UINTPTR)SharedSecretPtr;
	HkdfParams.KdfParams.KeyInLen = (u32)EciesParams->EccKeyLength;
	HkdfParams.KdfParams.KeyOutAddr = (u64)(UINTPTR)KOutPtr;
	HkdfParams.KdfParams.ContextAddr = EciesParams->ContextAddr;
	HkdfParams.KdfParams.ContextLen = EciesParams->ContextLen;
	HkdfParams.SaltAddr = (u64)(UINTPTR)EciesParams->SaltAddr;
	HkdfParams.SaltLen = EciesParams->SaltLen;

	/** Perform HKDF operation. */
	Status = XHkdf_Generate(DmaPtr, ShaInstancePtr, &HkdfParams);

	return Status;
}

/*************************************************************************************************/
/**
 *
 * @brief	This function performs the AES operation as part of ECIES.
 *
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	EciesParams	Pointer to the ECIES structure containing user input parameters.
 * @param	Key		Pointer to the key to be used in AES operation.
 * @param	OperationType	AES operation type.
 * 				XASU_AES_ENCRYPT_OPERATION - Encryption operation.
 * 				XASU_AES_DECRYPT_OPERATION - Decryption operation.
 *
 * @return
 * 	- XASUFW_SUCCESS, if AES operation is successful.
 * 	- XASUFW_ECIES_AES_WRITE_KEY_FAILURE, if AES write key fails.
 * 	- Errors codes from AES, if AES operation fails.
 *
 *************************************************************************************************/
static s32 XEcies_AesCompute(XAes *AesInstancePtr, XAsufw_Dma *DmaPtr,
		const XAsu_EciesParams *EciesParams, const u8 *Key, u8 OperationType)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XAsu_AesParams AesParams;
	XAsu_AesKeyObject KeyObject;

	KeyObject.KeySize = (u32)EciesParams->AesKeySize;
	KeyObject.KeySrc = XASU_AES_USER_KEY_7;
	KeyObject.KeyAddress = (u64)(UINTPTR)(Key);

	/** Write AES key. */
	Status = XAes_WriteKey(AesInstancePtr, DmaPtr, (u64)(UINTPTR)&KeyObject);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECIES_AES_WRITE_KEY_FAILURE;
		goto END;
	}

	/* Initialize AES parameters structure for encryption/decryption operation. */
	AesParams.EngineMode = (u8)XASU_AES_GCM_MODE;
	AesParams.OperationType = OperationType;
	AesParams.KeyObjectAddr = (u64)(UINTPTR)&KeyObject;
	AesParams.IvAddr = EciesParams->IvAddr;
	AesParams.IvLen = (u32)EciesParams->IvLength;
	AesParams.InputDataAddr = EciesParams->InDataAddr;
	AesParams.OutputDataAddr = EciesParams->OutDataAddr;
	AesParams.DataLen = EciesParams->DataLength;
	AesParams.AadAddr = 0U;
	AesParams.AadLen = 0U;
	AesParams.TagAddr = EciesParams->MacAddr;
	AesParams.TagLen = (u32)EciesParams->MacLength;

	/** Perform AES operation based on the operation type. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAes_Compute(AesInstancePtr, DmaPtr, &AesParams);

END:
	/** Clear the key written to the XASU_AES_USER_KEY_7 key source. */
	Status = XAsufw_UpdateErrorStatus(Status, XAes_KeyClear(AesInstancePtr, KeyObject.KeySrc));

	/** Zeroize the key object. */
	ClearStatus = Xil_SecureZeroize((u8 *)(UINTPTR)&KeyObject, sizeof(XAsu_AesKeyObject));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}
#endif /* XASU_ECIES_ENABLE */
/** @} */
