/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xfih.h"
#include "xasufw_perf.h"
#include "xasu_ecies_common.h"
#include "xasu_def.h"
#include "xkdf.h"

#ifdef XASU_ECIES_ENABLE
/************************************** Type Definitions *****************************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
static s32 XEcies_ValidateParameters(const XAsu_EciesParams *EciesParams,
				     u32 IsCmacKdf);
static s32 XEcies_HkdfGenerate(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
		const XAsu_EciesParams *EciesParams, const u8* SharedSecretPtr, const u8* KOutPtr);
static s32 XEcies_CmacKdfGenerate(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
		const XAsu_EciesParams *EciesParams, const u8 *SharedSecretPtr, u8 *KOutPtr);
static s32 XEcies_AesCompute(XAes *AesInstancePtr, XAsufw_Dma *DmaPtr,
		const XAsu_EciesParams *EciesParams, const u8 *Key, u8 OperationType);

/************************************** Macros Definitions ***************************************/
#define XECIES_CMAC_CONTEXT_OFFSET	(XASU_AES_KEY_SIZE_256BIT_IN_BYTES)
						/**< Offset of the CMAC context in the P-384
							shared secret. */
#define XECIES_CMAC_CONTEXT_LEN		(XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES - \
					 XECIES_CMAC_CONTEXT_OFFSET)
						/**< Length of the CMAC context in bytes. */

/************************************** Function Definitions *************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates ECIES parameters for the selected KDF profile.
 *
 * @param	EciesParams	Pointer to the XAsu_EciesParams structure.
 * @param	IsCmacKdf	Flag indicating whether the AEPI CMAC-KDF profile is selected.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ECIES parameters are valid for the selected KDF.
 * 	- XASUFW_FAILURE, if ECIES parameters or the KDF flag are invalid.
 *
 *************************************************************************************************/
static s32 XEcies_ValidateParameters(const XAsu_EciesParams *EciesParams,
				     u32 IsCmacKdf)
{
	s32 Status = XASUFW_FAILURE;

	if (IsCmacKdf == XASU_FALSE) {
		if (XAsu_ValidateEciesParameters(EciesParams) == XST_SUCCESS) {
			Status = XASUFW_SUCCESS;
		}
	} else if (IsCmacKdf == XASU_TRUE) {
		/**
		 * SHA, salt, and caller context fields are not used by this profile. The CMAC
		 * context is taken directly from bytes 32 through 47 of the P-384 ECDH result.
		 */
		if ((EciesParams == NULL) ||
		    ((EciesParams->AesKeySize != XASU_AES_KEY_SIZE_128_BITS) &&
		     (EciesParams->AesKeySize != XASU_AES_KEY_SIZE_256_BITS)) ||
		    (EciesParams->IvLength != XASU_AES_IV_SIZE_96BIT_IN_BYTES) ||
		    (EciesParams->MacLength != XASU_AES_MAX_TAG_LENGTH_IN_BYTES) ||
		    (EciesParams->DataLength == 0U) ||
		    (EciesParams->DataLength > XASUFW_DMA_BLOCKING_SIZE) ||
		    (EciesParams->EccCurveType != XASU_ECC_NIST_P384) ||
		    (EciesParams->EccKeyLength != XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES) ||
		    (EciesParams->InDataAddr == 0U) || (EciesParams->IvAddr == 0U) ||
		    (EciesParams->MacAddr == 0U) || (EciesParams->OutDataAddr == 0U) ||
		    (EciesParams->TxKeyAddr == 0U) || (EciesParams->RxKeyAddr == 0U)) {
			Status = XASUFW_FAILURE;
		} else {
			Status = XASUFW_SUCCESS;
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 *
 * @brief	This function performs ECIES encryption operation.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaInstancePtr	Pointer to the XSha instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	EciesParams	Pointer to the XAsu_EciesParams structure.
 * @param	IsCmacKdf	Flag indicating whether the AEPI CMAC-KDF profile is selected.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ECIES encryption is successful.
 * 	- XASUFW_ECIES_INVALID_PARAM, if input parameters are invalid.
 * 	- XASUFW_ECIES_PUB_KEY_VALIDATE_FAILURE, if Rx public key validation fails.
 * 	- XASUFW_ECIES_PVT_KEY_GEN_FAILURE, if private key generation fails.
 * 	- XASUFW_ECIES_PUB_KEY_GEN_FAILURE, if public key generation fails.
 * 	- XASUFW_ECIES_ECDH_FAILURE, if ECDH operation fails.
 * 	- XASUFW_ECIES_HKDF_FAILURE, if HKDF operation fails.
 * 	- XASUFW_ECIES_CMAC_KDF_FAILURE, if CMAC-KDF operation fails.
 * 	- XASUFW_ECIES_AES_FAILURE, if AES operation fails.
 * 	- Error code from XAsufw_UpdateBufStatus, if local buffer zeroization fails.
 *
 *************************************************************************************************/
s32 XEcies_Encrypt(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, XAes *AesInstancePtr,
		   const XAsu_EciesParams *EciesParams, u32 IsCmacKdf)
{
	/**
	 * Capture the start time of the ECIES encryption operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_ECIES_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEcies = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 SharedSecret[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES] = {0U};
	u8 KOut[XASU_AES_KEY_SIZE_256BIT_IN_BYTES] = {0U};
	u8 PrivKey[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES] = {0U};

	/** Validate the input parameters. */
	if ((DmaPtr == NULL) || (AesInstancePtr == NULL) ||
	    ((IsCmacKdf == XASU_FALSE) && (ShaInstancePtr == NULL))) {
		Status = XASUFW_ECIES_INVALID_PARAM;
		goto END;
	}

	/** Validate ECIES parameters. */
	Status = XEcies_ValidateParameters(EciesParams, IsCmacKdf);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECIES_INVALID_PARAM;
		goto END;
	}

	/** Validate the received Rx public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccValidatePubKey(DmaPtr, EciesParams->EccCurveType,
		EciesParams->EccKeyLength, EciesParams->RxKeyAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_PUB_KEY_VALIDATE_FAILURE);
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Generate the private key. */
	Status = XRsa_EccGeneratePvtKey(DmaPtr, EciesParams->EccCurveType, EciesParams->EccKeyLength,
					PrivKey, NULL, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_PVT_KEY_GEN_FAILURE);
		goto END_CLR;
	}

	/** Calculate Tx public key using the generated Tx private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGeneratePubKey(DmaPtr, EciesParams->EccCurveType,
				EciesParams->EccKeyLength, (u64)(UINTPTR)PrivKey,
				EciesParams->TxKeyAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_PUB_KEY_GEN_FAILURE);
		goto END_CLR;
	}

	/** Execute ECDH to compute the shared secret using the Tx Private key and Rx public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EcdhGenSharedSecret(DmaPtr, EciesParams->EccCurveType,
			EciesParams->EccKeyLength, (u64)(UINTPTR)PrivKey, EciesParams->RxKeyAddr,
			(u64)(UINTPTR)SharedSecret);
	if ((Status != XASUFW_SUCCESS) || (ReturnStatus != XASUFW_RSA_ECDH_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_ECDH_FAILURE);
	}
	/**
	 * Reset ReturnStatus after it has been checked. ECDH writes this global, so it must
	 * not remain SUCCESS and be reused by a later ECDH or other operation.
	 */
	ReturnStatus = XASUFW_FAILURE;

	/** Zeroize the private key immediately after use. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcies, ClearStatus, PrivKey,
			XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	/** Generate the encryption key from the shared secret using KDF.  */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (IsCmacKdf == XASU_FALSE) {
		Status = XEcies_HkdfGenerate(DmaPtr, ShaInstancePtr, EciesParams,
					     SharedSecret, KOut);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_HKDF_FAILURE);
		}
	} else {
		Status = XEcies_CmacKdfGenerate(DmaPtr, AesInstancePtr, EciesParams,
					       SharedSecret, KOut);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_CMAC_KDF_FAILURE);
		}
	}

	/** Zeroize the shared secret immediately after use. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcies, ClearStatus, SharedSecret,
			XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	/**
	 * Perform the AES operation to encrypt the provided plaintext using the key derived
	 * from the KDF, while also generating a MAC.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcies_AesCompute(AesInstancePtr, DmaPtr, EciesParams, KOut,
				   XASU_AES_ENCRYPT_OPERATION);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_AES_FAILURE);
	}

	/** Zeroize the key derived from KDF immediately after use. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcies, ClearStatus, KOut,
			XASU_AES_KEY_SIZE_256BIT_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/**
	 * Measure and print the performance time for the ECIES encryption operation, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_ECIES_ID);
	goto END;

END_CLR:
	/** Zeroize private key, shared secret, and derived key remnants. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcies, ClearStatus, PrivKey,
			XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihEcies, ClearStatus, SharedSecret,
			XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihEcies, ClearStatus, KOut,
			XASU_AES_KEY_SIZE_256BIT_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

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
 * @param	IsCmacKdf	Flag indicating whether the AEPI CMAC-KDF profile is selected.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ECIES decryption is successful.
 * 	- XASUFW_ECIES_INVALID_PARAM, if input parameters are invalid.
 * 	- XASUFW_ECIES_PUB_KEY_VALIDATE_FAILURE, if Tx public key validation fails.
 * 	- XASUFW_ECIES_ECDH_FAILURE, if ECDH operation fails.
 * 	- XASUFW_ECIES_HKDF_FAILURE, if HKDF operation fails.
 * 	- XASUFW_ECIES_CMAC_KDF_FAILURE, if CMAC-KDF operation fails.
 * 	- XASUFW_ECIES_AES_FAILURE, if AES operation fails.
 * 	- Error code from XAsufw_UpdateBufStatus, if local buffer zeroization fails.
 *
 *************************************************************************************************/
s32 XEcies_Decrypt(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, XAes *AesInstancePtr,
		   const XAsu_EciesParams *EciesParams, u32 IsCmacKdf)
{
	/**
	 * Capture the start time of the ECIES decryption operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_ECIES_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEcies = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 SharedSecret[XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES] = {0U};
	u8 KOut[XASU_AES_KEY_SIZE_256BIT_IN_BYTES] = {0U};

	/** Validate the input parameters. */
	if ((DmaPtr == NULL) || (AesInstancePtr == NULL) ||
	    ((IsCmacKdf == XASU_FALSE) && (ShaInstancePtr == NULL))) {
		Status = XASUFW_ECIES_INVALID_PARAM;
		goto END;
	}

	/** Validate ECIES parameters. */
	Status = XEcies_ValidateParameters(EciesParams, IsCmacKdf);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECIES_INVALID_PARAM;
		goto END;
	}

	/** Validate the received Tx public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccValidatePubKey(DmaPtr, EciesParams->EccCurveType,
		EciesParams->EccKeyLength, EciesParams->TxKeyAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_PUB_KEY_VALIDATE_FAILURE);
		goto END;
	}

	/** Execute ECDH to compute the shared secret using the Rx Private key and Tx public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EcdhGenSharedSecret(DmaPtr, EciesParams->EccCurveType,
		EciesParams->EccKeyLength, EciesParams->RxKeyAddr, EciesParams->TxKeyAddr,
		(u64)(UINTPTR)SharedSecret);
	if ((Status != XASUFW_SUCCESS) || (ReturnStatus != XASUFW_RSA_ECDH_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_ECDH_FAILURE);
		goto END_CLR;
	}

	/** Generate the decryption key from the shared secret using KDF.  */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (IsCmacKdf == XASU_FALSE) {
		Status = XEcies_HkdfGenerate(DmaPtr, ShaInstancePtr, EciesParams,
					     SharedSecret, KOut);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_HKDF_FAILURE);
		}
	} else {
		Status = XEcies_CmacKdfGenerate(DmaPtr, AesInstancePtr, EciesParams,
					       SharedSecret, KOut);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_CMAC_KDF_FAILURE);
		}
	}

	/** Zeroize the shared secret immediately after use. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcies, ClearStatus, SharedSecret,
		XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	/**
	 * Perform the AES operation to decrypt the provided ciphertext using the key derived
	 * from the KDF and verify the received MAC against the generated MAC.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcies_AesCompute(AesInstancePtr, DmaPtr, EciesParams, KOut,
				   XASU_AES_DECRYPT_OPERATION);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_AES_FAILURE);
	}

	/** Zeroize the key derived from KDF immediately after use. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcies, ClearStatus, KOut,
			XASU_AES_KEY_SIZE_256BIT_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/**
	 * Measure and print the performance time for the ECIES decryption operation, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_ECIES_ID);
	goto END;

END_CLR:
	/** Zeroize shared secret and derived key remnants. */
	XFIH_CALL(Xil_SecureZeroize, XFihEcies, ClearStatus, SharedSecret,
			XASU_ECC_P521_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihEcies, ClearStatus, KOut,
			XASU_AES_KEY_SIZE_256BIT_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	/**
	 * Reset ReturnStatus after it has been checked. ECDH writes this global, so it must
	 * not remain SUCCESS and be reused by a later ECDH or other operation.
	 */
	ReturnStatus = XASUFW_FAILURE;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function derives the ECIES AES key using the AEPI CMAC-KDF profile.
 *
 * @param	DmaPtr			Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr		Pointer to the AES instance.
 * @param	EciesParams		Pointer to the ECIES parameters.
 * @param	SharedSecretPtr	Pointer to the 48-byte P-384 shared secret.
 * @param	KOutPtr			Pointer to the derived key output buffer.
 *
 * @return
 * 	- XASUFW_SUCCESS, if CMAC-KDF generation and key clearing are successful.
 * 	- XASUFW_ECIES_INVALID_PARAM, if an input parameter is invalid.
 * 	- Error codes from XKdf_CmacGenerate, if CMAC-KDF generation fails.
 * 	- Error codes from XAes_KeyClear, if clearing the CMAC input key fails.
 *
 *************************************************************************************************/
static s32 XEcies_CmacKdfGenerate(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
		const XAsu_EciesParams *EciesParams, const u8 *SharedSecretPtr, u8 *KOutPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XAsu_KdfParams KdfParams = {0U};

	if ((DmaPtr == NULL) || (AesInstancePtr == NULL) || (EciesParams == NULL) ||
	    (SharedSecretPtr == NULL) || (KOutPtr == NULL)) {
		ASSIGN_VOLATILE(Status, XASUFW_ECIES_INVALID_PARAM);
		goto END;
	}

	/** Use the first 32 bytes of Z as the AES-256 CMAC key. */
	KdfParams.KeyObject.KeyInAddr = (u64)(UINTPTR)SharedSecretPtr;
	KdfParams.KeyObject.KeyInLen = XASU_AES_KEY_SIZE_256_BITS;
	KdfParams.KeyOutAddr = (u64)(UINTPTR)KOutPtr;
	if (EciesParams->AesKeySize == XASU_AES_KEY_SIZE_128_BITS) {
		KdfParams.KeyOutLen = XASU_AES_KEY_SIZE_128BIT_IN_BYTES;
	} else {
		KdfParams.KeyOutLen = XASU_AES_KEY_SIZE_256BIT_IN_BYTES;
	}

	/** Use the remaining 16 bytes of Z as the complete counter-mode context. */
	KdfParams.ContextAddr =
		(u64)(UINTPTR)&SharedSecretPtr[XECIES_CMAC_CONTEXT_OFFSET];
	KdfParams.ContextLen = XECIES_CMAC_CONTEXT_LEN;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XKdf_CmacGenerate(DmaPtr, AesInstancePtr, &KdfParams, XASU_AES_USER_KEY_7);

END:
	/** Clear the shared-secret portion written to USER_KEY_7 on every exit. */
	if (AesInstancePtr != NULL) {
		ASSIGN_VOLATILE(ClearStatus, XASUFW_FAILURE);
		ClearStatus = XAes_KeyClear(AesInstancePtr, XASU_AES_USER_KEY_7);
		if (ClearStatus != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, ClearStatus);
		}
	}

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
	HkdfParams.KdfParams.KeyObject.KeyInAddr = (u64)(UINTPTR)SharedSecretPtr;
	HkdfParams.KdfParams.KeyObject.KeyInLen = (u32)EciesParams->EccKeyLength;
	HkdfParams.KdfParams.KeyObject.KeyId = 0U;
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
 * 	- Error code from XAsufw_UpdateBufStatus, if AES key object zeroization fails.
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
	Status = XAes_WriteKey(AesInstancePtr, DmaPtr, &KeyObject);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECIES_AES_WRITE_KEY_FAILURE;
		goto END;
	}

	/* Initialize AES parameters structure for encryption/decryption operation. */
	AesParams.EngineMode = (u8)XASU_AES_GCM_MODE;
	AesParams.OperationType = OperationType;
	AesParams.KeyObjectAddr = (u64)(UINTPTR)&KeyObject;
	AesParams.IvObj.IvAddr = EciesParams->IvAddr;
	AesParams.IvObj.IvLen = (u32)EciesParams->IvLength;
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
