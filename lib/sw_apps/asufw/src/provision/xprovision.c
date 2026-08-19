/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xprovision.c
*
* This file implements provisioning of the USER eFUSE keys. The key comes from one of three sources
* selected via KeySource: ASU-generated (TRNG), AKV vault (KeyId), or user buffer (KeyAddr).
* USER_KEY_0 allows only the ASU-generated source; USER_KEY_1 allows all three. The key is always
* PUF KEK encrypted (black form) in the ASU, then sent to the PLM (XilNVM) over IPI for the eFUSE
* write.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   kp   06/27/26 Initial release
*
* </pre>
*
**************************************************************************************************/
/**
 * @addtogroup xprovision_server_apis Provisioning Server APIs
 * @{
 */
/************************************** Include Files ********************************************/
#include "xprovision.h"
#include "xasu_provision_common.h"
#include "xasufw_hw.h"
#include "xasufw_memory.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_debug.h"
#include "xasufw_init.h"
#include "xasufw_keytransfer.h"
#include "xasufw_ipi.h"
#include "xaes.h"
#include "xtrng.h"
#include "xkeymanager.h"
#include "xasu_aesinfo.h"
#include "xasu_generic.h"
#include "xfih.h"
#include "xil_sutil.h"

#ifdef XASU_PROVISION_ENABLE
/********************************** Constant Definitions *****************************************/

/************************************ Macro Definitions ******************************************/
#define XPROVISION_EFUSE_KEY_SIZE_IN_BYTES	(32U)	/**< USER eFUSE key size in bytes
							(256-bit) */
#define XPROVISION_EFUSE_IV_SIZE_IN_BYTES	(12U)	/**< IV size in bytes for the black key
							AES-GCM operation (96-bit) */
#define XPROVISION_EFUSE_TAG_SIZE_IN_BYTES	(16U)	/**< GCM tag size in bytes */

/************************************ Type Definitions *******************************************/

/********************************** Variable Definitions *****************************************/

/************************************ Function Prototypes ****************************************/
static s32 XProvision_AcquireKey(XAsufw_Dma *DmaPtr,
				 const XAsu_ProvisionUserEfuseParams *ParamsPtr,
				 u32 SubsystemId, u8 *KeyBuf);
static s32 XProvision_EncryptRedKey(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, u8 EfuseKeyId,
				      const u8 *PlainKey, u8 *BlackKey);
static s32 XProvision_WriteUserKeyToEfuse(u32 EfuseKeyId, const u32 *KeyData);
static s32 XProvision_CheckEfuseKeyBlank(u32 EfuseKeyId);

/*************************************************************************************************/
/**
 * @brief	This function programs a USER eFUSE key (USER_KEY_0 / USER_KEY_1) with a key
 *		from the source selected in ParamsPtr->KeySource (ASU-generated via TRNG, AKV
 *		vault, or user buffer). USER_KEY_0 is restricted to the ASU-generated source.
 *		The key is always stored in black form (PUF KEK encrypted) for both user keys.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance (used for red key encryption).
 * @param	ParamsPtr	Pointer to the XAsu_ProvisionUserEfuseParams structure.
 * @param	SubsystemId	Subsystem ID of the requesting subsystem (used for AKV access).
 *
 * @return
 *	- XASUFW_SUCCESS, if the USER eFUSE key is provisioned successfully.
 *	- XASUFW_PROVISION_INVALID_PARAM, if any input parameter is invalid.
 *	- XASUFW_PROVISION_KEY_FETCH_FAIL, if obtaining the key fails.
 *	- Error code from XAsufw_PmcKeyTransfer, if the PMC key transfer fails.
 *	- XASUFW_PUF_KEK_GEN_FAIL, if PUF KEK generation fails.
 *	- XASUFW_PROVISION_RED_ENCRYPT_FAIL, if red key encryption fails.
 *	- XASUFW_ERR_IPI_RSP_PLM_EFUSE_REQ, if the target key is already programmed, or the PLM
 *		reports a failure for the blank-key check or the eFUSE write.
 *	- Error code from the IPI transport, if the IPI request/response to PLM fails.
 *	- Error code from XAes_KeyClear or Xil_SecureZeroize, if clearing the key vault or zeroizing
 *		the local key buffers fails, even when provisioning itself succeeded.
 *
 * @note	Each eFUSE key is checked to be unprogrammed before any key material is produced, so
 *		a given key is encrypted under its RTCA base IV at most once per device.
 *
 *************************************************************************************************/
s32 XProvision_UserEfuse(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
			 const XAsu_ProvisionUserEfuseParams *ParamsPtr, u32 SubsystemId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 KeyBuf[XPROVISION_EFUSE_KEY_SIZE_IN_BYTES] = {0U};
	/* Black key passed to the PLM by address; word-aligned u32 array by type. */
	u32 BlackKeyBuf[XASUFW_NVM_EFUSE_AES_KEY_LEN_IN_WORDS] = {0U};
	u32 PufKekGenStatus = XASUFW_FAILURE;

	/** Validate the server-side resource pointers. */
	if ((DmaPtr == NULL) || (AesInstancePtr == NULL)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/**
	 * Validate all command parameters up front: EfuseKeyId, KeyLen, the per-source
	 * KeyId/KeyAddr rules, and the key source against the destination eFUSE.
	 */
	Status = XAsu_ProvisionValidateUserEfuseParams(ParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/** User key 0 accepts benign key only from ASU-generated source. */
	if ((ParamsPtr->EfuseKeyId == XASU_PROVISION_EFUSE_USER_KEY_0) &&
	    (ParamsPtr->KeySource != XASU_PROVISION_KEY_SOURCE_ASU_GENERATED)) {
		Status = XASUFW_PROVISION_INVALID_PARAM;
		goto END;
	}

	/**
	 * Reject an already programmed key before any key material is produced, so a doomed
	 * request cannot consume a vault key usage count or re-encrypt under the same KEK and IV.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_CheckEfuseKeyBlank((u32)ParamsPtr->EfuseKeyId);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Obtain the plaintext key from the requested source. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_AcquireKey(DmaPtr, ParamsPtr, SubsystemId, KeyBuf);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_KEY_FETCH_FAIL);
		goto END_CLR;
	}

	/** Reject an all-zero key before it is burned, for every key source. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_IsBufferNonZero(KeyBuf, XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_KEY_FETCH_FAIL;
		goto END_CLR;
	}

	/**
	 * Re-transfer the eFUSE keys and PUF KEK from the PMC for this encryption and validate the
	 * PUF KEK generation status.
	 */
	XFIH_CALL_GOTO(XAsufw_PmcKeyTransfer, XFihVar, Status, END_CLR, &PufKekGenStatus);

	if (PufKekGenStatus != (u32)XASUFW_SUCCESS) {
		Status = XASUFW_PUF_KEK_GEN_FAIL;
		goto END_CLR;
	}

	/** Both USER eFUSE keys are programmed in black form: PUF KEK encrypt inside the ASU. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_EncryptRedKey(DmaPtr, AesInstancePtr, ParamsPtr->EfuseKeyId, KeyBuf,
					 (u8 *)BlackKeyBuf);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PROVISION_RED_ENCRYPT_FAIL);
		goto END_CLR;
	}

	/** Request the PLM (XilNVM) to program the selected USER eFUSE key with the black key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XProvision_WriteUserKeyToEfuse((u32)ParamsPtr->EfuseKeyId,
						(const u32 *)BlackKeyBuf);

END_CLR:
	/**
	 * Clear eFUSE key 0, eFUSE key 1, eFUSE red key 0, eFUSE red key 1 and the PUF key
	 * in a single call, covering every path after XAsufw_PmcKeyTransfer.
	 */
	XFIH_CALL(XAes_KeyClear, XFihVar, ClearStatus, AesInstancePtr,
		  XAES_KEY_CLEAR_MULTI_KEY_FLAG |
		  XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_0) |
		  XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_RED_0) |
		  XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_1) |
		  XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_RED_1) |
		  XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_PUF_KEY));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/** Zeroize the local key buffers. */
	XFIH_CALL(Xil_SecureZeroize, XFihVar, ClearStatus, KeyBuf,
		  XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	ClearStatus = Xil_SecureZeroize((u8 *)BlackKeyBuf, XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function obtains the 256-bit plaintext key to be programmed, based on the
 *		explicitly selected key source (ASU-generated via TRNG, AKV vault, or user buffer).
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ParamsPtr	Pointer to the XAsu_ProvisionUserEfuseParams structure.
 * @param	SubsystemId	Subsystem ID of the requesting subsystem (used for AKV access).
 * @param	KeyBuf		Pointer to the buffer that receives the 256-bit key.
 *
 * @return
 *	- XASUFW_SUCCESS, if the key is obtained successfully.
 *	- XASUFW_PROVISION_INVALID_PARAM, if the key source is invalid or the vault key is not
 *		256-bit.
 *	- Error code from XKeyManager_UpdateKeyObjFromVault, if the AKV key fetch fails, including
 *		XASUFW_KEYMANAGER_KEY_TYPE_MISMATCH when the KeyId is not an AES sub-vault key.
 *	- XASUFW_MEM_COPY_FAIL, if copying the AKV key (vault path) fails.
 *	- XASUFW_DMA_COPY_FAIL, if the DMA transfer of the user key (buffer path) fails.
 *	- Error code from XTrng_GetRandomNumbers, if TRNG key generation (ASU-generated path) fails.
 *
 *************************************************************************************************/
static s32 XProvision_AcquireKey(XAsufw_Dma *DmaPtr,
				 const XAsu_ProvisionUserEfuseParams *ParamsPtr,
				 u32 SubsystemId, u8 *KeyBuf)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XAsu_AesKeyObject KeyObject = {0U};

	/** Obtain the key based on the explicitly selected key source. */
	switch (ParamsPtr->KeySource) {
		case XASU_PROVISION_KEY_SOURCE_ASU_GENERATED:
			/** ASU-generated path: generate a 256-bit AES key from the TRNG. */
			Status = XTrng_GetRandomNumbers(KeyBuf, XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
			break;
		case XASU_PROVISION_KEY_SOURCE_VAULT:
			/** Vault path: fetch the AES key from the AKV by KeyId. */
			KeyObject.KeyId = ParamsPtr->KeyId;
			Status = XKeyManager_UpdateKeyObjFromVault(DmaPtr, ParamsPtr->KeyId,
				(u64)(UINTPTR)&KeyObject, SubsystemId,
				(u8)(XASU_KEYMANAGER_AES_ENC_USE_CASE |
				XASU_KEYMANAGER_AES_DEC_USE_CASE),
				XKEYMANAGER_RSA_OP_NONE, XASU_AES_SUBVAULT_ID);
			if (Status != XASUFW_SUCCESS) {
				goto END;
			}
			/** Only a 256-bit key can be programmed. */
			if (KeyObject.KeySize != XASU_AES_KEY_SIZE_256_BITS) {
				Status = XASUFW_PROVISION_INVALID_PARAM;
				goto END;
			}
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = Xil_SMemCpy(KeyBuf, XPROVISION_EFUSE_KEY_SIZE_IN_BYTES,
					     (const void *)(UINTPTR)KeyObject.KeyAddress,
					     XPROVISION_EFUSE_KEY_SIZE_IN_BYTES,
					     XPROVISION_EFUSE_KEY_SIZE_IN_BYTES);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_MEM_COPY_FAIL;
			}
			break;
		case XASU_PROVISION_KEY_SOURCE_USER_BUFFER:
			/** Buffer path: copy the user supplied key into the local buffer. */
			Status = XAsufw_DmaXfr(DmaPtr, ParamsPtr->KeyAddr, (u64)(UINTPTR)KeyBuf,
					       XPROVISION_EFUSE_KEY_SIZE_IN_BYTES, 0U);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_DMA_COPY_FAIL;
			}
			break;
		default:
			Status = XASUFW_PROVISION_INVALID_PARAM;
			break;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function encrypts the given plaintext key with the PUF KEK to produce the black
 *		key to be programmed into the eFUSE. The caller transfers the PUF KEK before this
 *		call and clears the key vault after it.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	AesInstancePtr	Pointer to the XAes instance.
 * @param	EfuseKeyId	Target USER eFUSE key (selects the KEK IV: eFUSE0 or eFUSE1).
 * @param	PlainKey	Pointer to the 256-bit plaintext key.
 * @param	BlackKey	Pointer to the buffer that receives the 256-bit black
 *				(encrypted) key.
 *
 * @return
 *	- XASUFW_SUCCESS, if the black key is generated successfully.
 *	- XASUFW_MEM_COPY_FAIL, if copying the eFUSE KEK IV fails.
 *	- XASUFW_KEK_IV_IS_ZERO, if the eFUSE KEK IV is zero.
 *	- Error code from XAes_EncryptRedKey, if the AES encryption operation fails.
 *
 * @note	Each eFUSE key has its own RTCA base IV, which is used as-is.
 *
 *************************************************************************************************/
static s32 XProvision_EncryptRedKey(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr, u8 EfuseKeyId,
				      const u8 *PlainKey, u8 *BlackKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 ClearStatus = XASUFW_FAILURE;
	const u8 *IvPtr = (EfuseKeyId == XASU_PROVISION_EFUSE_USER_KEY_1) ?
		(const u8 *)(UINTPTR)XASUFW_PLM_RTCA_EFUSE_1_IV_ADDR :
		(const u8 *)(UINTPTR)XASUFW_PLM_RTCA_EFUSE_0_IV_ADDR;
	u8 IvBuf[XPROVISION_EFUSE_IV_SIZE_IN_BYTES] = {0U};
	u8 TagBuf[XPROVISION_EFUSE_TAG_SIZE_IN_BYTES] = {0U};
	XAsu_AesParams AesParams = {0U};

	/** Copy the eFUSE KEK IV (eFUSE0 or eFUSE1) from PLM RTCA and ensure it is non-zero. */
	Status = Xil_SMemCpy(IvBuf, XPROVISION_EFUSE_IV_SIZE_IN_BYTES, IvPtr,
			     XPROVISION_EFUSE_IV_SIZE_IN_BYTES, XPROVISION_EFUSE_IV_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_IsBufferNonZero(IvBuf, XPROVISION_EFUSE_IV_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEK_IV_IS_ZERO;
		goto END;
	}

	/** AES-GCM encrypt the red key with the PUF KEK, selected by XAes_EncryptRedKey. */
	AesParams.EngineMode = (u8)XASU_AES_GCM_MODE;
	AesParams.OperationType = (u8)XASU_AES_ENCRYPT_OPERATION;
	AesParams.IvObj.IvAddr = (u64)(UINTPTR)IvBuf;
	AesParams.IvObj.IvLen = XPROVISION_EFUSE_IV_SIZE_IN_BYTES;
	AesParams.InputDataAddr = (u64)(UINTPTR)PlainKey;
	AesParams.OutputDataAddr = (u64)(UINTPTR)BlackKey;
	AesParams.DataLen = XPROVISION_EFUSE_KEY_SIZE_IN_BYTES;
	AesParams.TagAddr = (u64)(UINTPTR)TagBuf;
	AesParams.TagLen = XPROVISION_EFUSE_TAG_SIZE_IN_BYTES;

	/** Encrypt the red key with the PUF KEK and return the raw AES status. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAes_EncryptRedKey(AesInstancePtr, DmaPtr, &AesParams);

	/** Zeroize the local tag buffer. */
	ClearStatus = Xil_SecureZeroize(TagBuf, XPROVISION_EFUSE_TAG_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends an IPI request to the PLM to program a USER eFUSE key
 *		(USER_KEY_0 / USER_KEY_1) with the provided black (PUF KEK encrypted) key.
 *
 * @param	EfuseKeyId	USER eFUSE key identifier (XASU_PROVISION_EFUSE_USER_KEY_0 or
 *				XASU_PROVISION_EFUSE_USER_KEY_1).
 * @param	KeyData		Pointer to the 256-bit (8-word) black key data to be programmed.
 *
 * @return
 *	- XASUFW_SUCCESS, if the USER eFUSE key is programmed.
 *	- Error code from the IPI transport, if the IPI request/response to PLM fails.
 *	- XASUFW_ERR_IPI_RSP_PLM_EFUSE_REQ, if the PLM reports a failure for the eFUSE write.
 *
 *************************************************************************************************/
static s32 XProvision_WriteUserKeyToEfuse(u32 EfuseKeyId, const u32 *KeyData)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Payload[XASUFW_PLM_XILNVM_WRITE_PAYLOAD_LEN];
	u32 Response = (u32)XASUFW_FAILURE;

	/**
	 * Build the AES-key-write IPI payload: {hdr, EnvDis|KeyType, KeyAddrLow,
	 * KeyAddrHigh}. The PLM reads the key by address; the caller supplies it in
	 * aligned, stable memory.
	 */
	Payload[XASUFW_PLM_XILNVM_CMD_HDR_INDEX] =
		XASUFW_PLM_IPI_HEADER(XASUFW_PLM_XILNVM_PLOAD_LEN,
				      XASUFW_PLM_XILNVM_WRITE_AES_KEY_API_ID,
				      XASUFW_PLM_XILNVM_MODULE_ID);
	Payload[XASUFW_PLM_XILNVM_ENVDIS_KEYTYPE_INDEX] =
		(EfuseKeyId << XASUFW_PLM_XILNVM_KEYTYPE_SHIFT) |
		XASUFW_PLM_XILNVM_ENV_MON_CHECK;
	Payload[XASUFW_PLM_XILNVM_KEY_ADDR_LOW_INDEX] = (u32)(UINTPTR)KeyData;
	/* ASU is a 32-bit address space; the high address word is always zero. */
	Payload[XASUFW_PLM_XILNVM_KEY_ADDR_HIGH_INDEX] = 0U;

	/** Send the IPI request to PLM; propagate any IPI transport error. */
	Status = XAsufw_SendIpiToPlm(Payload, XASUFW_PLM_XILNVM_WRITE_PAYLOAD_LEN);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Read the IPI response; propagate any IPI transport error. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ReadIpiRespFromPlm(&Response, XASUFW_PLM_XILNVM_RESP_LEN);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * The IPI read succeeded but the PLM may still report a failure in the response.
	 * Print the PLM response and return the provisioning error code to the client.
	 */
	if (Response != (u32)XASUFW_SUCCESS) {
		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "PLM eFUSE write failed: 0x%08x\r\n", Response);
		Status = XASUFW_ERR_IPI_RSP_PLM_EFUSE_REQ;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function asks the PLM to verify the target USER eFUSE key against the CRC of
 * 		an all-zero key, which matches only while the key is unprogrammed.
 *
 * @param	EfuseKeyId	USER eFUSE key identifier (XASU_PROVISION_EFUSE_USER_KEY_0 or
 *				XASU_PROVISION_EFUSE_USER_KEY_1).
 *
 * @return
 *	- XASUFW_SUCCESS, if the target USER eFUSE key is unprogrammed.
 *	- Error code from the IPI transport, if the IPI request/response to PLM fails.
 *	- XASUFW_ERR_IPI_RSP_PLM_EFUSE_REQ, if the key is already programmed or the PLM reports any
 *		other failure.
 *
 *************************************************************************************************/
static s32 XProvision_CheckEfuseKeyBlank(u32 EfuseKeyId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Payload[XASUFW_PLM_XILNVM_CRC_PAYLOAD_LEN];
	u32 Response = (u32)XASUFW_FAILURE;

	/**
	 * Build the CRC check payload: {hdr, Crc, KeyType}.
	 * KeyType occupies a whole word here.
	 */
	Payload[XASUFW_PLM_XILNVM_CMD_HDR_INDEX] =
		XASUFW_PLM_IPI_HEADER(XASUFW_PLM_XILNVM_CRC_PLOAD_LEN,
				      XASUFW_PLM_XILNVM_CHECK_CRC_API_ID,
				      XASUFW_PLM_XILNVM_MODULE_ID);
	Payload[XASUFW_PLM_XILNVM_CRC_INDEX] = XASUFW_NVM_EFUSE_CRC_AES_ZEROS;
	Payload[XASUFW_PLM_XILNVM_CRC_KEYTYPE_INDEX] = EfuseKeyId;

	/** Send the IPI request to PLM; propagate any IPI transport error. */
	Status = XAsufw_SendIpiToPlm(Payload, XASUFW_PLM_XILNVM_CRC_PAYLOAD_LEN);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Read the IPI response; propagate any IPI transport error. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ReadIpiRespFromPlm(&Response, XASUFW_PLM_XILNVM_RESP_LEN);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * A CRC mismatch means the key is already programmed; any other failure is a controller or
	 * command error. Print the PLM response and return a single error code for both.
	 */
	if (Response != (u32)XASUFW_SUCCESS) {
		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "PLM eFUSE CRC check failed: 0x%08x\r\n",
			      Response);
		Status = XASUFW_ERR_IPI_RSP_PLM_EFUSE_REQ;
	}

END:
	return Status;
}

#endif /* XASU_PROVISION_ENABLE */
/** @} */
